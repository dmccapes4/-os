/* host.h: what the bench board gives the test. */
#ifndef KYRIOS_HOST_H
#define KYRIOS_HOST_H

#include "../kernel/lib.h"

#define HOST_HZ      54000000UL
#define HOST_MEMORY  (32UL << 20)

extern int host_show;               /* also write the console to stdout */

const char *host_output(void);      /* the console since the last host_clear */
void host_clear(void);
void host_feed(const char *s);      /* bytes for con_getc */
int  host_pending(void);
void host_advance(u64 counts);      /* move the counter */

#endif
