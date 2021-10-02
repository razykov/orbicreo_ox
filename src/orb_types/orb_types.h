#ifndef ORB_TYPES_H
#define ORB_TYPES_H

#include <stddef.h>

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

#endif /* ORB_TYPES_H */
