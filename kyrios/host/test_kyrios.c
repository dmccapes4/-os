/* test_kyrios.c: the system code, run on the bench.
 *
 * Usage: test_kyrios [-v] [directory]
 * With a directory, kyrios memory is written there as memory.bin, with
 * memory.json saying where it started, for host/check_memory.py.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../system/kyrios.h"
#include "host.h"

static int checks, failures;

#define CHECK(cond) do { \
    checks++; \
    if (!(cond)) { failures++; printf("FAILED %s:%d  %s\n", __FILE__, __LINE__, #cond); } \
} while (0)

static struct kspace *hello = &kspaces[SP_HELLO];

/* Send one line and run the main loop until it has been taken. */
static const char *send(const char *text) {
    static char buf[8192];
    snprintf(buf, sizeof buf, "%s\r\n", text);
    host_clear();
    host_feed(buf);
    while (host_pending()) kyrios_turn();
    kyrios_turn();
    return host_output();
}

static void seconds(u64 n) {
    while (n--) {
        host_advance(HOST_HZ);
        kyrios_turn();
    }
}

static int has(const char *text, const char *part) { return strstr(text, part) != 0; }

/* The latest trace, as text. */
static const char *latest(void) {
    static char buf[K_TRACE + 1];
    u64 n;
    const char *l = record_line(record_count(), &n);
    memcpy(buf, l, n);
    buf[n] = 0;
    return buf;
}

/* The weight a document should have after `steps` rho steps with no use. */
static u64 untouched(u64 steps) {
    u64 w = 100, s;
    for (s = 1; s <= steps && w; s++) {
        u64 take = s <= 7 ? 0 : s <= 21 ? 2 : s <= 31 ? 4 : 8;
        if (take) w = w > take ? w - take : 0;
    }
    return w;
}

int main(int argc, char **argv) {
    const char *dir = 0, *o;
    struct kdoc *d, *quiet, *used;
    u64 before, n, i;
    u8 *memory, *copy;
    u64 memory_length;
    char text[2048];
    int a;

    for (a = 1; a < argc; a++) {
        if (!strcmp(argv[a], "-v")) host_show = 1;
        else dir = argv[a];
    }

    /* ---- power-on --------------------------------------------------- */
    CHECK(kyrios_init() == 1);
    memory = board_memory(&memory_length);
    CHECK(record_count() == 2);                         /* two ports assigned */
    CHECK(has(latest(), "\"what\": \"assign\""));
    CHECK(has(latest(), "\"port\": \"1b7d\""));
    CHECK(kstate == KS_WAITING);
    CHECK(kspaces[SP_KYRIOS].base == memory);           /* the one place to start */
    CHECK(memory[0] == '{');

    /* ---- nothing is taken before onychos starts kyrios -------------- */
    o = send("7037 create early too soon");
    CHECK(has(o, "\"result\": \"refused\"") && has(o, "\"reason\": \"not started\""));
    CHECK(record_count() == 4);                         /* refused, and traced */
    CHECK(hello->count == 0);

    o = send("7037 boot");
    CHECK(has(o, "\"reason\": \"not started\""));
    o = send("7038 boot");
    CHECK(has(o, "\"what\": \"boot\"") && has(o, "\"result\": \"done\"") && has(o, "\"who\": \"onychos\""));
    CHECK(kstate == KS_RUNNING);
    o = send("7038 boot");
    CHECK(has(o, "\"reason\": \"already started\""));

    /* ---- create, read, revise, list --------------------------------- */
    before = record_count();
    o = send("7037 create greeting {\"hello\": \"kyrios\"}");
    CHECK(record_count() == before + 2);                /* interaction, action */
    CHECK(has(o, "\"kind\": \"interaction\"") && has(o, "\"kind\": \"action\""));
    CHECK(has(o, "\"label\": \"0001\"") && has(o, "\"name\": \"greeting\""));
    CHECK(has(o, "\"length\": \"13\""));                /* 19 bytes */
    CHECK(has(o, "\"hash\": \"e46e0d5fb676198dc7a9c23462feb5356d958c6554535e6611828349624cc35f\""));
    CHECK(has(o, "\"tags\": [\"file\"]"));
    d = doc_by_label(hello, 1);
    CHECK(d && d->length == 19 && !memcmp(d->address, "{\"hello\": \"kyrios\"}", 19));
    CHECK(d->weight == 100 && d->made == before + 2 && d->last == before + 2);
    CHECK(d->address >= hello->base + K_TABLE && d->address + d->length <= hello->base + hello->length);

    o = send("7037 read 0001");
    CHECK(has(o, "\"what\": \"read\"") && has(o, "\"result\": \"done\""));
    CHECK(has(o, "}\n{\"hello\": \"kyrios\"}\n"));       /* the bytes follow the action's trace */
    CHECK(d->uses[KU_READ] == 1);

    o = send("7037 read 1");                            /* a label is a number */
    CHECK(has(o, "\"result\": \"done\""));

    o = send("7037 revise 0001 second text");
    CHECK(has(o, "\"what\": \"revise\"") && has(o, "\"result\": \"done\""));
    CHECK(has(o, "\"was\": \"e46e0d5fb676198dc7a9c23462feb5356d958c6554535e6611828349624cc35f\""));
    CHECK(d->length == 11 && !memcmp(d->address, "second text", 11));
    CHECK(d->uses[KU_REVISE] == 1 && d->label == 1);

    o = send("7037 create spaced   two leading spaces kept");
    d = doc_by_label(hello, 2);
    CHECK(d && d->length == 25 && !memcmp(d->address, "  two leading spaces kept", 25));

    o = send("7037 list");
    CHECK(has(o, "\"what\": \"list\"") && has(o, "kyrios-addresses-v0.0.1") && has(o, "\"0002\": {"));

    /* ---- refusals: each one is traced ------------------------------- */
    before = record_count();
    o = send("hello");
    CHECK(has(o, "\"reason\": \"unparsed\"") && record_count() == before + 2);
    o = send("9999 list");
    CHECK(has(o, "\"reason\": \"port not assigned\""));
    o = send("7037 frobnicate");
    CHECK(has(o, "\"reason\": \"unknown request\""));
    o = send("7037");
    CHECK(has(o, "\"reason\": \"unknown request\""));
    o = send("7037 create bad*name text");
    CHECK(has(o, "\"reason\": \"name\""));
    o = send("7037 create greeting again");
    CHECK(has(o, "\"reason\": \"name exists\""));
    o = send("7037 create empty");
    CHECK(has(o, "\"reason\": \"no content\""));
    o = send("7037 read 0009");
    CHECK(has(o, "\"reason\": \"no such label\""));
    o = send("7037 read zz");
    CHECK(has(o, "\"reason\": \"no such label\""));
    o = send("7037 revise 0001");
    CHECK(has(o, "\"reason\": \"no content\""));
    o = send("7037 remove 0001");
    CHECK(has(o, "\"reason\": \"not in v0.0.0\""));
    o = send("7037 halt");
    CHECK(has(o, "\"reason\": \"onychos only\"") && kstate == KS_RUNNING);
    o = send("7038 create x y");
    CHECK(has(o, "\"reason\": \"not in v0.0.0\""));
    o = send("7038 frobnicate");
    CHECK(has(o, "\"reason\": \"unknown request\""));
    CHECK(record_count() == before + 28);               /* 14 refusals, two traces each */
    CHECK(hello->count == 2);
    {
        static char longline[K_LINE + 400];
        memset(longline, 'x', sizeof longline - 1);
        memcpy(longline, "7037 create long ", 17);
        before = record_count();
        o = send(longline);
        CHECK(has(latest(), "\"reason\": \"line too long\"") && record_count() == before + 2);
        CHECK(hello->count == 2);
    }

    /* ---- onychos reads and nothing changes --------------------------- */
    copy = malloc(memory_length);
    memcpy(copy, memory, memory_length);
    before = record_count();
    d = doc_by_label(hello, 1);
    n = d->uses[KU_READ];

    o = send("7038 record");
    CHECK(has(o, "kyrios-record-v0.0.1") && has(o, "\"at\": \"1\"") && has(o, "\"reason\": \"line too long\""));
    o = send("7038 record 5");
    CHECK(!has(o, "\"at\": \"4\"") && has(o, "\"at\": \"5\""));
    o = send("7038 table hello-kyrios");
    CHECK(has(o, "\"name\": \"greeting\""));
    o = send("7038 table kyrios");
    CHECK(has(o, "\"name\": \"record.jsonl\"") && has(o, "\"name\": \"ports.json\""));
    o = send("7038 read hello-kyrios 0001");
    CHECK(has(o, "second text\n"));
    o = send("7038 read kyrios 0004");
    CHECK(has(o, "kyrios-ports-v0.0.1") && has(o, "\"1b7e\": {\"space\": \"onychos\""));
    o = send("7038 read kyrios 0005");
    CHECK(has(o, "\"rule\": \"schedule\"") && has(o, "\"takes\": [{\"for\": \"7\"}, {\"for\": \"e\", \"take\": \"2\"}"));
    o = send("7038 clew hello-kyrios 0001");
    CHECK(has(o, "\"what\": \"create\"") && has(o, "\"what\": \"revise\"") && !has(o, "\"name\": \"spaced\""));
    for (i = 0, n = 0; o[i]; i++) n += o[i] == '{';
    CHECK(n == 9);      /* create 1, two reads 4, revise 2, the refused revise 2 */
    o = send("7038 eth");
    CHECK(has(o, "ethernet:"));
    o = send("7038 read nowhere 0001");
    CHECK(has(o, "? no such space"));
    o = send("7038 read hello-kyrios 0040");
    CHECK(has(o, "? no such label"));

    CHECK(record_count() == before);
    CHECK(!memcmp(copy, memory, memory_length));        /* not one byte of kyrios memory differs */
    CHECK(doc_by_label(hello, 1)->uses[KU_READ] == 2);
    free(copy);

    /* ---- decay -------------------------------------------------------- */
    CHECK(ksched_documents.nrows == 32);
    CHECK(ksched_documents.step == 0);
    seconds(9);
    CHECK(ksched_documents.step == 0);
    seconds(1);
    CHECK(ksched_documents.step == 1);                  /* one step every 10 seconds */
    CHECK(has(latest(), "\"what\": \"rho\"") && has(latest(), "\"step\": \"1\""));
    CHECK(!has(latest(), "flagged"));
    d = doc_by_label(hello, 1);
    CHECK(d->weight == 100 && d->rho == 1);             /* used: 100 + 8 + 10, not past 100 */
    CHECK(d->uses[KU_READ] == 0 && d->uses[KU_REVISE] == 0);

    send("7037 create quiet never used again");
    quiet = doc_by_label(hello, 3);
    send("7037 create used read now and then");
    used = doc_by_label(hello, 4);
    CHECK(quiet && used && quiet->weight == 100);

    for (i = 1; i <= 34; i++) {
        seconds(10);
        CHECK(quiet->weight == untouched(i));
        if (i == 20) {
            CHECK(used->weight == untouched(20));       /* 100 - 13 x 2 = 74 */
            send("7037 read 0004");
        }
        if (i > 20) {
            /* The read added 4 and sent the document back to the start of
             * the table: seven steps with nothing taken, then 2 a step. */
            u64 since = i - 20;
            CHECK(used->weight == 78 - (since > 7 ? (since - 7) * 2 : 0));
        }
    }
    CHECK(quiet->weight == 8);
    snprintf(text, sizeof text, "%s", latest());
    seconds(10);                                        /* step 35 for the quiet document */
    CHECK(quiet->weight == 0);                          /* absent */
    o = latest();
    CHECK(has(o, "\"what\": \"rho\"") && has(o, "\"flagged\": [\""));
    {
        char id[33];
        struct kbuf b;
        kbuf_init(&b, id, 32);
        kbuf_hexbytes(&b, quiet->id, 16);
        id[32] = 0;
        CHECK(has(o, id));
        CHECK(!has(text, id));                          /* not flagged one step earlier */
        o = send("7038 table hello-kyrios");
        snprintf(text, sizeof text, "\"name\": \"quiet\"");
        CHECK(has(o, text));
        /* A flagged document has a schedule and no weight. */
        o = strstr(o, "\"name\": \"quiet\"");
        CHECK(o && has(o, "\"schedule\": \"documents\", \"rho\": "));
    }
    seconds(10);
    CHECK(quiet->weight == 0 && has(latest(), "flagged"));  /* still flagged; nothing else happens */

    /* A flagged document that is used has weight again. */
    o = send("7037 read 0003");
    CHECK(has(o, "never used again"));
    seconds(10);
    CHECK(quiet->weight == 4);
    {
        char id[33];
        struct kbuf b;
        kbuf_init(&b, id, 32);
        kbuf_hexbytes(&b, quiet->id, 16);
        id[32] = 0;
        CHECK(!has(latest(), id));                      /* no longer in the flagged list */
        CHECK(has(latest(), "flagged"));                /* the document `spaced` still is */
    }

    /* ---- halt ---------------------------------------------------------- */
    o = send("7038 halt");
    CHECK(has(o, "\"what\": \"halt\"") && has(o, "\"result\": \"done\"") && kstate == KS_HALTED);
    before = record_count();
    n = ksched_documents.step;
    seconds(100);
    CHECK(ksched_documents.step == n && record_count() == before);     /* rho stops with the system */
    o = send("7037 create late while halted");
    CHECK(has(o, "\"reason\": \"halted\""));
    o = send("7038 read hello-kyrios 0002");            /* onychos still reads */
    CHECK(has(o, "  two leading spaces kept"));
    o = send("7038 halt");
    CHECK(has(o, "\"reason\": \"not running\""));
    o = send("7038 resume");
    CHECK(has(o, "\"what\": \"resume\"") && has(o, "\"result\": \"done\"") && kstate == KS_RUNNING);
    o = send("7038 resume");
    CHECK(has(o, "\"reason\": \"not halted\""));
    seconds(10);
    CHECK(ksched_documents.step == n + 1);

    /* ---- limits --------------------------------------------------------- */
    for (i = hello->count + 1; i <= K_DOCS; i++) {
        snprintf(text, sizeof text, "7037 create doc-%lu content of document %lu", i, i);
        o = send(text);
        CHECK(has(o, "\"result\": \"done\""));
    }
    CHECK(hello->count == K_DOCS);
    o = send("7037 create one-more x");
    CHECK(has(o, "\"reason\": \"table full\""));
    CHECK(hello->table_length < K_TABLE - 1);
    o = send("7037 read 0040");
    CHECK(has(o, "content of document 64"));
    CHECK(kspaces[SP_ONYCHOS].count == 0 && kspaces[SP_KYRIOS].count == 6);

    /* ---- what was left in memory ---------------------------------------- */
    if (dir) {
        FILE *f;
        snprintf(text, sizeof text, "%s/memory.bin", dir);
        f = fopen(text, "wb");
        if (!f || fwrite(memory, 1, memory_length, f) != memory_length) { perror(text); return 2; }
        fclose(f);
        snprintf(text, sizeof text, "%s/memory.json", dir);
        f = fopen(text, "w");
        if (!f) { perror(text); return 2; }
        fprintf(f, "{\"base\": \"%lx\", \"length\": \"%lx\"}\n", (u64)memory, memory_length);
        fclose(f);
    }

    printf("%d checks, %d failed, %lu traces in the record\n", checks, failures, record_count());
    return failures ? 1 : 0;
}
