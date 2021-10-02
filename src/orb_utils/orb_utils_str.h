#ifndef ORB_UTILS_STR_H
#define ORB_UTILS_STR_H

#include <stdlib.h>

/*!
 * \brief Expand string and append tail
 * \param str Original string
 * \param len Original string lenght
 * \param tail Tail string
 * \return Expanded string
 */
char * orb_strexp(char * str, size_t * len, const char * tail);

/*!
 * \brief Replase all letters to uppercase
 * \param str Original string
 */
void orb_upstr(char * str);

#endif /* ORB_UTILS_STR_H */
