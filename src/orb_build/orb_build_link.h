#ifndef ORB_BUILD_LINK_H
#define ORB_BUILD_LINK_H

#include "../orb_types/orb_context.h"

/*!
 * \brief Link all .o files of project to binary file
 * \param project Project context
 * \return true on success
 */
bool orb_link_project(struct orb_project * project);

#endif /* ORB_BUILD_LINK_H */
