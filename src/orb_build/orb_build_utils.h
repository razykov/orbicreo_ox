#ifndef ORB_BUILD_UTILS_H
#define ORB_BUILD_UTILS_H

#include <stdio.h>

/*!
 * \brief Expand string for list looks in directory for library files
 * \param cmd Expandable string
 * \param len Lenght of cmd string
 * \return Expanded strng poiner
 */
char * orb_monorepo_libs(char * cmd, size_t * len);

#endif /* ORB_BUILD_UTILS_H */
