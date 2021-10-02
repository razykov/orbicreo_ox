#ifndef ORB_BUILD_COMPILE_H
#define ORB_BUILD_COMPILE_H

#include "../orb_types/orb_context.h"

/*!
 * \brief Compile all .c files of project
 * \param project Project context
 * \return true on success
 */
bool orb_compile_project(struct orb_project * project);

#endif /* ORB_BUILD_COMPILE_H */
