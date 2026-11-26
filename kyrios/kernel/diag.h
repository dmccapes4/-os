/* diag.h: board commissioning console — status model and shared output.
 *
 * Backends (HDMI, debug UART, header UART) are registered by the board
 * layer. A backend that is down or stalls must not stop the others.
 * This is a diagnostic surface, not a substitute for the drivers it
 * reports on.
 */
#ifndef KYRIOS_DIAG_H
#define KYRIOS_DIAG_H

#include "lib.h"

enum diag_id {
    DIAG_PROCESSOR = 0,
    DIAG_EXEC,
    DIAG_MEMORY,
    DIAG_RP1,
    DIAG_FAN_PWM,
    DIAG_FAN_RPM,
    DIAG_TEMP,
    DIAG_THERMAL,
    DIAG_UART_DEBUG,
    DIAG_UART_GPIO,
    DIAG_PCIE,
    DIAG_NVME,
    DIAG_USB,
    DIAG_ETHERNET,
    DIAG_STORE,
    DIAG_ONYCHOS,
    DIAG_COUNT
};

/* Short status words painted on the HDMI panel and logged on UART. */
enum diag_state {
    DIAG_UNKNOWN = 0,
    DIAG_PENDING,
    DIAG_DETECTING,
    DIAG_INITIALIZING,
    DIAG_OK,
    DIAG_FAIL,
    DIAG_DEFERRED,
    DIAG_ABSENT
};

/* Board-supplied sinks. Each write must tolerate a dead backend. */
#define DIAG_SINK_UART_DEBUG  1
#define DIAG_SINK_UART_GPIO   2
#define DIAG_SINK_HDMI        4

void diag_sink_write(unsigned sinks, const char *s, u64 n);
void diag_sink_putc(unsigned sinks, char c);
void diag_hdmi_clear(void);
void diag_hdmi_ready(int yes);          /* board says the panel can paint */

void diag_init(void);
void diag_set(enum diag_id id, enum diag_state st, const char *detail);
void diag_stage(const char *completed); /* last completed init stage */
void diag_failing(const char *op);      /* operation in progress / failing */
void diag_note(const char *line);       /* one log line to every live sink */
void diag_refresh(void);                /* repaint HDMI panel; UART unchanged */

const char *diag_state_word(enum diag_state st);

#endif
