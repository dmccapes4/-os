/* walk.c: see walk.h. */
#include "walk.h"

void kwalk_in(struct kwalk *w, u64 first, u64 last) {
    w->first = first;
    w->last = last;
    w->place = K_ABSENT;
}

enum kresult kwalk_next(struct kwalk *w, u64 n) {
    u64 from  = (w->place == K_PLACED) ? w->at : w->first;
    u64 steps = (w->place == K_PLACED) ? n : n - 1;   /* the first step lands on first */
    if (steps > w->last - from) return K_EDGE;
    w->at = from + steps;
    w->place = K_PLACED;
    return K_DONE;
}

enum kresult kwalk_back(struct kwalk *w, u64 n) {
    if (w->place != K_PLACED) return K_NOT_PLACED;
    if (n > w->at - w->first) return K_EDGE;
    w->at -= n;
    return K_DONE;
}

enum kresult kwalk_now(struct kwalk *w, u64 k) {
    if (k < w->first || k > w->last) return K_EDGE;
    w->at = k;
    w->place = K_PLACED;
    return K_DONE;
}

void kwalk_restart(struct kwalk *w) {
    w->place = K_ABSENT;
}
