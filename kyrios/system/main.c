/* main.c: power-on. */
#include "kyrios.h"
#include "../kernel/diag.h"

static int self_check(void) {
    static const u8 abc[32] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
    };
    u8 out[32];
    struct kwalk x;
    int ok = 1;

    sha256_of("abc", 3, out);
    if (memcmp(out, abc, 32)) { con_puts("self-check: SHA-256 is wrong\n"); ok = 0; }

    /* The example of section 2.3. */
    kwalk_in(&x, 1, 10);
    if (x.place != K_ABSENT) ok = 0;
    if (kwalk_back(&x, 1) != K_NOT_PLACED) ok = 0;
    if (kwalk_next(&x, 1) != K_DONE || x.at != 1) ok = 0;
    if (kwalk_now(&x, 5)  != K_DONE || x.at != 5) ok = 0;
    if (kwalk_next(&x, 2) != K_DONE || x.at != 7) ok = 0;
    if (kwalk_back(&x, 1) != K_DONE || x.at != 6) ok = 0;
    if (kwalk_back(&x, 2) != K_DONE || x.at != 4) ok = 0;
    if (kwalk_next(&x, 7) != K_EDGE || x.at != 4) ok = 0;      /* would leave the scope */
    if (kwalk_back(&x, 4) != K_EDGE || x.at != 4) ok = 0;
    if (kwalk_now(&x, 11) != K_EDGE || x.at != 4) ok = 0;
    kwalk_restart(&x);
    if (x.place != K_ABSENT) ok = 0;
    kwalk_in(&x, 3, 4);                                         /* declared again */
    if (kwalk_next(&x, 1) != K_DONE || x.at != 3) ok = 0;
    if (!ok) con_puts("self-check: failed\n");
    return ok;
}

static void show_space(struct kspace *sp) {
    con_puts("  space ");
    con_puts(sp->name);
    con_puts("  at ");
    con_hex((u64)sp->base);
    con_puts("  length ");
    con_hex(sp->length);
    if (sp->port) {
        con_puts("  port ");
        con_dec(sp->port);
    }
    con_putc('\n');
}

int kyrios_init(void) {
    u64 length;
    u8 *memory;
    int i;

    con_puts("\nkyrios v0.0.0\n");
    diag_failing("self_check");
    if (!self_check()) {
        diag_set(DIAG_EXEC, DIAG_FAIL, "self-check");
        return 0;
    }
    con_puts("self-check: SHA-256 and the walk are right\n");
    diag_stage("self_check");

    memory = board_memory(&length);
    if (length < 25 * 0x100000UL) {
        con_puts("not enough memory for the layout\n");
        diag_set(DIAG_STORE, DIAG_FAIL, "memory too small");
        diag_failing("store_init");
        return 0;
    }
    kstate = KS_WAITING;
    decay_init();
    store_init(memory, length);
    diag_set(DIAG_STORE, DIAG_OK, "spaces+record in RAM");
    diag_stage("store_init");

    /* The ports are assigned. Each assignment is traced. */
    for (i = 1; i < K_SPACES; i++) {
        struct ktrace t;
        memset(&t, 0, sizeof t);
        kspaces[i].since = record_next();
        ports_render();
        t.kind = "action";
        t.what = "assign";
        t.who = "system";
        t.port = kspaces[i].port;
        t.space = &kspaces[i];
        t.doc = kdoc_ports;
        t.where = 1;
        t.result = "done";
        record_append(&t);
    }

    con_puts("\nmemory at ");
    con_hex((u64)memory);
    con_puts("  length ");
    con_hex(length);
    con_puts("  (hexadecimal)\n");
    for (i = 0; i < K_SPACES; i++) show_space(&kspaces[i]);
    con_puts("  the first table is at ");
    con_hex((u64)kspaces[SP_KYRIOS].base);
    con_puts("; the record is at ");
    con_hex((u64)kdoc_record->address);
    con_puts("\n\nkyrios takes no interaction until onychos starts it.\n"
             "A request is one line: <port> <request>\n  ");
    con_dec(PORT_ONYCHOS);
    con_puts(" boot | halt | resume\n  ");
    con_dec(PORT_ONYCHOS);
    con_puts(" record [from] | table <space> | read <space> <label> | clew <space> <label> | eth\n  ");
    con_dec(PORT_HELLO);
    con_puts(" create <name> <content> | read <label> | revise <label> <content> | list\n\n");
    return 1;
}

void kyrios_main(void) {
    board_hdmi_terminal();      /* commissioning panel done; HDMI becomes the terminal */
    if (!kyrios_init()) {
        con_puts("kyrios did not start.\n");
        return;
    }
    for (;;) kyrios_turn();
}
