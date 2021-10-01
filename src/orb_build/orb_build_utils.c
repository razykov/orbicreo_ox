#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "orb_build_utils.h"
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_args.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_utils/orb_utils_str.h"

const char * orb_proj_type(json_object * project)
{
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    return orb_json_get_string(project, "project_type");
}

static void _bin_subdirs(const char * dirpath, char ** cmd, size_t * len)
{
    DIR * d;
    d = opendir(dirpath);
    char buff[ORB_PATH_SZ];

    *cmd = orb_strexp(*cmd, len, " -L");
    *cmd = orb_strexp(*cmd, len, dirpath);

    if (d) {
        struct dirent * dir;
        while ((dir = readdir(d)) != NULL) {
            sprintf(buff, "%s/%s", dirpath, dir->d_name);
            if (orb_is_include_dir(dir))
                _bin_subdirs(buff, cmd, len);
        }
        closedir(d);
    }
}

void orb_monorepo_libs(char ** cmd, size_t * len)
{
    char buff[ORB_PATH_SZ];
    sprintf(buff, "%s/bin", context.root);
    _bin_subdirs(buff, cmd, len);
}
