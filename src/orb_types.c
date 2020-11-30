#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/orb_log.h"

void orb_bts_free(struct orb_bts * bts) {
    if (bts) {
        if (bts->data)
            free(bts->data);
        free(bts);
    }
}

struct orb_bts * orb_bts_malloc(size_t size) {
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

void orb_bts_append_u8(struct orb_bts * bts, u8 byte) {
    if (bts) return;

    bts->data = realloc(bts->data, bts->size + sizeof(u8));
    bts->data[bts->size] = byte;
    ++bts->size;
}

static void _name_read(struct orb_ctx * ctx, char ** argv) {
    char * ptr = argv[0];
    char * name = ptr;

    do {
        if (*ptr == '/')
            name = ++ptr;
    } while(*(ptr++)) ;

    strncpy(ctx->appname, name, ORB_APPNAME_SZ);
}

static void _get_root(struct orb_ctx * ctx) {
    if(!strlen(ctx->root))
        getcwd(ctx->root, sizeof(ctx->root));

    snprintf(ctx->proj_path, ORB_PATH_SZ, "%s/projects", ctx->root);
}

struct orb_ctx * orb_ctx_create(char ** argv) {
    struct orb_ctx * ctx = calloc(1, sizeof(struct orb_ctx));

    if (!ctx) return NULL;

    ctx->goal = ORB_GOAL_BUILD;
    ctx->target_proj = NULL;
    ctx->proj_set = NULL;

    _name_read(ctx, argv);
    _get_root(ctx);

    return ctx;
}

void orb_ctx_destroy(struct orb_ctx * ctx) {
    if (ctx) {
        if (ctx->target_proj)
            free(ctx->target_proj);
        if (ctx->proj_set)
            json_object_put(ctx->proj_set);
        free(ctx);
    }
}
