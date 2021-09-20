#ifndef ORB_LOG_H
#define ORB_LOG_H

#include <stdio.h>
#include "../orb_types/orb_types.h"

#define FLAG_NONE  (0 << 0)
#define FLAG_ERROR (1 << 0)

#define RED ORB_COLOUR_RED
#define BLU ORB_COLOUR_BLUE
#define CYN ORB_COLOUR_CYAN
#define GRN ORB_COLOUR_GREEN
#define PPL ORB_COLOUR_PURPLE
#define WHT ORB_COLOUR_WHITE

/*! \brief Print error message */
#define orb_err(fmt, ...) _cprint(stderr, ORB_COLOUR_RED, FLAG_ERROR, \
                                  __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

/*! \brief Print user error message */
#define orb_usrerr(fmt, ...) _cprint(stderr, ORB_COLOUR_RED, FLAG_NONE, \
                                     "", fmt, ##__VA_ARGS__)

/*! \brief Print warning message */
#define orb_wrn(fmt, ...) _cprint(stderr, ORB_COLOUR_YELLOW, FLAG_NONE, \
                                  __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

/*! \brief Print information message */
#define orb_inf(fmt, ...) _cprint(stdout, ORB_COLOUR_WHITE, FLAG_NONE, \
                                  "", fmt, ##__VA_ARGS__)

/*! \brief Print status message */
#define orb_stat(colour, name, fmt, ...) _stprint(stdout, colour, name, \
                                                  fmt, ##__VA_ARGS__)

/*! \brief Print plain text */
#define orb_txt(fmt, ...) _cprint(stdout, ORB_COLOUR_WHITE, FLAG_NONE, \
                                  "", fmt, ##__VA_ARGS__)

#define orb_ret(count) printf("%*s", count, "\n");

/*!
 * \brief Basic print function
 * \param stream Output stream
 * \param colour Colour of text
 * \param flags Flags for other features
 * \param func Name of current function
 * \param fmt Formatter
 */
void _cprint(FILE * stream, const char *colour, u8 flags,
                            const char * func, const char * fmt, ...);

/*!
 * \brief Status print function
 * \param stream Output stream
 * \param colour Colour of text
 * \param name Status parameter
 * \param fmt Formatter
 */
void _stprint(FILE * stream, const char * colour,
              const char * name, const char * fmt, ...);

#endif /* ORB_LOG_H */
