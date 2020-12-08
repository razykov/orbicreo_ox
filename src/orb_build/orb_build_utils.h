#ifndef ORB_BUILD_UTILS_H
#define ORB_BUILD_UTILS_H

#include <dirent.h>
#include "../orb_types/orb_json.h"

const char * orb_proj_type(json_object * project);

void orb_find_nexp_files(const char * dirpath,
                         json_object * array, const char * nexp);

bool orb_is_include_dir(struct dirent * dir);

void orb_monorepo_libs(char ** cmd, size_t * len);

json_object * orb_dependency_list(json_object * project);

json_object * orb_projects_set(void);

#endif /* ORB_BUILD_UTILS_H */
