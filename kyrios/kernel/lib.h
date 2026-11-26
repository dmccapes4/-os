/* lib.h: the few routines a freestanding build needs, and a text writer.
 * Part of the kernel layer: it provides primitives and holds no authority. */
#ifndef KYRIOS_LIB_H
#define KYRIOS_LIB_H

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;
typedef long           s64;

#ifdef KYRIOS_HOST
#include <string.h>
#else
void *memcpy(void *dst, const void *src, u64 n);
void *memmove(void *dst, const void *src, u64 n);
void *memset(void *dst, int c, u64 n);
int   memcmp(const void *a, const void *b, u64 n);
#endif

u64 kstrlen(const char *s);
int kstreq(const char *a, const char *b);
int kstreqn(const char *a, u64 an, const char *b);   /* a has length an; b ends in NUL */

/* A writer into a fixed buffer. When the text does not fit, `over` is set
 * and nothing past the capacity is written. */
struct kbuf {
    char *p;
    u64   len, cap;
    int   over;
};

void kbuf_init(struct kbuf *b, char *p, u64 cap);
void kbuf_putc(struct kbuf *b, char c);
void kbuf_put(struct kbuf *b, const char *s);
void kbuf_putn(struct kbuf *b, const char *s, u64 n);
void kbuf_hex(struct kbuf *b, u64 v);                  /* hexadecimal, no leading zeros */
void kbuf_hexw(struct kbuf *b, u64 v, int width);      /* fixed width, for labels */
void kbuf_hexbytes(struct kbuf *b, const u8 *p, u64 n);
void kbuf_dec(struct kbuf *b, u64 v);                  /* decimal, for the human interface */

/* Reading numbers typed at an interface. Both return 1 when the whole of
 * s[0..n) is a number of 1 or more that fits, and 0 otherwise. */
int kparse_dec(const char *s, u64 n, u64 *out);
int kparse_hex(const char *s, u64 n, u64 *out);

#endif
