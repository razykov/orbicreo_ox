#ifndef ORB_TYPES_H
#define ORB_TYPES_H

#include <stddef.h>
#include <json-c/json.h>

#define ORB_COLOUR_CLEAR  "0"
#define ORB_COLOUR_RED    "1;31"
#define ORB_COLOUR_GREEN  "1;32"
#define ORB_COLOUR_YELLOW "1;33"
#define ORB_COLOUR_BLUE   "1;34"
#define ORB_COLOUR_PURPLE "0;35"
#define ORB_COLOUR_CYAN   "1;36"
#define ORB_COLOUR_WHITE  "1;37"

#define ORB_COL(color) "\033["color"m"

#define B_KB(VAL) (1024 * (VAL))

#define ORB_APPNAME_SZ (B_KB(1))
#define ORB_ROOT_SZ    (B_KB(4))
#define ORB_PATH_SZ    (ORB_ROOT_SZ + 512)

typedef   signed char i8;
typedef unsigned char u8;
typedef   signed int i32;
typedef unsigned int u32;

#if !defined(__cplusplus) && !defined(__bool_true_false_are_defined) && !defined(bool)
typedef enum {
    false,
    true
} bool;
#endif

enum orb_goal {
    ORB_GOAL_BUILD = 0,
    ORB_GOAL_HELP,
    ORB_GOAL_INIT,
    ORB_GOAL_LIST
};

struct orb_project {
    char * name;
};

struct orb_ctx {
    char appname[ORB_APPNAME_SZ];
    char root[ORB_ROOT_SZ];
    char repo_projects[ORB_PATH_SZ];

    char * target_proj;
    json_object * proj_set_json;

    struct {
        u32 ncount;
        struct orb_project * data;
    } projects;

    enum orb_goal goal;

    bool verbose;
};

struct orb_bts {
    u8 * data;
    size_t size;
};

struct orb_bts * orb_bts_malloc(size_t size);
void orb_bts_free(struct orb_bts * bts);

struct orb_ctx * orb_ctx_create(char ** argv);
void orb_ctx_destroy(struct orb_ctx * ctx);

void orb_cxt_proj_expand(struct orb_ctx * ctx);

void orb_bts_append_u8(struct orb_bts * bts, u8 byte);

#endif /* ORB_TYPES_H */
