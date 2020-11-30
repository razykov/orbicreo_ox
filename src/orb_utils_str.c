#include <string.h>
#include "../src/orb_log.h"
#include "../src/orb_types.h"
#include "../src/orb_utils_str.h"

#define IS_SMALL_LETTER(LETTER) (('a' <= LETTER) && (LETTER <= 'z'))
#define IS_BIG_LETTER(LETTER)   (('A' <= LETTER) && (LETTER <= 'Z'))
#define IS_DIGIT(LETTER)        (('0' <= LETTER) && (LETTER <= '9'))

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

void orb_upstr(char * str) {
    if (!str) return;

    while(*str) {
        if      (IS_SMALL_LETTER(*str))                 *str += 'A' - 'a';
        else if (IS_BIG_LETTER(*str) || IS_DIGIT(*str)) ;
        else                                            *str = '_';
        ++str;
    }
}
