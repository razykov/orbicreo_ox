#ifndef ORB_BUILD_UTILS_H
#define ORB_BUILD_UTILS_H

#include <dirent.h>
#include "../orb_types/orb_json.h"
#include "../orb_types/orb_context.h"

const char * orb_proj_type(json_object * project);

void orb_monorepo_libs(char ** cmd, size_t * len);

#endif /* ORB_BUILD_UTILS_H */
