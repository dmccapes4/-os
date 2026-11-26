/* host_board.c: board.h on the bench, so that the system code can be
 * tested without the board. Output is captured, input is fed by the test,
 * and the counter moves only when the test moves it. */
#include <stdio.h>
#include <stdlib.h>
#include "../kernel/board.h"
#include "../kernel/diag.h"
#include "host.h"

#define OUT_CAP (64u << 20)

static char *out;
static u64   out_len;
static const char *in;
static u64   counter = 1000;
static u8   *memory;

void con_putc(char c) {
    if (!out) out = malloc(OUT_CAP);
    if (out_len + 1 < OUT_CAP) out[out_len++] = c;
    out[out_len] = 0;
    if (host_show) fputc(c, stdout);
}

void con_write(const char *s, u64 n) { while (n--) con_putc(*s++); }
void con_puts(const char *s)         { while (*s) con_putc(*s++); }

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
    if (!in || !*in) return -1;
    return (unsigned char)*in++;
}

u64 board_counter(void)    { return counter; }
u64 board_counter_hz(void) { return HOST_HZ; }

u8 *board_memory(u64 *length) {
    if (!memory) memory = calloc(1, HOST_MEMORY);
    *length = HOST_MEMORY;
    return memory;
}

void board_pin_write(int pin, int value) { (void)pin; (void)value; }

void board_eth_report(void) { con_puts("ethernet: the bench board has no Ethernet controller.\n"); }
void board_eth_poll(void) { }

void board_idle(void) { }
void board_hdmi_terminal(void) { }     /* no HDMI on the bench */
s64 board_temperature_mC(void) { return 42000; }   /* fixed bench reading */
void board_stop(void) { fprintf(stderr, "board_stop\n"); exit(2); }

/* Diagnostic sinks on the bench: one stream (captured console). HDMI
 * clear is a no-op; panel text still flows through the write sink. */
void diag_sink_putc(unsigned sinks, char c) {
    (void)sinks;
    con_putc(c);
}
void diag_sink_write(unsigned sinks, const char *s, u64 n) {
    (void)sinks;
    while (n--) con_putc(*s++);
}
void diag_hdmi_clear(void) { }

int host_show;

const char *host_output(void) { return out ? out : ""; }
void host_clear(void)         { out_len = 0; if (out) out[0] = 0; }
void host_feed(const char *s) { in = s; }
int  host_pending(void)       { return in && *in; }
void host_advance(u64 counts) { counter += counts; }
