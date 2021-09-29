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

static const char * _last_dot(const char * fname)
{
    const char * ptr = NULL;
    while (*fname) {
        if (*fname == '.') ptr = fname;
        ++fname;
    }
    return ptr;
}

static bool _is_nexp(struct dirent * dir, const char * nexp)
{
    const char * last_dot;

    if (dir->d_type != DT_REG) return false;

    last_dot = _last_dot(dir->d_name);
    if (!last_dot) return false;

    return strcmp(last_dot, nexp) == 0;
}

void orb_find_nexp_files(const char * dirpath,
                         json_object * array, const char * nexp)
{
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

bool orb_is_include_dir(struct dirent * dir)
{
    if (!dir || dir->d_type != DT_DIR) return false;
    if (!strcmp(dir->d_name, "." ))    return false;
    if (!strcmp(dir->d_name, ".."))    return false;
    return true;
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
    sprintf(buff, "%s/bin", context->root);
    _bin_subdirs(buff, cmd, len);
}

static void _self_cleaning(json_object * dep_list)
{
    if(dep_list)
        for(ssize_t i = json_object_array_length(dep_list) - 1; i >= 0; --i) {
            json_object * dep = json_object_array_get_idx(dep_list, i);
            const char * dep_str = json_object_get_string(dep);
            if (!strcmp(dep_str, context->target_proj))
                json_object_array_del_idx(dep_list, i, 1);
        }
}

json_object * orb_dependency_list(json_object * project)
{
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    project = orb_json_find(project, "dependency_list");

    _self_cleaning(project);

    return project;
}

static json_object * _new_version(void)
{
    json_object * json = orb_json_object(NULL, NULL);
    orb_json_i32(json, "major", 0);
    orb_json_i32(json, "minor", 0);
    orb_json_i32(json, "build", 0);
    return json;
}

static json_object * _new_meta(void)
{
    json_object * meta = orb_json_object(NULL, NULL);
    orb_json_string(meta, "cfg_cache", "");
    return meta;
}

static void _receipt_parse(json_object * proj, json_object * recipe)
{
    json_object * tmp;

    orb_json_move(proj, recipe, "recipe");

    tmp = orb_json_find(recipe, "general");
    orb_json_string(proj, "project_name",
                          orb_json_get_string(tmp, "project_name"));
}

static void _proj_add(const char * dname, json_object * proj_set_json)
{
    json_object * tmp;
    json_object * json;
    json_object * repo_root;
    char buff[ORB_ROOT_SZ];
    char * path    = strdup(orb_cat(context->repo_projects, dname));
    char * recipe  = strdup(orb_cat(path, "recipe.json"));
    char * version = strdup(orb_cat(path, "version.json"));

    tmp = json_object_from_file(recipe);
    if (!tmp) {
        orb_err("error while project config read");
        goto proj_add_fail;
    }

    json = orb_json_object(NULL, NULL);
    repo_root = orb_json_string(json, "repo_root", context->root);
    orb_json_string(json, "dirname", dname);
    orb_json_string(json, "project_path", path);
    orb_json_string(json, "recipe_path", recipe);

    orb_json_bool(json, "compile_turn", false);

    _receipt_parse(json, tmp);

    tmp = json_object_from_file(version);
    if (!tmp) tmp = _new_version();
    orb_json_move(json, tmp, "version");

    snprintf(buff, ORB_ROOT_SZ - 1,
             "%s/build/%s/meta.json", json_object_get_string(repo_root), dname);
    orb_json_string(json, "meta_path", buff);

    tmp = json_object_from_file(buff);
    if (!tmp) tmp = _new_meta();
    orb_json_move(json, tmp, "meta");

    orb_json_move(proj_set_json, json, orb_json_get_string(json, "project_name"));

proj_add_fail:
    free(path);
    free(recipe);
    free(version);
}

json_object * orb_projects_set(void)
{
    DIR * d;
    struct dirent * dir;
    json_object * list = orb_json_object(NULL, NULL);

    d = opendir(context->repo_projects);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (!orb_is_include_dir(dir)) continue;
            _proj_add(dir->d_name, list);
        }
        closedir(d);
    }

    return list;
}
