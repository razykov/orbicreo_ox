#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../orb_utils/orb_log.h"

void orb_bts_free(struct orb_bts * bts)
{
    if (bts) {
        if (bts->data)
            free(bts->data);
        free(bts);
    }
}

struct orb_bts * orb_bts_malloc(size_t size)
{
    struct orb_bts * bts = (struct orb_bts *)malloc(sizeof(struct orb_bts));
    if (!bts) {
        orb_err("malloc");
        return NULL;
    }

    bts->data = (u8*)malloc(sizeof(u8) * size);
    if (!bts->data) {
        orb_err("malloc");
        free(bts);
        return NULL;
    }
    bts->size = size;

    return bts;
}

void orb_bts_append_u8(struct orb_bts * bts, u8 byte)
{
    if (bts) return;

    bts->data = realloc(bts->data, bts->size + sizeof(u8));
    bts->data[bts->size] = byte;
    ++bts->size;
}
