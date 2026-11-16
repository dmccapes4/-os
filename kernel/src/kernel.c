// kernel.c — kyrios B1/B2, Pi 4 (BCM2711).
// B1: hello-kyrios on the HDMI framebuffer.
// B2: GIC + ARM generic timer at 10 Hz; the tick counter on screen is
// redrawn from inside the IRQ handler. After 100 ticks kernel_main
// deliberately executes brk: the synchronous handler reports ESR/ELR and
// halts with the evidence on screen, so the pass can be photographed at
// leisure — it does not need a human present at the moment it happens.
//
// No library, no Linux: firmware mailbox -> framebuffer -> our own pixels.
// A 5x7 glyph set drawn at scale is enough for a hello the operator can
// photograph; a real console font is B4's work, not smuggled in here.

typedef unsigned int u32;
typedef unsigned long u64;
typedef volatile u32 reg;

#define PERIPH_BASE 0xFE000000UL
#define MBOX_BASE   (PERIPH_BASE + 0xB880UL)
#define MBOX_READ   ((reg *)(MBOX_BASE + 0x00))
#define MBOX_STATUS ((reg *)(MBOX_BASE + 0x18))
#define MBOX_WRITE  ((reg *)(MBOX_BASE + 0x20))
#define MBOX_FULL   0x80000000u
#define MBOX_EMPTY  0x40000000u
#define MBOX_PROP   8u

static u32 __attribute__((aligned(16))) mbox[36];

static int mbox_call(void) {
    u32 msg = ((u32)(unsigned long)mbox & ~0xFu) | MBOX_PROP;
    while (*MBOX_STATUS & MBOX_FULL) { }
    *MBOX_WRITE = msg;
    for (;;) {
        while (*MBOX_STATUS & MBOX_EMPTY) { }
        u32 r = *MBOX_READ;
        if ((r & ~0xFu) == (msg & ~0xFu) && (r & 0xF) == MBOX_PROP)
            return mbox[1] == 0x80000000u;
    }
}

// Framebuffer request: 1024x768, 32bpp. Filled in by firmware on success.
static u32 fb_width, fb_height, fb_pitch;
static u32 *fb;

static int fb_init(void) {
    mbox[0] = 35 * 4; mbox[1] = 0;
    mbox[2] = 0x48003; mbox[3] = 8; mbox[4] = 0; mbox[5] = 1024; mbox[6] = 768;
    mbox[7] = 0x48004; mbox[8] = 8; mbox[9] = 0; mbox[10] = 1024; mbox[11] = 768;
    mbox[12] = 0x48009; mbox[13] = 8; mbox[14] = 0; mbox[15] = 0; mbox[16] = 0;
    mbox[17] = 0x48005; mbox[18] = 4; mbox[19] = 0; mbox[20] = 32;
    mbox[21] = 0x48006; mbox[22] = 4; mbox[23] = 0; mbox[24] = 1; // RGB
    mbox[25] = 0x40001; mbox[26] = 8; mbox[27] = 0; mbox[28] = 4096; mbox[29] = 0;
    mbox[30] = 0x40008; mbox[31] = 4; mbox[32] = 0; mbox[33] = 0;
    mbox[34] = 0;
    if (!mbox_call() || mbox[20] != 32 || mbox[28] == 0) return 0;
    fb_width = mbox[5]; fb_height = mbox[6]; fb_pitch = mbox[33];
    // GPU address -> ARM physical (clear the VC alias bits).
    fb = (u32 *)(unsigned long)(mbox[28] & 0x3FFFFFFFu);
    return 1;
}

// 5x7 glyphs, one byte per row, bits 4..0 left->right. Only what B1 says.
static const char *glyph_rows(char c, int row) {
    static const char *const font[][7] = {
        /*A*/ {"01110","10001","10001","11111","10001","10001","10001"},
        /*B*/ {"11110","10001","10001","11110","10001","10001","11110"},
        /*C*/ {"01110","10001","10000","10000","10000","10001","01110"},
        /*D*/ {"11100","10010","10001","10001","10001","10010","11100"},
        /*E*/ {"11111","10000","10000","11110","10000","10000","11111"},
        /*F*/ {"11111","10000","10000","11110","10000","10000","10000"},
        /*G*/ {"01110","10001","10000","10111","10001","10001","01111"},
        /*H*/ {"10001","10001","10001","11111","10001","10001","10001"},
        /*I*/ {"01110","00100","00100","00100","00100","00100","01110"},
        /*J*/ {"00111","00010","00010","00010","00010","10010","01100"},
        /*K*/ {"10001","10010","10100","11000","10100","10010","10001"},
        /*L*/ {"10000","10000","10000","10000","10000","10000","11111"},
        /*M*/ {"10001","11011","10101","10101","10001","10001","10001"},
        /*N*/ {"10001","10001","11001","10101","10011","10001","10001"},
        /*O*/ {"01110","10001","10001","10001","10001","10001","01110"},
        /*P*/ {"11110","10001","10001","11110","10000","10000","10000"},
        /*Q*/ {"01110","10001","10001","10001","10101","10010","01101"},
        /*R*/ {"11110","10001","10001","11110","10100","10010","10001"},
        /*S*/ {"01111","10000","10000","01110","00001","00001","11110"},
        /*T*/ {"11111","00100","00100","00100","00100","00100","00100"},
        /*U*/ {"10001","10001","10001","10001","10001","10001","01110"},
        /*V*/ {"10001","10001","10001","10001","10001","01010","00100"},
        /*W*/ {"10001","10001","10001","10101","10101","10101","01010"},
        /*X*/ {"10001","10001","01010","00100","01010","10001","10001"},
        /*Y*/ {"10001","10001","01010","00100","00100","00100","00100"},
        /*Z*/ {"11111","00001","00010","00100","01000","10000","11111"},
    };
    static const char *dash[7]  = {"00000","00000","00000","01110","00000","00000","00000"};
    static const char *space[7] = {"00000","00000","00000","00000","00000","00000","00000"};
    static const char *zero[7]  = {"01110","10001","10011","10101","11001","10001","01110"};
    static const char *one[7]   = {"00100","01100","00100","00100","00100","00100","01110"};
    static const char *digits[][7] = {
        /*2*/ {"01110","10001","00001","00110","01000","10000","11111"},
        /*3*/ {"11110","00001","00001","01110","00001","00001","11110"},
        /*4*/ {"00010","00110","01010","10010","11111","00010","00010"},
        /*5*/ {"11111","10000","11110","00001","00001","10001","01110"},
        /*6*/ {"00110","01000","10000","11110","10001","10001","01110"},
        /*7*/ {"11111","00001","00010","00100","01000","01000","01000"},
        /*8*/ {"01110","10001","10001","01110","10001","10001","01110"},
        /*9*/ {"01110","10001","10001","01111","00001","00010","01100"},
    };
    if (c >= 'A' && c <= 'Z') return font[c - 'A'][row];
    if (c == '-') return dash[row];
    if (c == '0') return zero[row];
    if (c == '1') return one[row];
    if (c >= '2' && c <= '9') return digits[c - '2'][row];
    return space[row];
}

static void draw_char(u32 x, u32 y, char c, u32 scale, u32 colour) {
    for (int row = 0; row < 7; row++) {
        const char *bits = glyph_rows(c, row);
        for (int col = 0; col < 5; col++) {
            if (bits[col] != '1') continue;
            for (u32 dy = 0; dy < scale; dy++)
                for (u32 dx = 0; dx < scale; dx++) {
                    u32 px = x + (u32)col * scale + dx;
                    u32 py = y + (u32)row * scale + dy;
                    if (px < fb_width && py < fb_height)
                        fb[(py * fb_pitch) / 4 + px] = colour;
                }
        }
    }
}

static void draw_text(u32 x, u32 y, const char *s, u32 scale, u32 colour) {
    for (; *s; s++, x += 6 * scale)
        draw_char(x, y, *s, scale, colour);
}

static void fill_rect(u32 x, u32 y, u32 w, u32 h, u32 colour) {
    for (u32 py = y; py < y + h && py < fb_height; py++)
        for (u32 px = x; px < x + w && px < fb_width; px++)
            fb[(py * fb_pitch) / 4 + px] = colour;
}

static void draw_dec(u32 x, u32 y, u32 v, u32 scale, u32 colour) {
    char buf[11]; int n = 0;
    if (v == 0) buf[n++] = '0';
    char rev[11]; int r = 0;
    while (v) { rev[r++] = (char)('0' + v % 10); v /= 10; }
    while (r) buf[n++] = rev[--r];
    buf[n] = 0;
    draw_text(x, y, buf, scale, colour);
}

static void draw_hex64(u32 x, u32 y, u64 v, u32 scale, u32 colour) {
    char buf[17];
    for (int i = 0; i < 16; i++) {
        int nib = (int)((v >> ((15 - i) * 4)) & 0xF);
        buf[i] = (char)(nib < 10 ? '0' + nib : 'A' + nib - 10);
    }
    buf[16] = 0;
    draw_text(x, y, buf, scale, colour);
}

// ---- B2: GIC-400 + ARM generic timer (PPI 30), 10 Hz ----------------
#define GICD 0xFF841000UL
#define GICC 0xFF842000UL
static reg *gicd(unsigned long off) { return (reg *)(GICD + off); }
static reg *gicc(unsigned long off) { return (reg *)(GICC + off); }

volatile u32 ticks;
static u64 timer_step; // counts per 0.1 s

static u64 read_cntpct(void) { u64 v; __asm__ volatile("mrs %0, cntpct_el0" : "=r"(v)); return v; }
static u64 read_cntfrq(void) { u64 v; __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(v)); return v; }

static void timer_arm_next(void) {
    u64 cval = read_cntpct() + timer_step;
    __asm__ volatile("msr cntp_cval_el0, %0" ::"r"(cval));
}

static void draw_ticks(void) {
    fill_rect(60, 400, 500, 40, 0x00081018u);
    draw_text(60, 400, "TICKS", 4, 0x00E0F0FFu);
    draw_dec(280, 400, ticks, 4, 0x00FFD080u);
}

void irq_handler(void) {
    u32 iar = *gicc(0x0C);           // GICC_IAR: acknowledge
    u32 id = iar & 0x3FF;
    if (id == 30) {                  // physical timer PPI
        ticks++;
        timer_arm_next();
        draw_ticks();
    }
    if (id != 1023) *gicc(0x10) = iar; // GICC_EOIR
}

void exception_report(u64 esr, u64 elr) {
    __asm__ volatile("msr daifset, #0xF"); // mask everything; this is final
    draw_text(60, 480, "EXCEPTION CAUGHT AND HELD", 4, 0x00FF8060u);
    draw_text(60, 540, "ESR", 3, 0x00E0F0FFu);
    draw_hex64(180, 540, esr, 3, 0x00FFD080u);
    draw_text(60, 590, "ELR", 3, 0x00E0F0FFu);
    draw_hex64(180, 590, elr, 3, 0x00FFD080u);
    draw_text(60, 640, " TICKS AT FAULT", 3, 0x00E0F0FFu);
    draw_dec(500, 640, ticks, 3, 0x00FFD080u);
    for (;;) { __asm__ volatile("wfe"); }
}

static void b2_init(void) {
    *gicd(0x000) = 0;                       // distributor off while we set up
    *gicd(0x100) = (1u << 30);              // enable PPI 30 (timer)
    *(reg *)(GICD + 0x400 + (30 / 4) * 4) = // priority: byte for IRQ 30
        (*(reg *)(GICD + 0x400 + (30 / 4) * 4) & ~(0xFFu << ((30 % 4) * 8)))
        | (0x80u << ((30 % 4) * 8));
    *gicd(0x000) = 1;                       // distributor on
    *gicc(0x04) = 0xFF;                     // priority mask: accept all
    *gicc(0x00) = 1;                        // CPU interface on

    timer_step = read_cntfrq() / 10;        // 10 Hz
    timer_arm_next();
    __asm__ volatile("msr cntp_ctl_el0, %0" ::"r"((u64)1)); // enable, unmask
    __asm__ volatile("msr daifclr, #2");    // unmask IRQ at EL1
}

void kernel_main(void) {
    if (fb_init()) {
        for (u32 i = 0; i < (fb_pitch / 4) * fb_height; i++)
            fb[i] = 0x00081018u; // deep slate, unmistakably not a login screen
        draw_text(60, 120, "HELLO-KYRIOS", 8, 0x00E0F0FFu);
        draw_text(60, 240, "B2 - TIME AND INTERRUPTS", 3, 0x0080C0A0u);
        draw_text(60, 300, "BOOTED ON VALSEIR - PI 4", 3, 0x0080C0A0u);
        draw_ticks();
    }
    b2_init();
    // Ten honest seconds of heartbeat, then we fault on purpose.
    while (ticks < 100) { __asm__ volatile("wfe"); }
    __asm__ volatile("brk #0x42");
    for (;;) { __asm__ volatile("wfe"); }
}
