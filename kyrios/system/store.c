/* store.c: spaces, documents, address tables, the ports ledger.
 *
 * Layout of kyrios memory (chosen here; section 5.4 makes the layout the
 * operator's to accept, and that step is not built yet):
 *
 *   +0          space kyrios        8 MiB   the system's own documents
 *   +8 MiB      space onychos       1 MiB   port 7038
 *   +9 MiB      space hello-kyrios  16 MiB  port 7037
 *
 * Every space begins with its address table (space.addresses.json),
 * rendered as JSON text and followed by zero bytes. The table of the
 * space kyrios is therefore at the first address of kyrios memory. That
 * is the one place to start (section 3.5): it lists the three space
 * documents, ports.json, policy.jsonl and record.jsonl, each with its
 * address and length, and each space document gives the address of that
 * space's table.
 *
 * Current state is meant to survive power cycles by being stored to disk
 * (specification section 3.7). v0.0.1 has no disk driver, so memory
 * starts empty at every power-on.
 *
 * Two things here do not match what the operator has since stated
 * (specification section 9.5): the address tables are kept rendered as
 * JSON, where the stored form only needs addresses and weights; and the
 * space hello-kyrios is opened empty at power-on, where the first space
 * port should be the one opened at boot for the terminal program.
 */
#include "kyrios.h"

#define MIB 0x100000UL

struct kspace kspaces[K_SPACES];
struct kdoc *kdoc_ports, *kdoc_record;

/* Fixed places inside the space kyrios, after its table. */
#define SYS_SPACEDOC   0x10000UL    /* three space documents, 0x400 each */
#define SYS_PORTS      0x11000UL
#define SYS_POLICY     0x12000UL
#define SYS_RECORD     0x20000UL
#define SYS_SMALL      0x1000UL

/* ---- JSON text ------------------------------------------------------- */

void j_key(struct kbuf *b, int *first, const char *key) {
    if (!*first) kbuf_put(b, ", ");
    *first = 0;
    kbuf_putc(b, '"');
    kbuf_put(b, key);
    kbuf_put(b, "\": ");
}

void j_str(struct kbuf *b, int *first, const char *key, const char *s) {
    j_key(b, first, key);
    kbuf_putc(b, '"');
    kbuf_put(b, s);
    kbuf_putc(b, '"');
}

void j_hex(struct kbuf *b, int *first, const char *key, u64 v) {
    j_key(b, first, key);
    kbuf_putc(b, '"');
    kbuf_hex(b, v);
    kbuf_putc(b, '"');
}

void j_label(struct kbuf *b, int *first, const char *key, u64 label) {
    j_key(b, first, key);
    kbuf_putc(b, '"');
    kbuf_hexw(b, label, 4);
    kbuf_putc(b, '"');
}

void j_bytes(struct kbuf *b, int *first, const char *key, const u8 *p, u64 n) {
    j_key(b, first, key);
    kbuf_putc(b, '"');
    kbuf_hexbytes(b, p, n);
    kbuf_putc(b, '"');
}

/* ---- documents ------------------------------------------------------- */

/* Section 2.5:
 * doc_id = first 16 bytes of SHA-256("kyrios-doc-v0.0.1" 0 space 0 kind 0 name) */
static void doc_identity(struct kspace *sp, struct kdoc *d) {
    struct sha256 s;
    u8 out[32], zero = 0;
    sha256_begin(&s);
    sha256_add(&s, "kyrios-doc-v0.0.1", 17);
    sha256_add(&s, &zero, 1);
    sha256_add(&s, sp->name, kstrlen(sp->name));
    sha256_add(&s, &zero, 1);
    sha256_add(&s, d->kind, kstrlen(d->kind));
    sha256_add(&s, &zero, 1);
    sha256_add(&s, d->name, kstrlen(d->name));
    sha256_end(&s, out);
    memcpy(d->id, out, 16);
}

struct kdoc *doc_new(struct kspace *sp, const char *kind, const char *name, u64 name_length) {
    struct kdoc *d;
    if (sp->count >= K_DOCS || name_length < 1 || name_length > K_NAME) return 0;
    d = &sp->docs[sp->count];
    memset(d, 0, sizeof *d);
    sp->count++;
    d->used = 1;
    d->label = sp->count;
    d->kind = kind;
    memcpy(d->name, name, name_length);
    d->name[name_length] = 0;
    doc_identity(sp, d);
    return d;
}

struct kdoc *doc_by_label(struct kspace *sp, u64 label) {
    if (label < 1 || label > sp->count) return 0;
    return &sp->docs[label - 1];
}

struct kdoc *doc_by_name(struct kspace *sp, const char *kind, const char *name, u64 name_length) {
    u64 i;
    for (i = 0; i < sp->count; i++) {
        struct kdoc *d = &sp->docs[i];
        if (kstreq(d->kind, kind) && kstreqn(name, name_length, d->name)) return d;
    }
    return 0;
}

/* Room for n bytes in the space, or nothing when the space is full.
 * Bytes are never reclaimed in v0.0.1: a revision takes new room. */
u8 *space_room(struct kspace *sp, u64 n) {
    u8 *p = sp->next;
    if (n > (u64)(sp->base + sp->length - sp->next)) return 0;
    sp->next += n;
    return p;
}

struct kspace *space_by_port(u64 port) {
    int i;
    for (i = 0; i < K_SPACES; i++)
        if (kspaces[i].port && kspaces[i].since && kspaces[i].port == port) return &kspaces[i];
    return 0;
}

struct kspace *space_by_name(const char *name, u64 name_length) {
    int i;
    for (i = 0; i < K_SPACES; i++)
        if (kstreqn(name, name_length, kspaces[i].name)) return &kspaces[i];
    return 0;
}

/* ---- address table (section 3.2) ------------------------------------- */

void table_render(struct kspace *sp) {
    struct kbuf b;
    u64 i, was = sp->table_length;

    /* The record grows with every trace; its entry follows it. */
    if (sp == &kspaces[SP_KYRIOS] && kdoc_record) kdoc_record->length = record_length();

    kbuf_init(&b, (char *)sp->base, K_TABLE - 1);
    kbuf_put(&b, "{\"schema\": \"kyrios-addresses-v0.0.1\"");
    for (i = 0; i < sp->count; i++) {
        struct kdoc *d = &sp->docs[i];
        int first = 1;
        kbuf_put(&b, ",\n \"");
        kbuf_hexw(&b, d->label, 4);
        kbuf_put(&b, "\": {");
        j_bytes(&b, &first, "doc", d->id, 16);
        j_str(&b, &first, "kind", d->kind);
        j_str(&b, &first, "name", d->name);
        j_hex(&b, &first, "address", (u64)d->address);
        if (d->length) j_hex(&b, &first, "length", d->length);
        if (d->hashed) j_bytes(&b, &first, "hash", d->hash, 32);
        if (d->sched) {
            j_str(&b, &first, "schedule", d->sched->name);
            if (d->weight) j_hex(&b, &first, "weight", d->weight);
            if (d->rho) j_hex(&b, &first, "rho", d->rho);
        }
        if (d->made) j_hex(&b, &first, "made", d->made);
        if (d->last) j_hex(&b, &first, "last", d->last);
        kbuf_putc(&b, '}');
    }
    kbuf_put(&b, "}\n");
    for (i = b.len; i < was; i++) sp->base[i] = 0;
    sp->table_length = b.len;
}

/* ---- ports ledger (section 3.3) -------------------------------------- */

void ports_render(void) {
    struct kbuf b;
    int i;
    u64 was = kdoc_ports->length, k;

    kbuf_init(&b, (char *)kdoc_ports->address, SYS_SMALL - 1);
    kbuf_put(&b, "{\"schema\": \"kyrios-ports-v0.0.1\"");
    for (i = 0; i < K_SPACES; i++) {
        struct kspace *sp = &kspaces[i];
        int first = 1;
        if (!sp->port || !sp->since) continue;
        kbuf_put(&b, ",\n \"");
        kbuf_hex(&b, sp->port);
        kbuf_put(&b, "\": {");
        j_str(&b, &first, "space", sp->name);
        j_str(&b, &first, "state", "open");
        j_hex(&b, &first, "since", sp->since);
        kbuf_putc(&b, '}');
    }
    kbuf_put(&b, "}\n");
    for (k = b.len; k < was; k++) kdoc_ports->address[k] = 0;
    kdoc_ports->length = b.len;
    sha256_of(kdoc_ports->address, kdoc_ports->length, kdoc_ports->hash);
    kdoc_ports->hashed = 1;
}

/* ---- power-on -------------------------------------------------------- */

static void space_begin(struct kspace *sp, const char *name, u64 port, u8 *base, u64 length) {
    memset(sp, 0, sizeof *sp);
    sp->name = name;
    sp->port = port;
    sp->base = base;
    sp->length = length;
    sp->next = base + K_TABLE;
    memset(base, 0, K_TABLE);
}

/* A space document (section 5.1). */
static void space_document(struct kspace *sys, struct kspace *sp, u8 *at) {
    struct kdoc *d = doc_new(sys, "space", sp->name, kstrlen(sp->name));
    struct kbuf b;
    int first = 1;

    kbuf_init(&b, (char *)at, 0x400);
    kbuf_putc(&b, '{');
    if (sp->port) j_hex(&b, &first, "port", sp->port);
    j_key(&b, &first, "allotment");
    kbuf_putc(&b, '{');
    first = 1;
    j_hex(&b, &first, "address", (u64)sp->base);
    j_hex(&b, &first, "length", sp->length);
    kbuf_putc(&b, '}');
    j_hex(&b, &first, "table", (u64)sp->base);
    j_key(&b, &first, "limit");
    kbuf_putc(&b, '{');
    first = 1;
    j_hex(&b, &first, "document", K_LINE);
    j_hex(&b, &first, "documents", K_DOCS);
    kbuf_put(&b, "}}\n");

    d->address = at;
    d->length = b.len;
    sha256_of(at, b.len, d->hash);
    d->hashed = 1;
}

void store_init(u8 *memory, u64 length) {
    struct kspace *sys = &kspaces[SP_KYRIOS];
    struct kdoc *d;
    struct kbuf b;
    int i;

    (void)length;
    space_begin(&kspaces[SP_KYRIOS],  "kyrios",       0,            memory,            8 * MIB);
    space_begin(&kspaces[SP_ONYCHOS], "onychos",      PORT_ONYCHOS, memory + 8 * MIB,  1 * MIB);
    space_begin(&kspaces[SP_HELLO],   "hello-kyrios", PORT_HELLO,   memory + 9 * MIB,  16 * MIB);
    memset(sys->base + SYS_SPACEDOC, 0, SYS_RECORD - SYS_SPACEDOC);

    for (i = 0; i < K_SPACES; i++)
        space_document(sys, &kspaces[i], sys->base + SYS_SPACEDOC + (u64)i * 0x400);

    kdoc_ports = doc_new(sys, "port", "ports.json", 10);
    kdoc_ports->address = sys->base + SYS_PORTS;
    ports_render();

    d = doc_new(sys, "policy", "policy.jsonl", 12);
    d->address = sys->base + SYS_POLICY;
    kbuf_init(&b, (char *)d->address, SYS_SMALL - 1);
    decay_policy(&b);
    d->length = b.len;
    sha256_of(d->address, d->length, d->hash);
    d->hashed = 1;

    /* The record has no hash in the table: it changes with every trace.
     * Each trace carries the hash of the line before it instead. */
    kdoc_record = doc_new(sys, "record", "record.jsonl", 12);
    kdoc_record->address = sys->base + SYS_RECORD;
    record_init(kdoc_record->address, sys->length - SYS_RECORD);

    sys->next = sys->base + sys->length;        /* the space kyrios takes no other documents */
    for (i = 0; i < K_SPACES; i++) table_render(&kspaces[i]);
}
