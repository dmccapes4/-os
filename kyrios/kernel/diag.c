/* diag.c: commissioning status table and HDMI panel formatter. */
#include "diag.h"

#define DETAIL_CAP 40
#define STAGE_CAP  48

struct diag_row {
    enum diag_state st;
    char detail[DETAIL_CAP];
};

static struct diag_row rows[DIAG_COUNT];
static char last_stage[STAGE_CAP];
static char failing_op[STAGE_CAP];
static int  hdmi_live;
static int  inited;

static const char *labels[DIAG_COUNT] = {
    "Processor",
    "Execution state",
    "Memory",
    "RP1",
    "Fan PWM",
    "Fan RPM",
    "SoC temperature",
    "Thermal guard",
    "Debug UART",
    "GPIO UART",
    "PCIe",
    "NVMe",
    "USB",
    "Ethernet",
    "Document store",
    "onychos",
};

const char *diag_state_word(enum diag_state st) {
    switch (st) {
    case DIAG_UNKNOWN:       return "UNKNOWN";
    case DIAG_PENDING:       return "PENDING";
    case DIAG_DETECTING:     return "DETECTING";
    case DIAG_INITIALIZING:  return "INITIALIZING";
    case DIAG_OK:            return "OK";
    case DIAG_FAIL:          return "FAIL";
    case DIAG_DEFERRED:      return "DEFERRED";
    case DIAG_ABSENT:        return "ABSENT";
    default:                 return "?";
    }
}

static void copy_cap(char *dst, u64 cap, const char *src) {
    u64 i = 0;
    if (!src) { dst[0] = 0; return; }
    while (src[i] && i + 1 < cap) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static void emit_all(const char *s) {
    diag_sink_write(DIAG_SINK_UART_DEBUG | DIAG_SINK_UART_GPIO | DIAG_SINK_HDMI, s, kstrlen(s));
}

static void emit_uarts(const char *s) {
    diag_sink_write(DIAG_SINK_UART_DEBUG | DIAG_SINK_UART_GPIO, s, kstrlen(s));
}

static void emit_hdmi(const char *s) {
    diag_sink_write(DIAG_SINK_HDMI, s, kstrlen(s));
}

void diag_hdmi_ready(int yes) {
    hdmi_live = yes ? 1 : 0;
}

void diag_init(void) {
    int i;
    for (i = 0; i < DIAG_COUNT; i++) {
        rows[i].st = DIAG_PENDING;
        rows[i].detail[0] = 0;
    }
    rows[DIAG_PROCESSOR].st = DIAG_DETECTING;
    rows[DIAG_EXEC].st = DIAG_INITIALIZING;
    rows[DIAG_ONYCHOS].st = DIAG_DEFERRED;
    copy_cap(rows[DIAG_ONYCHOS].detail, DETAIL_CAP, "deferred");
    last_stage[0] = 0;
    failing_op[0] = 0;
    copy_cap(last_stage, STAGE_CAP, "diag_init");
    inited = 1;
}

void diag_set(enum diag_id id, enum diag_state st, const char *detail) {
    char line[96];
    struct kbuf b;
    if ((unsigned)id >= DIAG_COUNT) return;
    if (!inited) diag_init();
    rows[id].st = st;
    copy_cap(rows[id].detail, DETAIL_CAP, detail);

    kbuf_init(&b, line, sizeof line);
    kbuf_put(&b, "[diag] ");
    kbuf_put(&b, labels[id]);
    kbuf_put(&b, ": ");
    kbuf_put(&b, diag_state_word(st));
    if (detail && detail[0]) {
        kbuf_put(&b, " (");
        kbuf_put(&b, detail);
        kbuf_putc(&b, ')');
    }
    kbuf_putc(&b, '\n');
    emit_uarts(line);
    diag_refresh();
}

void diag_stage(const char *completed) {
    char line[96];
    struct kbuf b;
    if (!inited) diag_init();
    copy_cap(last_stage, STAGE_CAP, completed);
    failing_op[0] = 0;
    kbuf_init(&b, line, sizeof line);
    kbuf_put(&b, "[stage] ");
    kbuf_put(&b, last_stage);
    kbuf_putc(&b, '\n');
    emit_uarts(line);
    diag_refresh();
}

void diag_failing(const char *op) {
    char line[96];
    struct kbuf b;
    if (!inited) diag_init();
    copy_cap(failing_op, STAGE_CAP, op);
    kbuf_init(&b, line, sizeof line);
    kbuf_put(&b, "[failing] ");
    kbuf_put(&b, failing_op);
    kbuf_putc(&b, '\n');
    emit_uarts(line);
    diag_refresh();
}

void diag_note(const char *line) {
    if (!line) return;
    emit_all(line);
    if (line[0] && line[kstrlen(line) - 1] != '\n')
        emit_all("\n");
}

static void hdmi_line(const char *s) {
    emit_hdmi(s);
    emit_hdmi("\n");
}

void diag_refresh(void) {
    int i;
    char line[96];
    struct kbuf b;

    if (!hdmi_live) return;

    diag_hdmi_clear();

    hdmi_line("kyrios :: lirvol");
    hdmi_line("BOARD COMMISSIONING CONSOLE");
    hdmi_line("Boot image v0.0.0");
    hdmi_line("--------------------------------");

    for (i = 0; i < DIAG_COUNT; i++) {
        kbuf_init(&b, line, sizeof line);
        kbuf_put(&b, labels[i]);
        kbuf_put(&b, ": ");
        kbuf_put(&b, diag_state_word(rows[i].st));
        if (rows[i].detail[0]) {
            kbuf_put(&b, "  ");
            kbuf_put(&b, rows[i].detail);
        }
        hdmi_line(line);
    }

    hdmi_line("--------------------------------");
    kbuf_init(&b, line, sizeof line);
    kbuf_put(&b, "Last stage: ");
    kbuf_put(&b, last_stage[0] ? last_stage : "(none)");
    hdmi_line(line);

    kbuf_init(&b, line, sizeof line);
    kbuf_put(&b, "Failing op: ");
    kbuf_put(&b, failing_op[0] ? failing_op : "(none)");
    hdmi_line(line);

    hdmi_line("--------------------------------");
    hdmi_line("kyrios >");
}
