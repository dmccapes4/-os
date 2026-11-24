/* sha256.h: SHA-256 (FIPS 180-4). */
#ifndef KYRIOS_SHA256_H
#define KYRIOS_SHA256_H

#include "lib.h"

struct sha256 {
    u32 h[8];
    u8  block[64];
    u64 fill;       /* bytes waiting in block */
    u64 total;      /* bytes taken in so far */
};

void sha256_begin(struct sha256 *s);
void sha256_add(struct sha256 *s, const void *data, u64 n);
void sha256_end(struct sha256 *s, u8 out[32]);
void sha256_of(const void *data, u64 n, u8 out[32]);

#endif
