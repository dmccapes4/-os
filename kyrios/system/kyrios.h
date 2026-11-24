/* kyrios.h: kyrios v0.0.0 (development).
 *
 * Basis: docs/KYRIOS_TECHNICAL_SPECIFICATION_v0.0.1.md. Section numbers
 * in comments refer to it. Where this build had to choose something the
 * specification leaves open, the comment says "chosen here".
 *
 * Values. Counts start at 1. An absent value is an omitted key in the
 * tables and the record. Inside this code an absent count is held as 0,
 * which is never written out. There are no negatives and no division.
 * Numbers are hexadecimal in the tables and the record.
 */
#ifndef KYRIOS_H
#define KYRIOS_H

#include "../kernel/lib.h"
#include "../kernel/walk.h"
#include "../kernel/sha256.h"
#include "../kernel/board.h"

#define K_DOCS          64          /* documents per space */
#define K_NAME          96          /* longest name */
#define K_LINE          1024        /* longest request line */
#define K_TRACE         4096        /* longest trace line */
#define K_TABLE         0x10000UL   /* room for a space's address table */
#define K_RECORD_LINES  32768UL     /* traces the record index can hold */
#define K_ROWS          64          /* rows in a schedule's take table */

/* ---- decay (section 7) ----------------------------------------------- */

enum kuse { KU_READ = 0, KU_REVISE, KU_COUNT };      /* columns of the add array */

struct kseg { u64 steps; u64 take; };                /* steps absent: until the end */

struct ksched {
    const char *name;
    const char *kind;               /* the document kind it applies to */
    u64 start, most;
    u64 add[KU_COUNT];
    u64 every;                      /* seconds in one rho step */
    const struct kseg *segs;
    u64 nsegs;
    u64 rows[K_ROWS + 1];           /* take at each row, from row 1 */
    u64 nrows;
    u64 step;                       /* rho steps taken so far */
    struct kwalk seconds;           /* seconds inside the current step */
};

/* ---- documents and spaces (sections 3 and 5) ------------------------- */

struct kdoc {
    int  used;
    u64  label;                     /* from 1; never reused */
    const char *kind;
    char name[K_NAME + 1];
    u8   id[16];
    u8  *address;
    u64  length;
    int  hashed;
    u8   hash[32];
    struct ksched *sched;
    u64  weight;                    /* absent: flagged for eviction */
    u64  rho;                       /* the rho step at which weight was last set */
    u64  made, last;                /* record positions */
    struct kwalk walk;              /* where the document is in the take table */
    u64  uses[KU_COUNT];            /* traced uses since the last rho step */
};

struct kspace {
    const char *name;
    u64  port;
    u64  since;                     /* record position of the port's assignment */
    u8  *base;
    u64  length;
    u8  *next;                      /* where the next document's bytes go */
    u64  table_length;
    u64  count;                     /* labels given so far */
    struct kdoc docs[K_DOCS];
};

enum { SP_KYRIOS = 0, SP_ONYCHOS, SP_HELLO, K_SPACES };

extern struct kspace kspaces[K_SPACES];

void         store_init(u8 *memory, u64 length);
struct kdoc *doc_new(struct kspace *sp, const char *kind, const char *name, u64 name_length);
struct kdoc *doc_by_label(struct kspace *sp, u64 label);
struct kdoc *doc_by_name(struct kspace *sp, const char *kind, const char *name, u64 name_length);
u8          *space_room(struct kspace *sp, u64 n);
struct kspace *space_by_port(u64 port);
struct kspace *space_by_name(const char *name, u64 name_length);
void         table_render(struct kspace *sp);
void         ports_render(void);
extern struct kdoc *kdoc_ports, *kdoc_record;

/* ---- the record (section 4) ------------------------------------------ */

struct ktrace {
    const char *kind;               /* interaction or action */
    u64  operation;
    const char *what;
    const char *who;
    u64  port;
    struct kspace *space;
    struct kdoc *doc;
    int  named;                     /* include the document's name */
    int  where;                     /* include the document's address, length and hash */
    u64  length;                    /* a length when the document's is not included */
    const u8 *hash;                 /* a hash when the document's is not included */
    const u8 *was;
    const char *result;
    const char *reason;
    const char *tag;
    struct ksched *sched;           /* on a rho trace */
};

void record_init(u8 *base, u64 capacity);
u64  record_append(struct ktrace *t);       /* returns the trace's position */
u64  record_count(void);
u64  record_next(void);                     /* the position the next trace will get */
u64  record_length(void);
int  record_room(void);
const char *record_line(u64 at, u64 *length);
int  record_line_names(u64 at, struct kspace *sp, struct kdoc *d);
void record_metadata(u64 *length);

/* ---- decay ----------------------------------------------------------- */

extern struct ksched ksched_documents;

void decay_init(void);
void decay_policy(struct kbuf *b);
void decay_new(struct kdoc *d);
void decay_use(struct kdoc *d, enum kuse use);
void decay_second(void);
void decay_step(struct ksched *s);

/* ---- JSON text ------------------------------------------------------- */

void j_key(struct kbuf *b, int *first, const char *key);
void j_str(struct kbuf *b, int *first, const char *key, const char *s);
void j_hex(struct kbuf *b, int *first, const char *key, u64 v);
void j_label(struct kbuf *b, int *first, const char *key, u64 label);
void j_bytes(struct kbuf *b, int *first, const char *key, const u8 *p, u64 n);

/* ---- kyrios ---------------------------------------------------------- */

enum kstate { KS_WAITING = 1, KS_RUNNING, KS_HALTED };
extern enum kstate kstate;

#define PORT_ONYCHOS 7038UL         /* chosen here */
#define PORT_HELLO   7037UL         /* spaces/hello-kyrios.md */

int  kyrios_init(void);             /* 1 when kyrios is ready */
void kyrios_turn(void);             /* one turn of the main loop */
void kyrios_line(const char *line, u64 n);
void kyrios_main(void);

#endif
