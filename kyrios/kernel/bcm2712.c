/* bcm2712.c: lirvol, Raspberry Pi 5 (BCM2712). The kernel layer's board
 * support: serial lines, the firmware framebuffer, the hardware counter,
 * and the exception report.
 *
 * Nothing here uses interrupts. Input is polled and time is read from the
 * counter. The MMU and the data cache are off.
 *
 * Where the addresses and values come from:
 *   - Linux device tree for this board (raspberrypi/linux, rpi-6.12.y,
 *     arch/arm64/boot/dts/broadcom/bcm2712.dtsi and rp1.dtsi): the debug
 *     UART is a PL011 at 0x107d001000 with a 9216000 Hz clock; the
 *     firmware mailbox is at 0x107c013880; RP1 UART0 is at 0x1f00030000.
 *   - Raspberry Pi documentation, config.txt: enable_rp1_uart=1 makes the
 *     firmware set RP1 UART0 to 115200 on the 40-pin header and leave RP1
 *     running when it starts the image.
 *   - Circle (rsta2/circle), read as a reference for the order of the
 *     mailbox framebuffer tags on this board and for the RP1 pin registers.
 *
 * First run on lirvol: 2026-11-18. v0.0.1 booted to its waiting state
 * on HDMI (framebuffer 1920 x 1080, header UART set up by the firmware,
 * handed over at EL1). Each step that could stop the processor says what
 * it is about to do first, so the last line on a working surface names
 * the step that failed.
 */
#include "lib.h"
#include "board.h"
#include "diag.h"
#include "font8x8.h"

#define MMIO32(a) (*(volatile u32 *)(u64)(a))

#define WAIT_LIMIT 4000000UL        /* turns of a polling loop before giving up */

/* ---- guarded access -------------------------------------------------- */

static volatile u64 probe_armed, probe_hit;

static void settle(void) {
    int i;
    __asm__ volatile("isb");
    for (i = 0; i < 64; i++) __asm__ volatile("nop");
}

/* Read a register that may not be there. Returns 1 when the read came
 * back, 0 when it raised an exception. SError is let through only for
 * the length of this function. One that was already waiting from before
 * is taken first, so that it is not counted against the read. */
static int probe_read32(u64 addr, u32 *out, int *stale) {
    u32 v;
    probe_hit = 0;
    probe_armed = 1;
    __asm__ volatile("msr daifclr, #4");
    settle();
    *stale = probe_hit != 0;
    probe_hit = 0;
    v = MMIO32(addr);
    settle();
    __asm__ volatile("msr daifset, #4");
    settle();
    probe_armed = 0;
    if (probe_hit) return 0;
    *out = v;
    return 1;
}

/* ---- PL011 serial ---------------------------------------------------- */

#define UART_DR    0x00
#define UART_FR    0x18
#define UART_IBRD  0x24
#define UART_FBRD  0x28
#define UART_LCRH  0x2c
#define UART_CR    0x30
#define UART_IMSC  0x38
#define UART_ICR   0x44
#define FR_BUSY    (1u << 3)
#define FR_RXFE    (1u << 4)
#define FR_TXFF    (1u << 5)

struct pl011 {
    u64 base;
    int up;
    int stalls;
};

static struct pl011 debug_uart  = { 0x107d001000UL, 0, 0 };    /* 3-pin debug connector */
static struct pl011 header_uart = { 0x1f00030000UL, 0, 0 };    /* RP1 UART0: pins 8 and 10 */

static void uart_setup(struct pl011 *u, u32 ibrd, u32 fbrd) {
    u64 n;
    MMIO32(u->base + UART_CR) = 0;
    for (n = 0; n < WAIT_LIMIT && (MMIO32(u->base + UART_FR) & FR_BUSY); n++) { }
    MMIO32(u->base + UART_IMSC) = 0;
    MMIO32(u->base + UART_ICR) = 0x7ff;
    MMIO32(u->base + UART_IBRD) = ibrd;
    MMIO32(u->base + UART_FBRD) = fbrd;
    MMIO32(u->base + UART_LCRH) = 0x70;                 /* 8 bits, no parity, 1 stop, FIFO */
    MMIO32(u->base + UART_CR) = 0x301;                  /* enable, transmit, receive */
}

static void uart_putc(struct pl011 *u, char c) {
    u64 n;
    if (!u->up) return;
    for (n = 0; n < WAIT_LIMIT && (MMIO32(u->base + UART_FR) & FR_TXFF); n++) { }
    if (n == WAIT_LIMIT) {
        if (++u->stalls >= 4) u->up = 0;                /* a line that never drains is dropped */
        return;
    }
    u->stalls = 0;
    MMIO32(u->base + UART_DR) = (u8)c;
}

static int uart_getc(struct pl011 *u) {
    u32 d;
    if (!u->up) return -1;
    if (MMIO32(u->base + UART_FR) & FR_RXFE) return -1;
    d = MMIO32(u->base + UART_DR);
    if (d & 0xf00) return -1;                           /* framing, parity, break, overrun */
    return (int)(d & 0xff);
}

/* ---- firmware mailbox and framebuffer -------------------------------- */

#define MBOX_BASE      0x107c013880UL
#define MBOX0_READ     (MBOX_BASE + 0x00)
#define MBOX0_STATUS   (MBOX_BASE + 0x18)
#define MBOX1_WRITE    (MBOX_BASE + 0x20)
#define MBOX1_STATUS   (MBOX_BASE + 0x38)
#define MBOX_EMPTY     0x40000000u
#define MBOX_FULL      0x80000000u
#define MBOX_PROPERTY  8u

static u32 __attribute__((aligned(16))) mb[40];

static int mbox_call(void) {
    u32 addr = (((u32)(u64)mb) & ~0xc0000000u) | 0xc0000000u;
    u64 n;
    u32 r;
    for (n = 0; n < 64 && !(MMIO32(MBOX0_STATUS) & MBOX_EMPTY); n++) (void)MMIO32(MBOX0_READ);
    for (n = 0; n < WAIT_LIMIT && (MMIO32(MBOX1_STATUS) & MBOX_FULL); n++) { }
    if (n == WAIT_LIMIT) return 0;
    MMIO32(MBOX1_WRITE) = addr | MBOX_PROPERTY;
    for (;;) {
        for (n = 0; n < WAIT_LIMIT && (MMIO32(MBOX0_STATUS) & MBOX_EMPTY); n++) { }
        if (n == WAIT_LIMIT) return 0;
        r = MMIO32(MBOX0_READ);
        if ((r & 0xf) == MBOX_PROPERTY)
            return (r & ~0xfu) == addr && mb[1] == 0x80000000u;
    }
}

static struct {
    int  up;
    u8  *base;
    u32  width, height, pitch, depth;
    u32  cols, rows, col, row;
} fb;

#define CELL 16                     /* an 8x8 glyph drawn at twice its size */

static void fb_pixel(u32 x, u32 y, int on) {
    u8 *p = fb.base + (u64)y * fb.pitch;
    if (fb.depth == 32) ((volatile u32 *)(void *)p)[x] = on ? 0xffe0e8f0u : 0xff101820u;
    else                ((volatile u16 *)(void *)p)[x] = on ? 0xe75eu : 0x10c4u;
}

static void fb_clear(void) {
    u32 x, y;
    for (y = 0; y < fb.height; y++)
        for (x = 0; x < fb.width; x++) fb_pixel(x, y, 0);
    fb.col = 0;
    fb.row = 0;
}

static void fb_glyph(u32 col, u32 row, u8 c) {
    const unsigned char *g = font8x8[c < 128 ? c : '?'];
    u32 gx, gy;
    for (gy = 0; gy < CELL; gy++)
        for (gx = 0; gx < CELL; gx++)
            fb_pixel(col * CELL + gx, row * CELL + gy, (g[gy >> 1] >> (gx >> 1)) & 1);
}

/* Shift the visible surface up by one character-cell row and clear the
 * vacated bottom row with the background colour.  The MMU and data cache
 * are off, so this is an uncached byte copy of the whole framebuffer minus
 * one row; expect ~10–20 ms on a 1920×1080 surface at cold-memory speeds.
 * That is acceptable for a terminal but would be wrong for animation. */
static void fb_scroll(void) {
    u64 row_bytes = (u64)CELL * fb.pitch;
    u64 copy_sz   = (u64)(fb.rows - 1) * row_bytes;
    u8  *last_row = fb.base + (u64)(fb.rows - 1) * row_bytes;
    u64  i;

    memcpy(fb.base, fb.base + row_bytes, copy_sz);

    if (fb.depth == 32) {
        volatile u32 *p = (volatile u32 *)(void *)last_row;
        for (i = 0; i < (u64)CELL * fb.width; i++) p[i] = 0xff101820u;
    } else {
        volatile u16 *p = (volatile u16 *)(void *)last_row;
        for (i = 0; i < (u64)CELL * fb.width; i++) p[i] = 0x10c4u;
    }
}

static void fb_putc(char c) {
    if (!fb.up || c == '\r') return;
    if (c != '\n') {
        fb_glyph(fb.col, fb.row, (u8)c);
        if (++fb.col < fb.cols) return;
    }
    fb.col = 0;
    if (++fb.row >= fb.rows) {
        fb_scroll();
        fb.row = fb.rows - 1;   /* stay on the last row; scroll brought it blank */
    }
}

static u32 firmware_revision;

static int fb_setup(void) {
    u32 w = 0, h = 0, i;

    mb[0] = 7 * 4; mb[1] = 0;
    mb[2] = 0x00000001; mb[3] = 4; mb[4] = 0; mb[5] = 0;        /* firmware revision */
    mb[6] = 0;
    if (mbox_call()) firmware_revision = mb[5];

    mb[0] = 8 * 4; mb[1] = 0;
    mb[2] = 0x00040003; mb[3] = 8; mb[4] = 0; mb[5] = 0; mb[6] = 0;     /* display size */
    mb[7] = 0;
    if (mbox_call()) { w = mb[5]; h = mb[6]; }
    if (w < 640 || w > 4096 || h < 480 || h > 2160) { w = 1024; h = 768; }

    i = 0;
    mb[i++] = 0; mb[i++] = 0;
    mb[i++] = 0x00048003; mb[i++] = 8; mb[i++] = 8; mb[i++] = w; mb[i++] = h;    /* physical size */
    mb[i++] = 0x00048004; mb[i++] = 8; mb[i++] = 8; mb[i++] = w; mb[i++] = h;    /* virtual size  */
    mb[i++] = 0x00048005; mb[i++] = 4; mb[i++] = 4; mb[i++] = 32;                /* depth         */
    mb[i++] = 0x00048009; mb[i++] = 8; mb[i++] = 8; mb[i++] = 0; mb[i++] = 0;    /* offset        */
    mb[i++] = 0x00040001; mb[i++] = 8; mb[i++] = 4; mb[i++] = 0; mb[i++] = 0;    /* allocate      */
    mb[i++] = 0x00040008; mb[i++] = 4; mb[i++] = 0; mb[i++] = 0;                 /* pitch         */
    mb[i++] = 0;
    mb[0] = i * 4;
    if (!mbox_call()) return 0;

    fb.width  = mb[5];
    fb.height = mb[6];
    fb.depth  = mb[15];
    fb.base   = (u8 *)(u64)(mb[24] & 0x3fffffffu);
    fb.pitch  = mb[29];
    if (!fb.base || !fb.width || !fb.height) return 0;
    if (fb.depth != 32 && fb.depth != 16) return 0;
    if (!fb.pitch) fb.pitch = fb.width * (fb.depth >> 3);
    fb.cols = fb.width / CELL;
    fb.rows = fb.height / CELL;
    fb.up = 1;
    fb_clear();
    return 1;
}

/* ---- console / diagnostic sinks -------------------------------------- */

/* Text written before a surface is up is kept and replayed on it. */
static char early[4096];
static u64  early_len;
static int  panel_mode;     /* HDMI paints only via diag_refresh */

static void surface_out(int which, char c) {
    if (which == 1) uart_putc(&debug_uart, c);
    else if (which == 2) fb_putc(c);
    else uart_putc(&header_uart, c);
}

static void replay(int which) {
    u64 i;
    for (i = 0; i < early_len; i++) surface_out(which, early[i]);
}

/* Independent backends: a stall or missing surface must not stop others. */
void diag_sink_putc(unsigned sinks, char c) {
    if (sinks & DIAG_SINK_UART_DEBUG) uart_putc(&debug_uart, c);
    if (sinks & DIAG_SINK_UART_GPIO)  uart_putc(&header_uart, c);
    if (sinks & DIAG_SINK_HDMI)       fb_putc(c);
}

void diag_sink_write(unsigned sinks, const char *s, u64 n) {
    while (n--) {
        char c = *s++;
        if (c == '\n') diag_sink_putc(sinks, '\r');
        diag_sink_putc(sinks, c);
    }
}

void diag_hdmi_clear(void) {
    if (fb.up) fb_clear();
}

static void raw_putc(char c) {
    if (early_len < sizeof early) early[early_len++] = c;
    /* UARTs always get the stream. HDMI switches to panel mode once the
     * commissioning console owns the screen. */
    uart_putc(&debug_uart, c);
    uart_putc(&header_uart, c);
    if (!panel_mode) fb_putc(c);
}

void con_putc(char c) {
    if (c == '\n') raw_putc('\r');
    raw_putc(c);
}

void con_write(const char *s, u64 n) {
    while (n--) con_putc(*s++);
}

void con_puts(const char *s) {
    while (*s) con_putc(*s++);
}

void con_hex(u64 v) {
    char t[20];
    struct kbuf b;
    kbuf_init(&b, t, sizeof t);
    kbuf_hex(&b, v);
    con_write(t, b.len);
}

void con_dec(u64 v) {
    char t[24];
    struct kbuf b;
    kbuf_init(&b, t, sizeof t);
    kbuf_dec(&b, v);
    con_write(t, b.len);
}

int con_getc(void) {
    int c = uart_getc(&header_uart);
    if (c < 0) c = uart_getc(&debug_uart);
    return c;
}

/* ---- counter, memory, stop ------------------------------------------- */

u64 board_counter(void) {
    u64 v;
    __asm__ volatile("mrs %0, cntpct_el0" : "=r"(v));
    return v;
}

u64 board_counter_hz(void) {
    u64 v;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(v));
    return v ? v : 54000000UL;
}

/* 32 MiB starting at 16 MiB. The image sits at 0x80000 and is under
 * 1 MiB; the firmware's framebuffer and device tree are reported at boot
 * so an overlap with this range would be visible. */
#define KMEM_BASE   0x01000000UL
#define KMEM_LENGTH 0x02000000UL

u8 *board_memory(u64 *length) {
    *length = KMEM_LENGTH;
    return (u8 *)KMEM_BASE;
}

/* Firmware mailbox tag 0x00030006 (GET_TEMPERATURE): temperature id 0 is
 * the SoC; the value is milli-degrees Celsius. Source: Raspberry Pi
 * firmware mailbox property interface. */
#define MBOX_GET_TEMPERATURE 0x00030006u
#define THERMAL_WARN_mC      75000LL
#define THERMAL_FAULT_mC     85000LL

void board_stop(void);

s64 board_temperature_mC(void) {
    mb[0] = 8 * 4;
    mb[1] = 0;
    mb[2] = MBOX_GET_TEMPERATURE;
    mb[3] = 8;
    mb[4] = 4;
    mb[5] = 0;          /* temperature id: SoC */
    mb[6] = 0;          /* value (response) */
    mb[7] = 0;
    if (!mbox_call()) return -1;
    if (mb[5] != 0) return -1;
    return (s64)(u32)mb[6];
}

static void temp_show(s64 mC) {
    u64 whole, frac;
    if (mC < 0) {
        con_puts("unavailable");
        return;
    }
    whole = (u64)mC / 1000;
    frac  = ((u64)mC % 1000) / 100;
    con_dec(whole);
    con_putc('.');
    con_dec(frac);
    con_puts(" C");
}

static void board_thermal_report(const char *when) {
    s64 mC = board_temperature_mC();
    char detail[40];
    struct kbuf b;

    con_puts("temperature");
    if (when) {
        con_puts(" (");
        con_puts(when);
        con_puts(")");
    }
    con_puts(": ");
    temp_show(mC);
    con_putc('\n');

    if (mC < 0) {
        diag_set(DIAG_TEMP, DIAG_FAIL, "mailbox no answer");
        diag_set(DIAG_THERMAL, DIAG_FAIL, "no temperature");
        diag_failing("mailbox_get_temperature");
        return;
    }
    kbuf_init(&b, detail, sizeof detail);
    kbuf_dec(&b, (u64)mC / 1000);
    kbuf_putc(&b, '.');
    kbuf_dec(&b, ((u64)mC % 1000) / 100);
    kbuf_put(&b, " C");
    diag_set(DIAG_TEMP, DIAG_OK, detail);

    if (mC >= THERMAL_FAULT_mC) {
        con_puts("thermal fault: SoC at or above 85 C. Stopping.\n");
        diag_set(DIAG_THERMAL, DIAG_FAIL, "FAULT >=85C");
        diag_failing("thermal_fault");
        board_stop();
    }
    if (mC >= THERMAL_WARN_mC) {
        con_puts("thermal warning: SoC at or above 75 C. Fan is at commissioning max.\n");
        diag_set(DIAG_THERMAL, DIAG_OK, "WARN >=75C fan max");
    } else {
        diag_set(DIAG_THERMAL, DIAG_OK, "armed 75/85");
    }
    diag_stage("thermal");
}

void board_idle(void) {
    static u64 next_thermal;
    u64 now = board_counter();
    u64 until;

    /* Every ten seconds: temperature on the UART without flooding it. */
    if (next_thermal == 0 || now >= next_thermal) {
        next_thermal = now + board_counter_hz() * 10;
        board_thermal_report("idle");
    }

    /* Interrupts are masked and the GIC is not up, so WFI would not return
     * on a timer. A short yield wait cuts continuous busy-polling without
     * depending on an event that never arrives. */
    until = board_counter() + board_counter_hz() / 1000;
    while (board_counter() < until)
        __asm__ volatile("yield" ::: "memory");
}

void board_stop(void) {
    __asm__ volatile("msr daifset, #0xf");
    for (;;) __asm__ volatile("wfe");
}

/* ---- exceptions ------------------------------------------------------ */

u64 exception_c(u64 slot, u64 esr, u64 elr, u64 far) {
    if (probe_armed) {
        probe_hit = 1;
        return (slot & 3) == 1 ? 4 : 0;         /* synchronous: step over the access */
    }
    con_puts("\nexception: slot ");
    con_dec(slot);
    con_puts("  esr ");
    con_hex(esr);
    con_puts("  elr ");
    con_hex(elr);
    con_puts("  far ");
    con_hex(far);
    con_puts("\nkyrios is stopped.\n");
    board_stop();
    return 0;
}

/* ---- bring-up -------------------------------------------------------- */

#define RP1_GPIO_CTRL(pin)  (0x1f000d0000UL + (u64)(pin) * 8 + 4)
#define RP1_PADS(pin)       (0x1f000f0000UL + 4 + (u64)(pin) * 4)

static void show_reg(const char *name, u32 v) {
    con_puts(name);
    con_hex(v);
}

static void header_uart_bringup(void) {
    u32 cr = 0, v;
    int stale = 0, answered;

    diag_failing("header_uart_probe");
    diag_set(DIAG_UART_GPIO, DIAG_DETECTING, "RP1 UART0");
    con_puts("header uart (pins 8 and 10): asking RP1 ... ");
    answered = probe_read32(header_uart.base + UART_CR, &cr, &stale);
    if (stale) con_puts("(an SError was already waiting; discarded) ");
    if (!answered || cr == 0xffffffffu) {
        con_puts("no answer. Not used.\n"
                 "  (needs enable_rp1_uart=1 in config.txt and a bootloader that knows it)\n");
        diag_set(DIAG_UART_GPIO, DIAG_FAIL, "RP1 UART0 no answer");
        diag_set(DIAG_RP1, DIAG_FAIL, "UART window silent");
        return;
    }
    diag_set(DIAG_RP1, DIAG_OK, "MMIO window answers");
    if (cr & 1) {
        con_puts("set up by the firmware.\n");
    } else {
        /* The firmware did not set it up. 115200 from a 50 MHz clock.
         * This path is a guess at what the firmware would have done. */
        con_puts("answers, but not set up by the firmware. Setting up here.\n");
        uart_setup(&header_uart, 27, 8);
    }

    /* Whoever set it up: receive must be on, and pins 14 and 15 must be
     * given to UART0 (function 4), or nothing typed will arrive. */
    cr = MMIO32(header_uart.base + UART_CR);
    if ((cr & 0x301) != 0x301) {
        MMIO32(header_uart.base + UART_CR) = cr | 0x301;
        con_puts("  transmit and receive switched on\n");
    }
    v = MMIO32(RP1_GPIO_CTRL(14));
    if ((v & 0x1f) != 4) {
        MMIO32(RP1_GPIO_CTRL(14)) = (v & ~0xf01fu) | 4;
        MMIO32(RP1_PADS(14)) = (MMIO32(RP1_PADS(14)) & ~0xccu) | 0x40;      /* no pulls, output on */
        con_puts("  pin 14 given to UART0\n");
    }
    v = MMIO32(RP1_GPIO_CTRL(15));
    if ((v & 0x1f) != 4) {
        MMIO32(RP1_GPIO_CTRL(15)) = (v & ~0xf01fu) | 4;
        MMIO32(RP1_PADS(15)) = (MMIO32(RP1_PADS(15)) & ~0x84u) | 0x48;      /* input on, pull up */
        con_puts("  pin 15 given to UART0\n");
    }
    header_uart.up = 1;
    replay(3);

    show_reg("  cr ", MMIO32(header_uart.base + UART_CR));
    show_reg("  ibrd ", MMIO32(header_uart.base + UART_IBRD));
    show_reg("  fbrd ", MMIO32(header_uart.base + UART_FBRD));
    show_reg("  lcrh ", MMIO32(header_uart.base + UART_LCRH));
    show_reg("  pin14 ", MMIO32(RP1_GPIO_CTRL(14)));
    show_reg("  pin15 ", MMIO32(RP1_GPIO_CTRL(15)));
    con_puts("\n");
    diag_set(DIAG_UART_GPIO, DIAG_OK, "115200 8N1 pins 8/10");
    diag_stage("gpio_uart");
}

/* ---- RP1 GPIO output --------------------------------------------------- */

/* Where the values come from: raspberrypi/linux (rpi-6.12.y)
 * drivers/pinctrl/pinctrl-rp1.c and rp1.dtsi. RP1's 54 pins sit in three
 * banks starting at pins 0, 28 and 34; each bank has its own GPIO, RIO
 * and pads blocks at offsets 0x0000, 0x4000 and 0x8000 from the bank
 * bases (0x1f000d0000, 0x1f000e0000, 0x1f000f0000 from the processor).
 * A pin is a GPIO output when its control register's function field is
 * 5 with the OUTOVER and OEOVER fields at 0 (peripheral), its pad's
 * OUT_DISABLE bit is clear, its RIO output value is set, and its RIO
 * output-enable bit is set. RIO value and enable writes at +0x2000 set
 * bits and at +0x3000 clear them. */
static const struct { u32 first, count, off; } rp1_banks[3] = {
    { 0, 28, 0x0000 }, { 28, 6, 0x4000 }, { 34, 20, 0x8000 }
};

#define RP1_GPIO_BASE   0x1f000d0000UL
#define RP1_RIO_BASE    0x1f000e0000UL
#define RP1_PADS2_BASE  0x1f000f0000UL
#define RIO_SET         0x2000
#define RIO_CLR         0x3000

void board_pin_write(int pin, int value) {
    int b;
    u32 off, bit, ctrl;
    /* These are above 4 GiB. A 32-bit variable drops the 0x1f prefix and
     * the write lands in low memory, not on the pin. */
    u64 ctrl_addr, pad_addr, rio;

    for (b = 0; b < 3; b++)
        if (pin >= (int)rp1_banks[b].first &&
            pin < (int)(rp1_banks[b].first + rp1_banks[b].count)) break;
    if (b == 3) return;
    off = (u32)pin - rp1_banks[b].first;
    bit = 1u << off;
    ctrl_addr = RP1_GPIO_BASE + rp1_banks[b].off + (u64)off * 8 + 4;
    pad_addr  = RP1_PADS2_BASE + rp1_banks[b].off + 4 + (u64)off * 4;
    rio       = RP1_RIO_BASE + rp1_banks[b].off;

    /* The pad drives out; no pulls. */
    MMIO32(pad_addr) = MMIO32(pad_addr) & ~0x8cu;
    /* The value first, then the function, then output enable. All three
     * override fields are cleared: an OUTOVER left by the firmware
     * would otherwise hold the pin where it was, whatever RIO says. */
    MMIO32(rio + (value ? RIO_SET : RIO_CLR)) = bit;
    ctrl = MMIO32(ctrl_addr);
    MMIO32(ctrl_addr) = (ctrl & ~0xf01fu) | 5u;   /* GPIO, OUTOVER/OEOVER peripheral */
    MMIO32(rio + 4 + RIO_SET) = bit;
}

/* The same pin read back: its control register and the level on it. */
static void rp1_pin_show(int pin) {
    int b;
    u32 off;
    u64 ctrl_addr, rio;

    for (b = 0; b < 3; b++)
        if (pin >= (int)rp1_banks[b].first &&
            pin < (int)(rp1_banks[b].first + rp1_banks[b].count)) break;
    if (b == 3) return;
    off = (u32)pin - rp1_banks[b].first;
    ctrl_addr = RP1_GPIO_BASE + rp1_banks[b].off + (u64)off * 8 + 4;
    rio       = RP1_RIO_BASE + rp1_banks[b].off;

    con_puts("  gpio ");
    con_dec((u64)(u32)pin);
    show_reg(" ctrl", MMIO32(ctrl_addr));
    show_reg("  rio out", MMIO32(rio));
    show_reg("  rio oe", MMIO32(rio + 4));
    show_reg("  rio in", MMIO32(rio + 8));
    con_putc('\n');
}

/* Wait by the hardware counter. */
static void delay_ms(u64 ms) {
    u64 until = board_counter() + board_counter_hz() / 1000 * ms;
    while (board_counter() < until) { }
}

/* ---- fan: the Pi 5 fan header --------------------------------------- */

/* bcm2712-rpi-5-b.dts names gpio 45 FAN_PWM and gpio 29 FAN_TACH, and
 * its cooling_fan node is rp1_pwm1 channel 3, period 41566 ns, polarity
 * inverted, with the rpm count read back at register offset 0x3c.
 * rp1.dtsi puts that PWM block at 0x4009c000 (0x1f0009c000 from here)
 * and assigns its clock 50 MHz. pinctrl-rp1.c lists pwm1 as function 0
 * on gpio 45, with a pull down. clk-rp1.c gives CLK_PWM1 at clock-block
 * offsets 0x84/0x88/0x8c; its parents are auxiliary, and xosc (50 MHz)
 * is parent 2, so a divider of 1 is the assigned rate. pwm-rp1.c gives
 * the channel registers and the enable bit. */
#define FAN_PWM_PIN   45
#define FAN_TACH_PIN  29
#define PWM1_BASE     0x1f0009c000UL
#define CLK_PWM1_CTRL 0x1f00018084UL
#define CLK_PWM1_DIV  0x1f00018088UL
#define CLK_PWM1_FRAC 0x1f0001808cUL
#define PWM_CH_CTRL   0x044u   /* channel 3: 0x014 + 3*16 */
#define PWM_CH_RANGE  0x048u
#define PWM_CH_DUTY   0x050u
#define PWM_RPM       0x03cu
#define FAN_RANGE     2078u    /* 50 MHz * 41566 ns */
#define FAN_DUTY      2037u    /* range * 250/255, linux's fastest step */

static int rp1_pin_where(int pin, u64 *ctrl, u64 *pad, u64 *rio, u32 *bit) {
    int b;
    u32 off;

    for (b = 0; b < 3; b++)
        if (pin >= (int)rp1_banks[b].first &&
            pin < (int)(rp1_banks[b].first + rp1_banks[b].count)) break;
    if (b == 3) return 0;
    off = (u32)pin - rp1_banks[b].first;
    *bit  = 1u << off;
    *ctrl = RP1_GPIO_BASE + rp1_banks[b].off + (u64)off * 8 + 4;
    *pad  = RP1_PADS2_BASE + rp1_banks[b].off + 4 + (u64)off * 4;
    *rio  = RP1_RIO_BASE + rp1_banks[b].off;
    return 1;
}

static void board_fan_start(void) {
    u32 ctrl, div, rpm0, rpm1, level, prev, edges, bit;
    u64 pwm_ctrl, pwm_pad, tach_ctrl, tach_pad, rio, until;
    int stale = 0;
    char detail[40];
    struct kbuf b;

    diag_failing("fan_pwm_clk");
    diag_set(DIAG_FAN_PWM, DIAG_INITIALIZING, "gpio45 pwm1 ch3");
    diag_set(DIAG_FAN_RPM, DIAG_DETECTING, "gpio29 tach");
    con_puts("fan: pwm on gpio 45, tach on gpio 29\n");
    if (!probe_read32(CLK_PWM1_CTRL, &ctrl, &stale) ||
        !probe_read32(CLK_PWM1_DIV, &div, &stale)) {
        con_puts("  clk_pwm1: no answer\n");
        diag_set(DIAG_FAN_PWM, DIAG_FAIL, "clk_pwm1 no answer");
        diag_set(DIAG_FAN_RPM, DIAG_UNKNOWN, "pwm failed");
        diag_failing("fan_pwm_clk");
        return;
    }
    show_reg("  clk_pwm1 ctrl", ctrl);
    show_reg("  div", div);
    con_putc('\n');

    /* Parent 2 is xosc. AUXSRC sits at bits 5-9; bit 11 enables. */
    MMIO32(CLK_PWM1_DIV) = 1;
    MMIO32(CLK_PWM1_FRAC) = 0;
    ctrl = (ctrl & ~0x3e0u) | (2u << 5) | (1u << 11);
    MMIO32(CLK_PWM1_CTRL) = ctrl;
    delay_ms(1);
    show_reg("  clk_pwm1 now", MMIO32(CLK_PWM1_CTRL));
    show_reg("  div", MMIO32(CLK_PWM1_DIV));
    con_puts("\n");

    if (!rp1_pin_where(FAN_PWM_PIN, &pwm_ctrl, &pwm_pad, &rio, &bit) ||
        !rp1_pin_where(FAN_TACH_PIN, &tach_ctrl, &tach_pad, &rio, &bit)) {
        con_puts("  fan pins are not on this gpio map\n");
        diag_set(DIAG_FAN_PWM, DIAG_FAIL, "pin map");
        diag_set(DIAG_FAN_RPM, DIAG_FAIL, "pin map");
        diag_failing("fan_pin_map");
        return;
    }

    /* PWM pin: function 0 (pwm1), pad drives, pull down, overrides left
     * to the peripheral. Tach pin: a GPIO input, pull up, pad output off. */
    MMIO32(pwm_pad) = (MMIO32(pwm_pad) & ~0x8cu) | 0x04u;
    MMIO32(pwm_ctrl) = MMIO32(pwm_ctrl) & ~0xf01fu;
    MMIO32(tach_pad) = (MMIO32(tach_pad) & ~0x8cu) | 0xc8u;
    MMIO32(tach_ctrl) = (MMIO32(tach_ctrl) & ~0xf01fu) | 5u | (2u << 14);
    MMIO32(rio + RIO_CLR + 4) = bit;

    /* Trailing-edge mode, polarity inverted, then the period and duty,
     * then channel 3's enable, then the update bit. Same order as
     * pwm-rp1.c. */
    MMIO32(PWM1_BASE + PWM_CH_CTRL) = (1u << 8) | (1u << 3) | (1u << 0);
    MMIO32(PWM1_BASE + PWM_CH_DUTY) = FAN_DUTY;
    MMIO32(PWM1_BASE + PWM_CH_RANGE) = FAN_RANGE;
    ctrl = MMIO32(PWM1_BASE) | (1u << 3);
    MMIO32(PWM1_BASE) = ctrl;
    MMIO32(PWM1_BASE) = ctrl | (1u << 31);
    __asm__ volatile("dsb sy" ::: "memory");

    con_puts("  pwm1 ch3 range ");
    con_dec(FAN_RANGE);
    con_puts(" duty ");
    con_dec(FAN_DUTY);
    con_puts(" (250/255 of that range)\n");
    show_reg("  gpio 45 ctrl", MMIO32(pwm_ctrl));
    show_reg("  gpio 29 ctrl", MMIO32(tach_ctrl));
    con_putc('\n');

    rpm0 = MMIO32(PWM1_BASE + PWM_RPM);
    prev = (MMIO32(rio + 8) & bit) ? 1u : 0u;
    edges = 0;
    until = board_counter() + board_counter_hz() / 2;
    while (board_counter() < until) {
        level = (MMIO32(rio + 8) & bit) ? 1u : 0u;
        if (level && !prev) edges++;
        prev = level;
    }
    rpm1 = MMIO32(PWM1_BASE + PWM_RPM);
    show_reg("  rpm reg", rpm0);
    show_reg(" then", rpm1);
    con_puts("\n  tach edges in 500 ms: ");
    con_dec(edges);
    con_putc('\n');

    /* 2 pulses/rev typical; edges in 0.5 s → RPM = edges * 60. */
    kbuf_init(&b, detail, sizeof detail);
    kbuf_dec(&b, (u64)edges * 60u);
    kbuf_put(&b, " est; reg ");
    kbuf_hex(&b, rpm1);
    if (edges == 0 && rpm0 == rpm1)
        diag_set(DIAG_FAN_RPM, DIAG_FAIL, detail);
    else
        diag_set(DIAG_FAN_RPM, DIAG_OK, detail);
    diag_set(DIAG_FAN_PWM, DIAG_OK, "max commissioning duty");
    diag_stage("fan");

    /* Temperature after the fan is commanded, before the application layer. */
    board_thermal_report("after fan");
}

/* ---- Ethernet: RP1's Cadence GEM ------------------------------------- */

/* RP1's Ethernet is a Cadence GEM ("cdns,macb"). Where the values come
 * from: raspberrypi/linux (rpi-6.12.y) rp1.dtsi puts the controller at
 * RP1 address 0x40100000, which is 0x1f00100000 from the processor;
 * bcm2712-rpi-5-b.dts puts the PHY at MDIO address 1; drivers/net/
 * ethernet/cadence/macb.h gives the register offsets and bit fields.
 * Register reads go through probe_read32, so a controller that is not
 * there is reported, not faulted on. Writes: the PHY reset pin is
 * driven, the DMA queues are set up and enabled, and one test frame
 * is transmitted at bring-up. */
#define ETH_BASE    0x1f00100000UL
#define ETH_NCR     0x0000
#define ETH_NCFGR   0x0004
#define ETH_NSR     0x0008
#define ETH_DMACFG  0x0010
#define ETH_TSR     0x0014
#define ETH_RBQP    0x0018
#define ETH_TBQP    0x001c
#define ETH_RSR     0x0020
#define ETH_ISR     0x0024
#define ETH_IDR     0x002c
#define ETH_MAN     0x0034
#define ETH_SA1B    0x0098
#define ETH_SA1T    0x009c
#define ETH_MID     0x00fc
#define ETH_TBQPH   0x04c8
#define ETH_RBQPH   0x04d4
#define NSR_LINK    (1u << 0)
#define NSR_IDLE    (1u << 2)
#define NCR_MPE     (1u << 4)

static int eth_wait_idle(void) {
    u64 n;
    for (n = 0; n < WAIT_LIMIT; n++)
        if (MMIO32(ETH_BASE + ETH_NSR) & NSR_IDLE) return 1;
    return 0;
}

/* One clause-22 MDIO read. -1 when the MDIO port never went idle. */
static int eth_phy_read_at(u32 addr, u32 reg) {
    MMIO32(ETH_BASE + ETH_MAN) =
        (1u << 30) | (2u << 28) | (addr << 23) | (reg << 18) | (2u << 16);
    if (!eth_wait_idle()) return -1;
    return (int)(MMIO32(ETH_BASE + ETH_MAN) & 0xffffu);
}

static int eth_phy_read(u32 reg) { return eth_phy_read_at(1, reg); }

/* ---- the driver proper: DMA queues, receive, transmit ----------------- */

/* The GEM's DMA reaches the processor's RAM through PCIe2's inbound
 * window: bcm2712.dtsi (pcie2 dma-ranges) maps RP1 bus address
 * 0x10_0000_0000 to RAM address 0. So every DMA address below is the
 * buffer's own address plus 0x10_0000_0000, which needs the GEM's
 * 64-bit descriptor form: {address low, control, address high, 0}
 * (macb.h, struct macb_dma_desc + macb_dma_desc_64). The caches are
 * off, so no cache maintenance is needed between the processor and
 * the GEM. The assumption in this paragraph is the unproven part;
 * the register and descriptor readings in the report are how it is
 * checked on hardware. */
#define ETH_DMA(p)      ((u64)(p) + 0x1000000000ULL)
#define ETH_RXN         16
#define ETH_TXN         8
#define ETH_BUFSZ       2048

struct eth_desc { u32 addr, ctrl, addrh, resvd; };

static struct eth_desc eth_rxq[ETH_RXN] __attribute__((aligned(32)));
static struct eth_desc eth_txq[ETH_TXN] __attribute__((aligned(32)));
static u8 eth_rxbuf[ETH_RXN][ETH_BUFSZ] __attribute__((aligned(32)));
static u8 eth_txbuf[ETH_TXN][ETH_BUFSZ] __attribute__((aligned(32)));

static u8  eth_mac[6];
static int eth_inited;
static u64 eth_rx_seen, eth_tx_sent;

static void eth_hex2(u32 v) {
    static const char *d = "0123456789abcdef";
    char t[3];
    t[0] = d[(v >> 4) & 15]; t[1] = d[v & 15]; t[2] = 0;
    con_puts(t);
}

static void eth_show_mac(const u8 *m) {
    int i;
    for (i = 0; i < 6; i++) {
        if (i) con_putc(':');
        eth_hex2(m[i]);
    }
}

static void eth_mac_address(void) {
    /* The board's MAC address from the firmware mailbox (property tag
     * 0x00010003). The fallback is a locally administered address made
     * here, so the board never transmits as all-zero. */
    mb[0] = 8 * 4; mb[1] = 0;
    mb[2] = 0x00010003; mb[3] = 8; mb[4] = 0; mb[5] = 0; mb[6] = 0;
    mb[7] = 0;
    if (mbox_call() && (mb[5] || mb[6])) {
        eth_mac[0] = (u8)(mb[5]);       eth_mac[1] = (u8)(mb[5] >> 8);
        eth_mac[2] = (u8)(mb[5] >> 16); eth_mac[3] = (u8)(mb[5] >> 24);
        eth_mac[4] = (u8)(mb[6]);       eth_mac[5] = (u8)(mb[6] >> 8);
        return;
    }
    eth_mac[0] = 0x02; eth_mac[1] = 0x6b; eth_mac[2] = 0x79;
    eth_mac[3] = 0x72; eth_mac[4] = 0x6f; eth_mac[5] = 0x73;
}

static void eth_queues_init(int gigabit, int fast) {
    u32 cfg;
    int i;

    MMIO32(ETH_BASE + ETH_IDR) = 0xffffffffu;      /* polled, no interrupts */
    MMIO32(ETH_BASE + ETH_NCR) = 0;

    for (i = 0; i < ETH_RXN; i++) {
        u64 d = ETH_DMA(eth_rxbuf[i]);
        eth_rxq[i].addr  = ((u32)d & ~3u) | (i == ETH_RXN - 1 ? 2u : 0u);
        eth_rxq[i].ctrl  = 0;
        eth_rxq[i].addrh = (u32)(d >> 32);
        eth_rxq[i].resvd = 0;
    }
    for (i = 0; i < ETH_TXN; i++) {
        u64 d = ETH_DMA(eth_txbuf[i]);
        eth_txq[i].addr  = (u32)d;
        eth_txq[i].ctrl  = (1u << 31) | (i == ETH_TXN - 1 ? (1u << 30) : 0u);
        eth_txq[i].addrh = (u32)(d >> 32);
        eth_txq[i].resvd = 0;
    }

    /* DMA: INCR16 bursts, 1536-byte receive buffers (RXBS in 64-byte
     * units), full packet-buffer memory, 64-bit addressing. */
    MMIO32(ETH_BASE + ETH_DMACFG) =
        16u | (3u << 8) | (1u << 10) | (24u << 16) | (1u << 30);

    cfg = MMIO32(ETH_BASE + ETH_NCFGR);
    cfg &= ~(7u << 18);  cfg |= (5u << 18);        /* MDIO clock /96, as the firmware had it */
    cfg |= (1u << 1) | (1u << 4) | (1u << 8);      /* full duplex, copy all frames, 1536-byte frames */
    if (gigabit)   cfg |= (1u << 10);              /* GBE */
    else if (fast) cfg |= (1u << 0);               /* SPD: 100 */
    MMIO32(ETH_BASE + ETH_NCFGR) = cfg;

    MMIO32(ETH_BASE + ETH_SA1B) =
        (u32)eth_mac[0] | ((u32)eth_mac[1] << 8) |
        ((u32)eth_mac[2] << 16) | ((u32)eth_mac[3] << 24);
    MMIO32(ETH_BASE + ETH_SA1T) =
        (u32)eth_mac[4] | ((u32)eth_mac[5] << 8);

    MMIO32(ETH_BASE + ETH_RBQP)  = (u32)ETH_DMA(eth_rxq);
    MMIO32(ETH_BASE + ETH_RBQPH) = (u32)(ETH_DMA(eth_rxq) >> 32);
    MMIO32(ETH_BASE + ETH_TBQP)  = (u32)ETH_DMA(eth_txq);
    MMIO32(ETH_BASE + ETH_TBQPH) = (u32)(ETH_DMA(eth_txq) >> 32);

    __asm__ volatile("dsb sy");
    MMIO32(ETH_BASE + ETH_NCR) = (1u << 2) | (1u << 3) | (1u << 4);
    eth_inited = 1;

    con_puts("  gem: queues up, receive and transmit enabled, mac ");
    eth_show_mac(eth_mac);
    con_putc('\n');
}

/* Send one broadcast test frame, so the other end of the cable (or a
 * capture on it) can see this board transmit. */
static void eth_send_test(void) {
    static const char msg[] = "kyrios v0.0.0: ethernet test frame";
    u8 *f = eth_txbuf[0];
    u32 len = 60, ncr;
    int i;

    for (i = 0; i < 6; i++) f[i] = 0xff;
    for (i = 0; i < 6; i++) f[6 + i] = eth_mac[i];
    f[12] = 0x88; f[13] = 0xb5;                     /* experimental ethertype */
    for (i = 0; msg[i] && 14 + i < (int)len; i++) f[14 + i] = (u8)msg[i];
    /* The rest of the frame is padding; the buffer starts zeroed. */

    eth_txq[0].addr  = (u32)ETH_DMA(f);
    eth_txq[0].addrh = (u32)(ETH_DMA(f) >> 32);
    eth_txq[0].ctrl  = len | (1u << 15);            /* length, last buffer, used clear */
    __asm__ volatile("dsb sy");

    ncr = MMIO32(ETH_BASE + ETH_NCR);
    MMIO32(ETH_BASE + ETH_NCR) = ncr | (1u << 9);   /* TSTART */
    eth_tx_sent++;
}

/* Report and return to the GEM every frame that has arrived.
 * Returns how many frames that was. */
static int eth_rx_scan(void) {
    int i, n = 0;

    for (i = 0; i < ETH_RXN; i++) {
        u32 len;
        if (!(eth_rxq[i].addr & 1u)) continue;      /* used bit: the GEM owns the result */
        len = eth_rxq[i].ctrl & 0xfffu;
        con_puts("  rx frame: length ");
        con_dec(len);
        con_puts(" from ");
        eth_show_mac(&eth_rxbuf[i][6]);
        con_puts(" ethertype ");
        eth_hex2(eth_rxbuf[i][12]);
        eth_hex2(eth_rxbuf[i][13]);
        con_putc('\n');
        eth_rxq[i].ctrl = 0;
        eth_rxq[i].addr &= ~1u;                     /* give the buffer back */
        __asm__ volatile("dsb sy");
        eth_rx_seen++;
        n++;
    }
    return n;
}

void board_eth_report(void) {
    u32 mid = 0, ncr = 0, ncfgr = 0, nsr = 0;
    int stale = 0, bmsr, b, gigabit = 0, fast = 0;

    diag_failing("ethernet_gem");
    diag_set(DIAG_ETHERNET, DIAG_DETECTING, "RP1 GEM");
    con_puts("ethernet: RP1 GEM at ");
    con_hex(ETH_BASE);
    con_puts(": ");
    if (!probe_read32(ETH_BASE + ETH_MID, &mid, &stale) ||
        mid == 0 || mid == 0xffffffffu) {
        con_puts("no answer.\n");
        diag_set(DIAG_ETHERNET, DIAG_FAIL, "GEM no answer");
        return;
    }
    if (!probe_read32(ETH_BASE + ETH_NCR, &ncr, &stale) ||
        !probe_read32(ETH_BASE + ETH_NCFGR, &ncfgr, &stale) ||
        !probe_read32(ETH_BASE + ETH_NSR, &nsr, &stale)) {
        con_puts("the id register answered and the next read did not.\n");
        return;
    }
    con_puts("answers\n");
    show_reg("  mid", mid);
    show_reg("  ncr", ncr);
    show_reg("  ncfgr", ncfgr);
    show_reg("  nsr", nsr);
    con_puts(nsr & NSR_LINK ? "\n  gem link bit: up\n"
                            : "\n  gem link bit: down\n");

    /* The PHY is held in reset by RP1 GPIO 32, active low
     * (bcm2712-rpi-5-b.dts: phy-reset-gpios, reset duration 5 ms).
     * Pulse it properly: assert, wait, release, wait. */
    if (!eth_inited) {
        /* RP1's clock block (clocks@18000 in rp1.dtsi; clk-rp1.c gives
         * the offsets, and CLK_CTRL_ENABLE is bit 11). Linux enables
         * CLK_ETH (125 MHz) before the GEM is probed. If the firmware
         * left it off, the GEM's registers still answer on the system
         * clock but the MAC, and perhaps its MDIO, has no clock. */
        u32 c_eth = MMIO32(0x1f00018064UL), c_tsu = MMIO32(0x1f00018134UL);
        show_reg("  clk_eth ctrl", c_eth);
        show_reg("  div", MMIO32(0x1f00018068UL));
        show_reg("  sel", MMIO32(0x1f00018070UL));
        show_reg("  clk_eth_tsu ctrl", c_tsu);
        con_putc('\n');
        if (!(c_eth & (1u << 11))) {
            MMIO32(0x1f00018064UL) = c_eth | (1u << 11);
            con_puts("  clk_eth: was off, enabled here\n");
        } else con_puts("  clk_eth: already enabled\n");
        if (!(c_tsu & (1u << 11))) {
            MMIO32(0x1f00018134UL) = c_tsu | (1u << 11);
            con_puts("  clk_eth_tsu: was off, enabled here\n");
        }
        delay_ms(5);

        board_pin_write(32, 0);
        delay_ms(10);
        board_pin_write(32, 1);
        delay_ms(250);
        con_puts("  phy reset (RP1 GPIO 32): pulsed and released\n");
        rp1_pin_show(32);
        show_reg("  dcfg1", MMIO32(ETH_BASE + 0x280));
        show_reg("  dcfg2", MMIO32(ETH_BASE + 0x284));
        con_putc('\n');
    }

    /* MDIO clock: for the GEM the clock field of ncfgr is at bits
     * 18-20 (macb.h GEM_CLK_OFFSET), value 5 is /96, as the firmware
     * had it (the register read above shows 540000). */
    MMIO32(ETH_BASE + ETH_NCFGR) = (ncfgr & ~(7u << 18)) | (5u << 18);

    if (!(ncr & NCR_MPE)) MMIO32(ETH_BASE + ETH_NCR) = ncr | NCR_MPE;
    if (!eth_wait_idle()) {
        con_puts("  phy 1: the MDIO port never went idle\n");
    } else {
        /* Scan the MDIO bus at three clock divisions: any address
         * that answers with something other than all ones or all zero
         * has a PHY on it. The divisions tried are /96, /224 and /32
         * of the GEM's clock (GEM_CLK values 5, 7 and 2). */
        static const u32 divs[3] = { 5, 7, 2 };
        u32 addr;
        int found = 0, di;
        for (di = 0; di < 3 && !found; di++) {
            MMIO32(ETH_BASE + ETH_NCFGR) =
                (MMIO32(ETH_BASE + ETH_NCFGR) & ~(7u << 18)) | (divs[di] << 18);
            con_puts("  mdio scan (clock field ");
            con_dec(divs[di]);
            con_puts("):");
            for (addr = 0; addr < 32; addr++) {
                int v = eth_phy_read_at(addr, 2);
                if (v > 0 && v != 0xffff) {
                    con_puts(" addr ");
                    con_dec(addr);
                    con_puts(" id2 ");
                    con_hex((u64)(u32)v);
                    found = 1;
                }
            }
            con_puts(found ? "\n" : " nothing answered\n");
        }
        int id2 = eth_phy_read(2), id3 = eth_phy_read(3);
        con_puts("  phy 1 id: ");
        con_hex((u64)(u32)id2);
        con_putc(' ');
        con_hex((u64)(u32)id3);
        con_putc('\n');
        (void)eth_phy_read(1);          /* the first read can carry the latched link state */
        bmsr = eth_phy_read(1);
        /* Autonegotiation after a reset release takes seconds. Wait
         * for it, up to about three seconds, so the reading below is
         * the settled one. */
        for (b = 0; b < 30 && bmsr > 0 && bmsr != 0xffff &&
             (bmsr & 0x0024) != 0x0024; b++) {
            int again;
            delay_ms(100);
            again = eth_phy_read(1);
            if (again > 0) bmsr = again;
        }
        if (bmsr < 0) {
            con_puts("  phy 1: no answer to the read\n");
        } else if (bmsr == 0 || bmsr == 0xffff) {
            con_puts("  phy 1: no PHY answering\n");
        } else {
            int anlpar = eth_phy_read(5), gb = eth_phy_read(10);
            con_puts("  phy 1 register 1: ");
            con_hex((u64)(u32)bmsr);
            if (bmsr & 0x0004) con_puts("  link up");
            else               con_puts("  link down");
            if (bmsr & 0x0020) con_puts(", autonegotiation complete");
            else               con_puts(", autonegotiation not complete");
            con_putc('\n');
            if ((bmsr & 0x0024) == 0x0024) {
                /* Partner abilities: register 5 for 10/100, register 10
                 * bits 15 and 14 for 1000 full and half. */
                con_puts("  negotiated: ");
                if (gb > 0 && (gb & 0xc000))       { con_puts(gb & 0x8000 ? "1000 full" : "1000 half"); gigabit = 1; }
                else if (anlpar > 0 && (anlpar & 0x0100)) { con_puts("100 full"); fast = 1; }
                else if (anlpar > 0 && (anlpar & 0x0080)) { con_puts("100 half"); fast = 1; }
                else if (anlpar > 0 && (anlpar & 0x0040)) con_puts("10 full");
                else if (anlpar > 0 && (anlpar & 0x0020)) con_puts("10 half");
                else con_puts("nothing the PHY reports");
                con_putc('\n');
            }
        }
    }
    if (!eth_inited) {
        eth_mac_address();
        eth_queues_init(gigabit, fast);
        eth_send_test();
        delay_ms(5);
        con_puts("  test frame sent\n");
    }
    show_reg("  tsr", MMIO32(ETH_BASE + ETH_TSR));
    show_reg("  rsr", MMIO32(ETH_BASE + ETH_RSR));
    show_reg("  isr", MMIO32(ETH_BASE + ETH_ISR));
    con_putc('\n');
    if (!eth_rx_scan()) con_puts("  rx: no frame yet\n");
    if (nsr & NSR_LINK)
        diag_set(DIAG_ETHERNET, DIAG_OK, "GEM up, link bit set");
    else
        diag_set(DIAG_ETHERNET, DIAG_OK, "GEM up, link down");
    diag_stage("ethernet");
}

void board_eth_poll(void) {
    if (eth_inited) (void)eth_rx_scan();
}

void kyrios_main(void);

/* Explicit stubs: name the missing drivers so the panel never looks done. */
static void board_probe_pcie_nvme_usb(void) {
    u32 mid = 0;
    int stale = 0;

    diag_failing("pcie_rp1_window");
    diag_set(DIAG_PCIE, DIAG_DETECTING, "RP1 via PCIe inbound");
    /* Reachability of the RP1 peripheral window is the only PCIe evidence
     * available without a host-bridge enumerator. */
    if (probe_read32(0x1f00030000UL + 0x30, &mid, &stale) && mid != 0xffffffffu) {
        diag_set(DIAG_PCIE, DIAG_OK, "RP1 window maps; enum ABSENT");
    } else {
        diag_set(DIAG_PCIE, DIAG_FAIL, "RP1 window silent");
    }
    diag_stage("pcie_probe");

    diag_failing("nvme_driver");
    diag_set(DIAG_NVME, DIAG_ABSENT, "no NVMe driver yet");
    diag_stage("nvme_stub");

    diag_failing("usb_driver");
    diag_set(DIAG_USB, DIAG_ABSENT, "no USB driver yet");
    diag_stage("usb_stub");
}

/* Called by kyrios_main() when the commissioning phase is done.
 * Releases the HDMI surface from the diag panel and re-enables scrolling
 * terminal output on it.  After this call every con_putc goes to both
 * UARTs and to the framebuffer, so the HDMI monitor shows the same
 * interactive session as the serial terminal. */
void board_hdmi_terminal(void) {
    if (!fb.up) return;
    diag_hdmi_ready(0);     /* diag_refresh() will no longer touch HDMI */
    panel_mode = 0;
    fb_clear();
}

static void board_store_status(void) {
    u64 length = 0;
    u8 *mem = board_memory(&length);
    char detail[40];
    struct kbuf b;

    diag_failing("document_store");
    diag_set(DIAG_STORE, DIAG_DETECTING, "RAM layout");
    if (!mem || length < 25 * 0x100000UL) {
        diag_set(DIAG_STORE, DIAG_FAIL, "memory too small");
        diag_failing("board_memory");
        return;
    }
    kbuf_init(&b, detail, sizeof detail);
    kbuf_put(&b, "RAM only ");
    kbuf_hex(&b, length);
    kbuf_put(&b, " @ ");
    kbuf_hex(&b, (u64)mem);
    diag_set(DIAG_STORE, DIAG_OK, detail);
    diag_note("[diag] persistent storage: ABSENT (no disk driver; store is RAM)\n");
    diag_stage("store_ram");
}

void kernel_main(u64 dtb) {
    u64 el;
    char detail[40];
    struct kbuf b;

    diag_init();
    diag_failing("debug_uart_setup");

    uart_setup(&debug_uart, 5, 0);              /* 9216000 / (16 x 115200) = 5 */
    debug_uart.up = 1;
    diag_set(DIAG_UART_DEBUG, DIAG_OK, "PL011 115200 debug connector");
    diag_stage("debug_uart");

    __asm__ volatile("mrs %0, CurrentEL" : "=r"(el));
    kbuf_init(&b, detail, sizeof detail);
    kbuf_put(&b, "BCM2712 EL");
    kbuf_dec(&b, el >> 2);
    diag_set(DIAG_PROCESSOR, DIAG_OK, detail);
    diag_set(DIAG_EXEC, DIAG_INITIALIZING, "commissioning");

    con_puts("\nkyrios v0.0.0 kernel on lirvol (BCM2712), EL");
    con_dec(el >> 2);
    con_puts("\ndebug uart: up\n");

    diag_failing("framebuffer_setup");
    con_puts("framebuffer: asking the firmware ... ");
    if (fb_setup()) {
        replay(2);
        con_dec(fb.width); con_puts(" x "); con_dec(fb.height);
        con_puts(", "); con_dec(fb.depth); con_puts(" bits, at ");
        con_hex((u64)fb.base);
        con_puts("\n");
        panel_mode = 1;
        diag_hdmi_ready(1);
        diag_refresh();
        diag_stage("hdmi_console");
    } else {
        con_puts("none.\n");
        diag_hdmi_ready(0);
        diag_note("[diag] HDMI: FAIL (no framebuffer); UART backends continue\n");
        diag_failing("framebuffer_setup");
    }
    if (firmware_revision) {
        con_puts("firmware revision: ");
        con_hex(firmware_revision);
        con_puts("\n");
    }

    header_uart_bringup();

    diag_failing("memory_map");
    diag_set(DIAG_MEMORY, DIAG_DETECTING, "KMEM");
    kbuf_init(&b, detail, sizeof detail);
    kbuf_hex(&b, KMEM_BASE);
    kbuf_put(&b, "+");
    kbuf_hex(&b, KMEM_LENGTH);
    if (dtb + 0x100000 > KMEM_BASE && dtb < KMEM_BASE + KMEM_LENGTH) {
        con_puts("WARNING: the device tree lies inside kyrios memory.\n");
        diag_set(DIAG_MEMORY, DIAG_FAIL, "DTB overlap");
        diag_failing("memory_dtb_overlap");
    } else if (fb.up && (u64)fb.base + (u64)fb.pitch * fb.height > KMEM_BASE &&
               (u64)fb.base < KMEM_BASE + KMEM_LENGTH) {
        con_puts("WARNING: the framebuffer lies inside kyrios memory.\n");
        diag_set(DIAG_MEMORY, DIAG_FAIL, "FB overlap");
        diag_failing("memory_fb_overlap");
    } else {
        diag_set(DIAG_MEMORY, DIAG_OK, detail);
        diag_stage("memory");
    }

    con_puts("device tree at ");
    con_hex(dtb);
    con_puts(", counter ");
    con_dec(board_counter_hz());
    con_puts(" per second\n");

    /* Fan and thermal before the application layer. */
    board_fan_start();

    board_probe_pcie_nvme_usb();
    board_eth_report();
    board_store_status();

    diag_set(DIAG_ONYCHOS, DIAG_DEFERRED, "not required for console");
    diag_set(DIAG_EXEC, DIAG_OK, "entering kyrios_main");
    diag_stage("commission_console_ready");
    diag_failing("(none)");

    kyrios_main();
    board_stop();
}
