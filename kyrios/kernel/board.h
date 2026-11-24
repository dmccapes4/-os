/* board.h: what the kernel layer provides to kyrios on any board.
 * kernel/bcm2712.c provides it on lirvol; host/host_board.c provides it
 * on the bench so the same system code can be tested there. */
#ifndef KYRIOS_BOARD_H
#define KYRIOS_BOARD_H

#include "lib.h"

/* Console: one stream of text, written to every surface that is up. */
void con_putc(char c);
void con_write(const char *s, u64 n);
void con_puts(const char *s);
void con_hex(u64 v);
void con_dec(u64 v);

/* Input: the next byte from any serial surface, or -1 when there is none. */
int con_getc(void);

/* The hardware counter and how many counts it makes in one second. */
u64 board_counter(void);
u64 board_counter_hz(void);

/* The memory kyrios lays its spaces out in. */
u8 *board_memory(u64 *length);

/* Drive one board pin to a value (0 or 1) as a GPIO output. On lirvol
 * the pin is an RP1 GPIO number. */
void board_pin_write(int pin, int value);

/* Bring the Ethernet up (PHY out of reset, queues on) and print its
 * status. One test frame is sent on the first call. */
void board_eth_report(void);

/* Print and return to the controller any Ethernet frames that have
 * arrived. Called once per turn of the main loop. */
void board_eth_poll(void);

/* Called once per turn of the main loop when there was nothing to do. */
void board_idle(void);

/* Stop the processor. Does not return. */
void board_stop(void);

#endif
