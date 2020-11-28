#ifndef ORB_LOG_H
#define ORB_LOG_H

#include <stdio.h>
#include "../src/orb_types.h"

#define FLAG_NONE  (0 << 0)
#define FLAG_ERROR (1 << 0)
#define FLAG_TIME  (1 << 1)

/*! \brief Print error message */
#define orb_err(fmt, ...) _cprint(stderr, ORB_COLOUR_A8_RED, \
                                      FLAG_TIME | FLAG_ERROR, \
                                      __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
/*! \brief Print warning message */
#define orb_wrn(fmt, ...) _cprint(stderr, ORB_COLOUR_A8_YELLOW, FLAG_TIME, \
                                      __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
/*! \brief Print information message */
#define orb_inf(fmt, ...) _cprint(stdout, ORB_COLOUR_A8_BLUE, FLAG_TIME, \
                                      "", fmt, ##__VA_ARGS__)
/*! \brief Print plain text */
#define orb_txt(fmt, ...) _cprint(stdout, ORB_COLOUR_A8_WHITE, FLAG_NONE, \
                                      "", fmt, ##__VA_ARGS__)

/*!
 * \brief Basic print function
 * \param colour Colour of text
 * \param err Flag, errno print
 * \param func Name of current function
 * \param fmt Formatter
 */
void _cprint(FILE * stream, u8 colour, u8 flags,
                            const char * func, const char * fmt, ...);

#endif /* ORB_LOG_H */
