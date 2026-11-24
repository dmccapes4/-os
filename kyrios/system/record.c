/* record.c: record.jsonl (section 4).
 *
 * The first line is the metadata object. Every line after it is a trace.
 * A trace is written once and not changed. Each trace is also written to
 * the console as it enters the record.
 *
 * Field order is fixed (chosen here): at, kind, operation, what, who,
 * port, space, doc, label, name, address, length, hash, was, result,
 * reason, tags, schedule, step, flagged, rho, ticks, prev.
 *
 * Not in v0.0.1: the boot count (nothing is stored to disk yet, so it
 * cannot be known) and eviction of the record.
 */
#include "kyrios.h"

static const char metadata[] =
    "{\"schema\": \"kyrios-record-v0.0.1\", \"fields\": {"
    "\"at\": \"position in the record, from 1; this line has none\", "
    "\"kind\": \"interaction or action\", "
    "\"operation\": \"the at of the interaction this trace belongs to\", "
    "\"what\": \"what happened\", "
    "\"who\": \"operator, onychos or system\", "
    "\"port\": \"the port it arrived at\", "
    "\"space\": \"the space concerned\", "
    "\"doc\": \"the document concerned\", "
    "\"label\": \"its label in the space's address table\", "
    "\"name\": \"its name, when it is made\", "
    "\"address\": \"where its bytes are after this trace\", "
    "\"length\": \"how many bytes\", "
    "\"hash\": \"SHA-256 of the bytes\", "
    "\"was\": \"the hash before a revision\", "
    "\"result\": \"done or refused, on an action\", "
    "\"reason\": \"why, on a refusal\", "
    "\"tags\": \"anything else that categorises the trace\", "
    "\"schedule\": \"the decay schedule, on a rho trace\", "
    "\"step\": \"the rho step, on a rho trace\", "
    "\"flagged\": \"documents whose weight is absent after the step\", "
    "\"rho\": \"the rho step of the document's schedule at this trace\", "
    "\"ticks\": \"hardware counter\", "
    "\"prev\": \"SHA-256 of the previous trace's line without its newline; absent on the first trace\"}, "
    "\"numbers\": \"hexadecimal strings; an absent value is an omitted key\", "
    "\"update\": {\"what\": \"schema\", \"fields\": \"fields added, renamed or retired from this line on\"}}";

static u8  *rec;                            /* the record's bytes */
static u64  rec_capacity, rec_length;
static u64  count;                          /* traces so far */
static u32  line_at[K_RECORD_LINES + 2];    /* where line k starts; line 0 is the metadata */
static u8   line_space[K_RECORD_LINES + 2]; /* the space of the document a trace names, from 1 */
static u8   line_label[K_RECORD_LINES + 2]; /* and its label */
static u8   prev[32];

void record_init(u8 *base, u64 capacity) {
    u64 n = sizeof metadata - 1;
    rec = base;
    rec_capacity = capacity;
    count = 0;
    memcpy(rec, metadata, n);
    rec[n] = '\n';
    rec_length = n + 1;
    line_at[0] = 0;
    line_at[1] = (u32)rec_length;
}

u64 record_count(void)  { return count; }
u64 record_next(void)   { return count + 1; }
u64 record_length(void) { return rec_length; }

/* Room for one more operation and for the halt trace that says the
 * record is full. */
int record_room(void) {
    return count + 8 <= K_RECORD_LINES && rec_length + 8 * K_TRACE <= rec_capacity;
}

const char *record_line(u64 at, u64 *length) {
    if (at > count) return 0;
    *length = (u64)line_at[at + 1] - line_at[at] - 1;
    return (const char *)rec + line_at[at];
}

static int space_number(struct kdoc *d) {
    int i;
    for (i = 0; i < K_SPACES; i++)
        if (d >= kspaces[i].docs && d < kspaces[i].docs + K_DOCS) return i + 1;
    return 0;
}

int record_line_names(u64 at, struct kspace *sp, struct kdoc *d) {
    if (at < 1 || at > count) return 0;
    return line_space[at] == (u8)(sp - kspaces + 1) && line_label[at] == (u8)d->label;
}

u64 record_append(struct ktrace *t) {
    static char line[K_TRACE];
    struct kbuf b;
    struct kdoc *d = t->doc;
    int first = 1, i;
    u64 at = count + 1, k;

    if (at > K_RECORD_LINES || rec_length + K_TRACE > rec_capacity) return 0;

    kbuf_init(&b, line, K_TRACE - 1);
    kbuf_putc(&b, '{');
    j_hex(&b, &first, "at", at);
    j_str(&b, &first, "kind", t->kind);
    if (t->operation) j_hex(&b, &first, "operation", t->operation);
    if (t->what) j_str(&b, &first, "what", t->what);
    if (t->who) j_str(&b, &first, "who", t->who);
    if (t->port) j_hex(&b, &first, "port", t->port);
    if (t->space) j_str(&b, &first, "space", t->space->name);
    if (d) {
        j_bytes(&b, &first, "doc", d->id, 16);
        j_label(&b, &first, "label", d->label);
        if (t->named) j_str(&b, &first, "name", d->name);
    }
    if (d && t->where) {
        j_hex(&b, &first, "address", (u64)d->address);
        j_hex(&b, &first, "length", d->length);
        if (d->hashed) j_bytes(&b, &first, "hash", d->hash, 32);
    } else {
        if (t->length) j_hex(&b, &first, "length", t->length);
        if (t->hash) j_bytes(&b, &first, "hash", t->hash, 32);
    }
    if (t->was) j_bytes(&b, &first, "was", t->was, 32);
    if (t->result) j_str(&b, &first, "result", t->result);
    if (t->reason) j_str(&b, &first, "reason", t->reason);
    if (t->tag) {
        j_key(&b, &first, "tags");
        kbuf_put(&b, "[\"");
        kbuf_put(&b, t->tag);
        kbuf_put(&b, "\"]");
    }
    if (t->sched) {
        int any = 0;
        j_str(&b, &first, "schedule", t->sched->name);
        j_hex(&b, &first, "step", t->sched->step);
        for (i = 0; i < K_SPACES; i++) {
            for (k = 0; k < kspaces[i].count; k++) {
                struct kdoc *f = &kspaces[i].docs[k];
                if (f->sched != t->sched || f->weight) continue;
                if (!any) { j_key(&b, &first, "flagged"); kbuf_putc(&b, '['); }
                else kbuf_put(&b, ", ");
                any = 1;
                kbuf_putc(&b, '"');
                kbuf_hexbytes(&b, f->id, 16);
                kbuf_putc(&b, '"');
            }
        }
        if (any) kbuf_putc(&b, ']');
    }
    if (d && d->sched && d->sched->step) j_hex(&b, &first, "rho", d->sched->step);
    j_hex(&b, &first, "ticks", board_counter());
    if (count) j_bytes(&b, &first, "prev", prev, 32);
    kbuf_putc(&b, '}');

    memcpy(rec + rec_length, line, b.len);
    rec[rec_length + b.len] = '\n';
    rec_length += b.len + 1;
    count = at;
    line_at[at + 1] = (u32)rec_length;
    sha256_of(line, b.len, prev);

    line_space[at] = 0;
    line_label[at] = 0;
    if (d) {
        line_space[at] = (u8)space_number(d);
        line_label[at] = (u8)d->label;
        if (!d->made) d->made = at;
        d->last = at;
    }

    /* The tables follow the record: the record's own entry, and the entry
     * of the document this trace names. */
    table_render(&kspaces[SP_KYRIOS]);
    if (d && space_number(d) > 1) table_render(&kspaces[space_number(d) - 1]);

    con_write(line, b.len);
    con_putc('\n');
    return at;
}
