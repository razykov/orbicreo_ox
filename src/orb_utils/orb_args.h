#ifndef ORB_ARGS_H
#define ORB_ARGS_H

#include "../orb_types/orb_types.h"

/*!
 * \brief Parse CLI arguments
 * \param argc CLI arguments count
 * \param argv CLI arguments
 * \return true on success
 */
bool orb_args_parse(i32 argc, char ** argv);

#endif /* ORB_ARGS_H */
