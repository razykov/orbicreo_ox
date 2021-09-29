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

static void _name_read(struct orb_ctx * ctx, char ** argv)
{
    char * ptr = argv[0];
    char * name = ptr;

    do {
        if (*ptr == '/')
            name = ++ptr;
    } while(*(ptr++)) ;

    strncpy(ctx->appname, name, ORB_APPNAME_SZ);
}

static void _get_root(struct orb_ctx * ctx)
{
    if(!strlen(ctx->root))
        getcwd(ctx->root, sizeof(ctx->root));

    snprintf(ctx->repo_projects, ORB_PATH_SZ, "%s/projects", ctx->root);
}

struct orb_ctx * orb_ctx_create(char ** argv)
{
    struct orb_ctx * ctx = calloc(1, sizeof(struct orb_ctx));

    if (!ctx) return NULL;

    ctx->goal = ORB_GOAL_BUILD;
    ctx->target_proj = NULL;
    ctx->proj_set_json = NULL;

    ctx->projects.data = malloc(0);
    ctx->projects.ncount = 0;

    ctx->njobs = 1;

    ctx->verbose = false;
    ctx->clear = false;
    ctx->clear_deps = false;

    _name_read(ctx, argv);
    _get_root(ctx);

    return ctx;
}

static void _proj_free(struct orb_project * proj)
{
    if (proj->name) free(proj->name);
}

void orb_ctx_destroy(struct orb_ctx * ctx)
{
    if (ctx) {
        if (ctx->target_proj)
            free(ctx->target_proj);
        if (ctx->proj_set_json)
            json_object_put(ctx->proj_set_json);

        if (ctx->projects.data) {
            for(u32 i = 0; i < ctx->projects.ncount; ++i)
                _proj_free(&ctx->projects.data[i]);

            free(ctx->projects.data);
            ctx->projects.ncount = 0;
        }

        free(ctx);
    }
}

void orb_cxt_proj_expand(struct orb_ctx * ctx)
{
    u32 size = ++ctx->projects.ncount * sizeof(struct orb_project);
    ctx->projects.data = realloc(ctx->projects.data, size);
}
