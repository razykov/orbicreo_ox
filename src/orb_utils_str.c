#include <string.h>
#include "../src/orb_types.h"
#include "../src/orb_utils_str.h"


char * orb_strexp(char * str, size_t * len, const char * tail) {
    size_t tsize;

    if (!str || !len || !tail) return NULL;

    tsize = strlen(tail);

    str = realloc(str, *len + tsize + sizeof(u8));
    if (!str) return NULL;

    memcpy(str + (*len), tail, tsize);
    *len += tsize;
    str[*len - 0] = '\0';

    return str;
}
