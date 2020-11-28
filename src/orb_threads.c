#include <stdlib.h>
#include "../src/orb_log.h"
#include "../src/orb_threads.h"

struct orb_thrds {
    pthread_t * array;
    size_t size;
};

orb_thrds_t orb_thrd_create(void) {
    struct orb_thrds * thrds = malloc(sizeof(struct orb_thrds));

    if (!thrds) return NULL;

    thrds->array = NULL;
    thrds->size = 0;

    return thrds;
}

static void _thrd_free(orb_thrds_t thrds) {
    if (thrds) {
        if (thrds->array)
            free(thrds->array);
        free(thrds);
    }
}

bool orb_thrds_append(orb_thrds_t contaier, void * (*func)(void*), void * payload) {
    pthread_t thrd;

    if (!contaier || !func) return false;

    ++contaier->size;
    contaier->array = realloc(contaier->array, contaier->size * sizeof(pthread_t));
    if (!contaier->array) return false;

    pthread_create(&thrd, NULL, func, payload);
    contaier->array[contaier->size - 1] = thrd;

    return true;
}

bool orb_thrds_join(orb_thrds_t thrds, orb_thrd_parse_f func) {
    bool res = true;
    if (!thrds) return false;

    for(u32 i = 0; i < thrds->size; ++i) {
        void * thrd_ret;
        pthread_join(thrds->array[i], &thrd_ret);
        if (!func(thrd_ret))
            res = false;
    }

    _thrd_free(thrds);

    return res;
}
