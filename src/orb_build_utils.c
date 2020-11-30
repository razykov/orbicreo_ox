#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "../src/orb_log.h"
#include "../src/orb_args.h"
#include "../src/orb_utils_str.h"
#include "../src/orb_build_utils.h"

const char * orb_proj_type(json_object * project) {
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    return orb_json_get_string(project, "project_type");
}

static const char * _last_dot(const char * fname) {
    const char * ptr = NULL;
    while (*fname) {
        if (*fname == '.') ptr = fname;
        ++fname;
    }
    return ptr;
}

static bool _is_nexp(struct dirent * dir, const char * nexp) {
    const char * last_dot;

    if (dir->d_type != DT_REG) return false;

    last_dot = _last_dot(dir->d_name);
    if (!last_dot) return false;

    return strcmp(last_dot, nexp) == 0;
}

void orb_find_nexp_files(const char * dirpath,
                         json_object * array, const char * nexp) {
    DIR * d = opendir(dirpath);
    char buff[ORB_PATH_SZ];
    if (d) {
        struct dirent * dir;
        while ((dir = readdir(d)) != NULL) {
            sprintf(buff, "%s/%s", dirpath, dir->d_name);
            if (_is_nexp(dir, nexp))          orb_json_string(array, NULL, buff);
            else if (orb_is_include_dir(dir)) orb_find_nexp_files(buff, array, nexp);
        }
        closedir(d);
    }
}

bool orb_is_include_dir(struct dirent * dir) {
    if (!dir || dir->d_type != DT_DIR) return false;
    if (!strcmp(dir->d_name, "." ))    return false;
    if (!strcmp(dir->d_name, ".."))    return false;
    return true;
}

static void _bin_subdirs(const char * dirpath, char ** cmd, size_t * len) {
    DIR * d;
    d = opendir(dirpath);
    char buff[ORB_PATH_SZ];
    if (d) {
        struct dirent * dir;
        while ((dir = readdir(d)) != NULL) {
            sprintf(buff, "%s/%s", dirpath, dir->d_name);
            if (orb_is_include_dir(dir))
                _bin_subdirs(buff, cmd, len);
        }

        *cmd = orb_strexp(*cmd, len, " -L");
        *cmd = orb_strexp(*cmd, len, buff);
        closedir(d);
    }
}

void orb_monorepo_libs(char ** cmd, size_t * len) {
    char buff[ORB_PATH_SZ];
    sprintf(buff, "%s/bin", context->root);
    _bin_subdirs(buff, cmd, len);
}

json_object * orb_dependency_list(json_object * project) {
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    project = orb_json_find(project, "dependency_list");
    return project;
}
