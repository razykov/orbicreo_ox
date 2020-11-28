#ifndef ORB_THREADS_H
#define ORB_THREADS_H

#include <pthread.h>
#include "../src/orb_types.h"

typedef struct orb_thrds * orb_thrds_t;

typedef bool (*orb_thrd_parse_f)(void *);

orb_thrds_t orb_thrd_create(void);

bool orb_thrds_append(orb_thrds_t contaier, void * (*func)(void*), void * payload);

bool orb_thrds_join(orb_thrds_t thrds, orb_thrd_parse_f func);

#endif /* ORB_THREADS_H */
