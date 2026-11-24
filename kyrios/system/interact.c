/* interact.c: the interface and the order of checks (section 8).
 *
 * The interface here is a serial line (chosen here; the
 * specification's first interface is a port over Ethernet, and there is
 * no Ethernet driver yet). A request is one line of text:
 *
 *     <port> <request> ...
 *
 * The port is written in decimal. Labels and record positions are
 * written as the tables and the record show them, in hexadecimal.
 *
 * At a port assigned to an operator's space:
 *     create <name> <content>      content is the rest of the line, as sent
 *     read <label>
 *     revise <label> <content>
 *     list
 *
 * At the port of onychos:
 *     boot | halt | resume         traced
 *     record [from]                the record, from a position
 *     table <space>                a space's address table
 *     read <space> <label>         a document's bytes
 *     clew <space> <label>         every trace that names the document
 *     eth                          the Ethernet controller's status
 *   The last five write nothing: no trace, no weight, no change to any
 *   table (section 3.5). `eth` is a development addition: it is
 *   answered in every state, including before boot, so a board that
 *   shows nothing on a screen can still be asked about its Ethernet
 *   port from the serial keyboard.
 *
 * This interface is compiled into kyrios here. As stated since, it
 * belongs to a terminal program, the first application, in a space port
 * kyrios opens at boot (specification sections 8.7 and 9.5).
 *
 * Every other line that arrives is one operation: the interaction's
 * trace, then the action's trace with done or refused. There are no keys
 * in this build. `who` is taken from the port the line names, which checks
 * nothing.
 */
#include "kyrios.h"

enum kstate kstate;

static const char *const known[] = {
    "create", "read", "revise", "remove", "list", "link", "start", "stop",
    "point", "decide", "boot", "halt", "resume", 0
};

static const char *known_word(const char *w, u64 n) {
    int i;
    for (i = 0; known[i]; i++)
        if (kstreqn(w, n, known[i])) return known[i];
    return 0;
}

static int is_space(char c) { return c == ' ' || c == '\t'; }

/* The next word at *p. Returns 0 when there is none. */
static int next_word(const char **p, const char *end, const char **w, u64 *n) {
    const char *q = *p;
    while (q < end && is_space(*q)) q++;
    *w = q;
    while (q < end && !is_space(*q)) q++;
    *n = (u64)(q - *w);
    *p = q;
    return *n > 0;
}

/* Content: what follows the one separator after the last word. */
static const char *content_at(const char *p, const char *end, u64 *n) {
    if (p < end && is_space(*p)) p++;
    *n = (u64)(end - p);
    return p;
}

static int name_ok(const char *w, u64 n) {
    u64 i;
    if (n < 1 || n > K_NAME) return 0;
    for (i = 0; i < n; i++) {
        char c = w[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) continue;
        if (c == '.' || c == '_' || c == ':' || c == '/' || c == '#' || c == '+' || c == '>' || c == '-') continue;
        return 0;
    }
    return 1;
}

static void say(const char *s) {
    con_puts(s);
    con_putc('\n');
}

/* ---- onychos reads: nothing is written -------------------------------- */

static struct kdoc *named_document(const char **p, const char *end, struct kspace **spp) {
    const char *w;
    u64 n, label;
    struct kdoc *d;
    if (!next_word(p, end, &w, &n) || !(*spp = space_by_name(w, n))) {
        say("? no such space");
        return 0;
    }
    if (!next_word(p, end, &w, &n) || !kparse_hex(w, n, &label) || !(d = doc_by_label(*spp, label))) {
        say("? no such label");
        return 0;
    }
    return d;
}

static int onychos_read(const char *w, u64 n, const char *p, const char *end) {
    struct kspace *sp;
    struct kdoc *d;
    const char *a;
    u64 an, at, len;

    if (kstreqn(w, n, "record")) {
        at = 0;
        if (next_word(&p, end, &a, &an) && !kparse_hex(a, an, &at)) { say("? no such position"); return 1; }
        for (; at <= record_count(); at++) {
            const char *l = record_line(at, &len);
            con_write(l, len);
            con_putc('\n');
        }
        return 1;
    }
    if (kstreqn(w, n, "table")) {
        if (!next_word(&p, end, &a, &an) || !(sp = space_by_name(a, an))) { say("? no such space"); return 1; }
        con_write((const char *)sp->base, sp->table_length);
        return 1;
    }
    if (kstreqn(w, n, "read")) {
        if (!(d = named_document(&p, end, &sp))) return 1;
        if (d == kdoc_record) d->length = record_length();
        con_write((const char *)d->address, d->length);
        if (d->length && d->address[d->length - 1] != '\n') con_putc('\n');
        return 1;
    }
    if (kstreqn(w, n, "eth")) {
        board_eth_report();
        return 1;
    }
    if (kstreqn(w, n, "clew")) {
        if (!(d = named_document(&p, end, &sp))) return 1;
        for (at = d->made; at && at <= d->last; at++) {
            const char *l;
            if (!record_line_names(at, sp, d)) continue;
            l = record_line(at, &len);
            con_write(l, len);
            con_putc('\n');
        }
        return 1;
    }
    return 0;
}

/* ---- one operation ---------------------------------------------------- */

static void record_full(void) {
    static int said;
    struct ktrace t;
    if (!said) {
        said = 1;
        memset(&t, 0, sizeof t);
        t.kind = "action";
        t.what = "halt";
        t.who = "system";
        t.result = "done";
        t.tag = "record full";
        record_append(&t);
        kstate = KS_HALTED;
    }
    say("? the record is full. kyrios is halted and takes nothing more.");
}

static void operation(const char *line, u64 n, u64 arrived) {
    const char *p = line, *end = line + n, *w, *content;
    u64 wn, port = 0, label = 0, clen = 0;
    struct ktrace in, act;
    struct kspace *sp;
    struct kdoc *d = 0;
    const char *what;
    u8 line_hash[32], was[32], *room;
    int onychos;

    if (!next_word(&p, end, &w, &wn)) return;           /* nothing arrived */

    memset(&in, 0, sizeof in);
    memset(&act, 0, sizeof act);
    in.kind = "interaction";
    act.kind = "action";
    act.who = "system";
    act.result = "refused";

    /* 1. The port. */
    if (!kparse_dec(w, wn, &port) || port > 0xffff) {
        if (!record_room()) { record_full(); return; }
        in.operation = act.operation = record_next();
        sha256_of(line, n, line_hash);
        in.length = arrived;
        in.hash = line_hash;
        record_append(&in);
        act.reason = arrived > n ? "line too long" : "unparsed";
        record_append(&act);
        return;
    }
    sp = space_by_port(port);
    onychos = (sp == &kspaces[SP_ONYCHOS]);

    /* 2. The request word. A read by onychos is answered here and leaves
     *    nothing behind. */
    what = 0;
    if (next_word(&p, end, &w, &wn)) {
        if (onychos && arrived == n && onychos_read(w, wn, p, end)) return;
        what = known_word(w, wn);
    }

    if (!record_room()) { record_full(); return; }
    in.operation = act.operation = record_next();
    in.port = act.port = port;
    in.what = act.what = what;
    if (!what || arrived > n) {
        sha256_of(line, n, line_hash);
        in.length = arrived;
        in.hash = line_hash;
    }
    if (sp) {
        in.space = act.space = sp;
        in.who = onychos ? "onychos" : "operator";
    }

/* REFUSE writes both traces of a refused operation. REFUSE_ACTION writes
 * the action's trace when the interaction's is already in the record. */
#define REFUSE(why) do { record_append(&in); act.reason = (why); record_append(&act); return; } while (0)
#define REFUSE_ACTION(why) do { act.reason = (why); record_append(&act); return; } while (0)

    /* 3. Checks that do not depend on the request (section 8.2). */
    if (arrived > n) REFUSE("line too long");
    if (!sp) REFUSE("port not assigned");

    /* 4. The port of onychos: boot, halt, resume. */
    if (onychos) {
        if (!what) REFUSE("unknown request");
        if (kstreq(what, "boot")) {
            if (kstate != KS_WAITING) REFUSE("already started");
            kstate = KS_RUNNING;
        } else if (kstreq(what, "halt")) {
            if (kstate != KS_RUNNING) REFUSE("not running");
            kstate = KS_HALTED;
            act.tag = "onychos";
        } else if (kstreq(what, "resume")) {
            if (kstate != KS_HALTED) REFUSE("not halted");
            kstate = KS_RUNNING;
        } else {
            REFUSE("not in v0.0.0");
        }
        record_append(&in);
        act.result = "done";
        record_append(&act);
        return;
    }

    /* 5. An operator's port. Nothing is taken until onychos has started
     *    kyrios, and nothing while it is halted. */
    if (kstate == KS_WAITING) REFUSE("not started");
    if (kstate == KS_HALTED) REFUSE("halted");
    if (!what) REFUSE("unknown request");

    if (kstreq(what, "create")) {
        const char *name;
        u64 nn;
        int have = next_word(&p, end, &name, &nn);
        content = content_at(p, end, &clen);
        in.length = clen;
        record_append(&in);
        if (!have || !name_ok(name, nn)) REFUSE_ACTION("name");
        if (doc_by_name(sp, "file", name, nn)) REFUSE_ACTION("name exists");
        if (clen < 1) REFUSE_ACTION("no content");
        if (sp->count >= K_DOCS) REFUSE_ACTION("table full");
        if (!(room = space_room(sp, clen))) REFUSE_ACTION("space full");
        d = doc_new(sp, "file", name, nn);
        memcpy(room, content, clen);
        d->address = room;
        d->length = clen;
        sha256_of(room, clen, d->hash);
        d->hashed = 1;
        decay_new(d);
        act.doc = d;
        act.named = 1;
        act.where = 1;
        act.tag = "file";
        act.result = "done";
        record_append(&act);
        return;
    }

    if (kstreq(what, "read") || kstreq(what, "revise")) {
        int revise = kstreq(what, "revise");
        if (next_word(&p, end, &w, &wn) && kparse_hex(w, wn, &label)) d = doc_by_label(sp, label);
        content = content_at(p, end, &clen);
        in.doc = d;
        if (revise) in.length = clen;
        record_append(&in);
        if (!d) REFUSE_ACTION("no such label");
        act.doc = d;
        if (revise) {
            if (clen < 1) REFUSE_ACTION("no content");
            if (!(room = space_room(sp, clen))) REFUSE_ACTION("space full");
            memcpy(was, d->hash, 32);
            memcpy(room, content, clen);
            d->address = room;
            d->length = clen;
            sha256_of(room, clen, d->hash);
            decay_use(d, KU_REVISE);
            act.was = was;
        } else {
            decay_use(d, KU_READ);
        }
        act.where = 1;
        act.result = "done";
        record_append(&act);
        if (!revise) {
            con_write((const char *)d->address, d->length);
            con_putc('\n');
        }
        return;
    }

    record_append(&in);
    if (kstreq(what, "list")) {
        act.result = "done";
        record_append(&act);
        con_write((const char *)sp->base, sp->table_length);
        return;
    }
    if (kstreq(what, "boot") || kstreq(what, "halt") || kstreq(what, "resume")) REFUSE_ACTION("onychos only");
    REFUSE_ACTION("not in v0.0.0");
#undef REFUSE
#undef REFUSE_ACTION
}

void kyrios_line(const char *line, u64 n) {
    operation(line, n, n);
}

/* ---- the main loop ----------------------------------------------------- */

static char line[K_LINE];
static u64  line_n;             /* bytes kept */
static u64  line_arrived;       /* bytes that arrived, kept or not */
static int  after_cr;
static u64  next_second, second;

static void take_byte(int c) {
    if (c == '\n' && after_cr) { after_cr = 0; return; }
    after_cr = (c == '\r');

    if (c == '\r' || c == '\n') {
        con_putc('\n');
        if (line_arrived) operation(line, line_n, line_arrived);
        line_n = 0;
        line_arrived = 0;
        return;
    }
    if (c == 8 || c == 127) {
        if (line_n && line_n == line_arrived) {
            line_n--;
            line_arrived--;
            con_puts("\b \b");
        }
        return;
    }
    if (c < 0x20 && c != '\t') return;          /* not text */
    line_arrived++;
    if (line_n < K_LINE) line[line_n++] = (char)c;
    con_putc((char)c);                          /* echo */
}

void kyrios_turn(void) {
    int c = con_getc();
    u64 now = board_counter();

    if (c >= 0) take_byte(c);
    board_eth_poll();

    if (!second) {
        second = board_counter_hz();
        next_second = now + second;
    }
    if (now >= next_second) {
        next_second += second;
        if (kstate == KS_RUNNING) decay_second();
    }
    if (c < 0) board_idle();
}
