#ifndef ORB_TYPES_H
#define ORB_TYPES_H

#include <stddef.h>

#define ORB_COLOUR_A8_RED    (1)
#define ORB_COLOUR_A8_YELLOW (3)
#define ORB_COLOUR_A8_BLUE   (4)
#define ORB_COLOUR_A8_CYAN   (6)
#define ORB_COLOUR_A8_WHITE  (7)

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
    ORB_GOAL_INIT
};

struct orb_ctx {
    char appname[ORB_APPNAME_SZ];
    char root[ORB_ROOT_SZ];
    char proj_path[ORB_PATH_SZ];

    enum orb_goal goal;
};

struct orb_bts {
    u8 * data;
    size_t size;
};

void orb_bts_free(struct orb_bts * bts);
struct orb_bts * orb_bts_malloc(size_t size);

void orb_bts_append_u8(struct orb_bts * bts, u8 byte);

#endif /* ORB_TYPES_H */
