/* decay.c: the decay clock (section 7).
 *
 * A schedule is two arrays. `add` is the weight added for each traced use
 * of a document, by what the use was. The take table gives the weight
 * taken at each rho step; a document walks it (section 2.3), and a use
 * sends the document back to the start of the table.
 *
 * Every number below is a placeholder. The numbers are those of
 * GAMEPLAN_ONYX_OX_20261117.md section 4.1 with one difference: a rho
 * step is 10 seconds here so that a session at the bench can see a
 * document go from made to flagged. Finding the right schedules is the
 * work, and it has not been done.
 *
 *   start 100, never raised past 100
 *   rows  1..7    nothing taken
 *   rows  8..21   take 2
 *   rows 22..31   take 4
 *   row  32       take 8, and the document stays on this row
 *   add: read 4, revise 10
 *
 * A document that is not used is flagged at step 35.
 *
 * rho stops here only when kyrios is halted. The rho clock is meant to be
 * turned off and on by itself (specification section 7.2); that is not
 * built. Spaces have no schedule: a space decays by its documents
 * decaying.
 */
#include "kyrios.h"

static const struct kseg documents_segs[] = {
    { 7, 0 }, { 14, 2 }, { 10, 4 }, { 0, 8 }
};

struct ksched ksched_documents = {
    .name  = "documents",
    .kind  = "file",
    .start = 100,
    .most  = 100,
    .add   = { [KU_READ] = 4, [KU_REVISE] = 10 },
    .every = 10,
    .segs  = documents_segs,
    .nsegs = 4,
};

void decay_init(void) {
    struct ksched *s = &ksched_documents;
    struct kwalk seg, n;
    u64 row = 0;

    /* Lay the segments out as rows. Two walks: one over the segments, one
     * over the steps of a segment. */
    kwalk_in(&seg, 1, s->nsegs);
    while (kwalk_next(&seg, 1) == K_DONE) {
        const struct kseg *g = &s->segs[seg.at - 1];
        kwalk_in(&n, 1, g->steps ? g->steps : 1);
        while (row < K_ROWS && kwalk_next(&n, 1) == K_DONE) s->rows[++row] = g->take;
    }
    s->nrows = row;
    s->step = 0;
    kwalk_in(&s->seconds, 1, s->every);
}

/* policy.jsonl: the metadata line, then the schedule. */
void decay_policy(struct kbuf *b) {
    struct ksched *s = &ksched_documents;
    int first = 1;
    u64 i;

    kbuf_put(b, "{\"schema\": \"kyrios-policy-v0.0.1\", \"fields\": {"
                "\"rule\": \"what kind of policy line this is\", "
                "\"name\": \"the schedule's name\", "
                "\"kinds\": \"the document kinds it applies to\", "
                "\"start\": \"weight given to a new document\", "
                "\"most\": \"the weight is never raised past this\", "
                "\"takes\": \"weight taken at each rho step: for that many steps, take that much; the last entry has no end\", "
                "\"add\": \"weight added for each traced use since the last step\", "
                "\"step\": \"what makes the schedule's clock step\", "
                "\"while_halted\": \"continue or stop\"}, "
                "\"numbers\": \"hexadecimal strings; an absent value is an omitted key\", "
                "\"update\": {\"rule\": \"schema\", \"fields\": \"fields added, renamed or retired from this line on\"}}\n");

    kbuf_putc(b, '{');
    j_str(b, &first, "rule", "schedule");
    j_str(b, &first, "name", s->name);
    j_key(b, &first, "kinds");
    kbuf_put(b, "[\"");
    kbuf_put(b, s->kind);
    kbuf_put(b, "\"]");
    j_str(b, &first, "method", "accrual");
    j_hex(b, &first, "start", s->start);
    j_hex(b, &first, "most", s->most);
    j_key(b, &first, "takes");
    kbuf_putc(b, '[');
    for (i = 0; i < s->nsegs; i++) {
        int f = 1;
        if (i) kbuf_put(b, ", ");
        kbuf_putc(b, '{');
        if (s->segs[i].steps) j_hex(b, &f, "for", s->segs[i].steps);
        if (s->segs[i].take) j_hex(b, &f, "take", s->segs[i].take);
        kbuf_putc(b, '}');
    }
    kbuf_putc(b, ']');
    j_key(b, &first, "add");
    kbuf_putc(b, '{');
    {
        int f = 1;
        j_hex(b, &f, "read", s->add[KU_READ]);
        j_hex(b, &f, "revise", s->add[KU_REVISE]);
    }
    kbuf_putc(b, '}');
    j_key(b, &first, "step");
    kbuf_putc(b, '{');
    {
        int f = 1;
        j_str(b, &f, "clock", "seconds");
        j_hex(b, &f, "every", s->every);
    }
    kbuf_putc(b, '}');
    j_str(b, &first, "while_halted", "stop");
    kbuf_put(b, "}\n");
}

void decay_new(struct kdoc *d) {
    struct ksched *s = &ksched_documents;
    if (!kstreq(d->kind, s->kind)) return;
    d->sched = s;
    d->weight = s->start;
    kwalk_in(&d->walk, 1, s->nrows);
}

/* A traced use. Counted here as it happens; the same counts can be had
 * from the record (section 7.3, step 1). */
void decay_use(struct kdoc *d, enum kuse use) {
    if (d->sched) d->uses[use]++;
}

/* One rho step (section 7.3). */
void decay_step(struct ksched *s) {
    struct ktrace t;
    int i;
    u64 k;

    s->step++;
    for (i = 0; i < K_SPACES; i++) {
        for (k = 0; k < kspaces[i].count; k++) {
            struct kdoc *d = &kspaces[i].docs[k];
            u64 add = 0, take;
            int u;
            if (d->sched != s) continue;

            /* Add. One row of the product: counts of uses times the add
             * array. A use sends the document back to the start. */
            for (u = 0; u < KU_COUNT; u++) {
                add += s->add[u] * d->uses[u];
                d->uses[u] = 0;
            }
            if (add) {
                d->weight += add;
                if (d->weight > s->most) d->weight = s->most;
                kwalk_restart(&d->walk);
            }

            /* Take. At the edge of the table the position does not move,
             * so the last row keeps applying. */
            if (d->weight) {
                kwalk_next(&d->walk, 1);
                take = s->rows[d->walk.at];
                if (take) {
                    if (d->weight > take) d->weight -= take;
                    else d->weight = 0;         /* absent: flagged */
                }
            }
            d->rho = s->step;
        }
    }

    memset(&t, 0, sizeof t);
    t.kind = "action";
    t.what = "rho";
    t.who = "system";
    t.sched = s;
    t.result = "done";
    record_append(&t);
    for (i = 1; i < K_SPACES; i++) table_render(&kspaces[i]);
}

/* Called once for each second that passes while kyrios is running. */
void decay_second(void) {
    struct ksched *s = &ksched_documents;
    if (kwalk_next(&s->seconds, 1) != K_DONE) return;
    if (s->seconds.at != s->seconds.last) return;
    kwalk_restart(&s->seconds);
    if (record_room()) decay_step(s);
}
