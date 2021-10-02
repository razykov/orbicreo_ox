#include <dirent.h>
#include "orb_build_utils.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_utils/orb_utils_str.h"
#include "../orb_types/orb_context.h"


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

char * orb_monorepo_libs(char * cmd, size_t * len)
{
    char buff[ORB_PATH_SZ];
    sprintf(buff, "%s/bin", context.root);
    _bin_subdirs(buff, &cmd, len);
    return cmd;
}
