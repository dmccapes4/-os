/* lib.c: see lib.h. */
#include "lib.h"

#ifndef KYRIOS_HOST
/* The compiler may emit calls to these even in a freestanding build.
 * Byte loops only: with the MMU off, data accesses must stay aligned. */
void *memcpy(void *dst, const void *src, u64 n) {
    u8 *d = dst;
    const u8 *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

void *memmove(void *dst, const void *src, u64 n) {
    u8 *d = dst;
    const u8 *s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

void *memset(void *dst, int c, u64 n) {
    u8 *d = dst;
    while (n--) *d++ = (u8)c;
    return dst;
}

int memcmp(const void *a, const void *b, u64 n) {
    const u8 *x = a, *y = b;
    for (; n; n--, x++, y++)
        if (*x != *y) return *x < *y ? -1 : 1;
    return 0;
}
#endif

u64 kstrlen(const char *s) {
    u64 n = 0;
    while (s[n]) n++;
    return n;
}

int kstreq(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}

int kstreqn(const char *a, u64 an, const char *b) {
    u64 i;
    for (i = 0; i < an; i++)
        if (!b[i] || a[i] != b[i]) return 0;
    return b[an] == 0;
}

void kbuf_init(struct kbuf *b, char *p, u64 cap) {
    b->p = p;
    b->len = 0;
    b->cap = cap;
    b->over = 0;
}

void kbuf_putc(struct kbuf *b, char c) {
    if (b->len < b->cap) b->p[b->len++] = c;
    else b->over = 1;
}

void kbuf_put(struct kbuf *b, const char *s) {
    while (*s) kbuf_putc(b, *s++);
}

void kbuf_putn(struct kbuf *b, const char *s, u64 n) {
    while (n--) kbuf_putc(b, *s++);
}

static const char hexdigit[] = "0123456789abcdef";

void kbuf_hex(struct kbuf *b, u64 v) {
    int shift = 60, started = 0;
    for (; shift >= 0; shift -= 4) {
        u64 nib = (v >> shift) & 0xf;
        if (nib || started || shift == 0) {
            kbuf_putc(b, hexdigit[nib]);
            started = 1;
        }
    }
}

void kbuf_hexw(struct kbuf *b, u64 v, int width) {
    int shift = (width - 1) * 4;
    for (; shift >= 0; shift -= 4)
        kbuf_putc(b, hexdigit[(v >> shift) & 0xf]);
}

void kbuf_hexbytes(struct kbuf *b, const u8 *p, u64 n) {
    while (n--) {
        kbuf_putc(b, hexdigit[*p >> 4]);
        kbuf_putc(b, hexdigit[*p & 0xf]);
        p++;
    }
}

/* Decimal by taking away: for each power of ten, count how many times it
 * can be taken from what is left. */
void kbuf_dec(struct kbuf *b, u64 v) {
    static const u64 ten[20] = {
        10000000000000000000UL, 1000000000000000000UL, 100000000000000000UL,
        10000000000000000UL, 1000000000000000UL, 100000000000000UL,
        10000000000000UL, 1000000000000UL, 100000000000UL, 10000000000UL,
        1000000000UL, 100000000UL, 10000000UL, 1000000UL, 100000UL, 10000UL,
        1000UL, 100UL, 10UL, 1UL
    };
    int i, started = 0;
    for (i = 0; i < 20; i++) {
        int takes = 0;
        while (v >= ten[i]) { v -= ten[i]; takes++; }
        if (takes || started || i == 19) {
            kbuf_putc(b, (char)('0' + takes));
            started = 1;
        }
    }
}

int kparse_dec(const char *s, u64 n, u64 *out) {
    u64 v = 0, i;
    if (n < 1 || n > 19) return 0;
    for (i = 0; i < n; i++) {
        if (s[i] < '0' || s[i] > '9') return 0;
        v = v * 10 + (u64)(s[i] - '0');
    }
    if (v < 1) return 0;
    *out = v;
    return 1;
}

int kparse_hex(const char *s, u64 n, u64 *out) {
    u64 v = 0, i;
    if (n < 1 || n > 16) return 0;
    for (i = 0; i < n; i++) {
        char c = s[i];
        u64 d;
        if (c >= '0' && c <= '9') d = (u64)(c - '0');
        else if (c >= 'a' && c <= 'f') d = (u64)(c - 'a') + 10;
        else if (c >= 'A' && c <= 'F') d = (u64)(c - 'A') + 10;
        else return 0;
        v = (v << 4) | d;
    }
    if (v < 1) return 0;
    *out = v;
    return 1;
}
