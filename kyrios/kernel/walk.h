/* walk.h: the walk. A declared scope and separate steps, in place of the
 * for loop. See docs/KYRIOS_TECHNICAL_SPECIFICATION_v0.0.1.md, section 2.3.
 *
 *     x in [1..10]      kwalk_in(&x, 1, 10)      x is absent
 *     x.next            kwalk_next(&x, 1)        1
 *     x.now(5)          kwalk_now(&x, 5)         5
 *     x.next(2)         kwalk_next(&x, 2)        7
 *     x.back            kwalk_back(&x, 1)        6
 *     x.back(2)         kwalk_back(&x, 2)        4
 *     x.restart         kwalk_restart(&x)        absent
 *
 * The position cannot leave the scope. A step that would leave it is
 * refused and the position does not move. Declaring the scope again is a
 * second call to kwalk_in.
 */
#ifndef KYRIOS_WALK_H
#define KYRIOS_WALK_H

#include "lib.h"

enum kplace  { K_ABSENT = 1, K_PLACED };
enum kresult { K_DONE = 1, K_EDGE, K_NOT_PLACED };

struct kwalk {
    u64 first, last;      /* the scope, both ends included */
    u64 at;               /* the position; meaningful only when placed */
    enum kplace place;
};

void         kwalk_in(struct kwalk *w, u64 first, u64 last);
enum kresult kwalk_next(struct kwalk *w, u64 n);
enum kresult kwalk_back(struct kwalk *w, u64 n);
enum kresult kwalk_now(struct kwalk *w, u64 k);
void         kwalk_restart(struct kwalk *w);

#endif
