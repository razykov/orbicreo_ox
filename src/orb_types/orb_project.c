#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/limits.h>

#include "orb_project.h"
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_types/orb_json.h"
#include "../orb_types/orb_context.h"


static void _self_cleaning(json_object * dep_list)
{
    for(ssize_t i = json_object_array_length(dep_list) - 1; i >= 0; --i) {
        json_object * dep = json_object_array_get_idx(dep_list, i);
        const char * dep_str = json_object_get_string(dep);
        if (!strcmp(dep_str, context.target_proj))
            json_object_array_del_idx(dep_list, i, 1);
    }
}

static json_object * _json_unlink(json_object * obj)
{
    json_object * t = json_object_get(obj);
    json_object_put(obj);
    return t;
}

static bool _dependency_list(struct orb_project * project)
{
    json_object * obj = orb_json_find(project->recipe.obj, "dependency_list");
    if (obj) {
        obj = _json_unlink(obj);
        _self_cleaning(obj);
        project->recipe.dependency_list = obj;
    } else {
        project->recipe.dependency_list = json_object_new_array();
    }
    return project->recipe.dependency_list != NULL;
}

static bool _dependency_include(struct orb_project * project)
{
    json_object * obj = orb_json_find(project->recipe.obj, "dependency_include");
    if (obj) {
        obj = _json_unlink(obj);
        project->recipe.dependency_include = obj;
    } else {
        project->recipe.dependency_include = json_object_new_array();
    }
    return project->recipe.dependency_include != NULL;
}

static inline const char * _directory_dest(struct orb_project * project)
{
    const char * dist;
    dist = orb_json_get_string(project->recipe.obj, "directory_dest");
    return dist ? dist : "";
}

static const char * _dest(struct orb_project * project)
{
    static __thread char buff[ORB_PATH_SZ];

    sprintf(buff, "%s/bin/%s", context.root, _directory_dest(project));
    if (buff[strlen(buff) - 1] != '/')
        buff[strlen(buff)] = '/';

    return buff;
}

static inline bool _output_file_path(struct orb_project * project)
{
    const char * fmt = strcmp(project->type, "shared") ? "%s%s" : "%slib%s.so";
    asprintf(&project->recipe.output_file, fmt, _dest(project), project->name);
    return project->recipe.output_file != NULL;
}

static bool _objs_path(struct orb_project * project)
{
    asprintf(&project->objs_path,
             "%s/build/%s/obj", context.root, project->name);
    return project->objs_path != NULL;
}

static bool _json_receipt_parse(struct orb_project * prj)
{
    prj->recipe.obj = json_object_from_file(prj->recipe_path);
    if (!prj->recipe.obj) {
        orb_err("error while project recipe read");
        return false;
    }

    prj->recipe.obj = orb_json_find(prj->recipe.obj, "general");
    if (!prj->recipe.obj) {
        orb_err("general receipt not found");
        return false;
    }

    prj->name = orb_json_get_string(prj->recipe.obj, "project_name");
    if (!prj->name) {
        orb_err("project_name not found");
        return false;
    }

    if (context.goal != ORB_GOAL_BUILD)
        return true;

    prj->type = orb_json_get_string(prj->recipe.obj, "project_type");
    if (!prj->type) {
        orb_err("project_type not found");
        return false;
    }

    if (!_dependency_list(prj)) {
        orb_err("dependency_list creation failed");
        return false;
    }

    if (!_dependency_include(prj)) {
        orb_err("dependency_include creation failed");
        return false;
    }

    prj->recipe.exp_incl = orb_json_find(prj->recipe.obj, "export_includes");

    if (!_output_file_path(prj)) {
        orb_err("output_file creation failed");
        return false;
    }

    if (!_objs_path(prj)) {
        orb_err("objs_path creation failed");
        return false;
    }

    return true;
}

static bool _proj_paths_init(struct orb_project * project, const char * dname)
{
    project->dirname = strdup(dname);
    orb_try(project->dirname);

    project->root = strdup(orb_cat(context.repo_projects, dname));
    orb_try(project->root);

    project->recipe_path = strdup(orb_cat(project->root, "recipe.json"));
    orb_try(project->recipe_path);

    project->version_path = strdup(orb_cat(project->root, "version.json"));
    orb_try(project->version_path);

    return true;
}

static json_object * _new_version(void)
{
    json_object * json = orb_json_object(NULL, NULL);
    orb_json_i32(json, "major", 0);
    orb_json_i32(json, "minor", 0);
    orb_json_i32(json, "build", 0);
    return json;
}

static bool _json_version_parse(struct orb_project * project)
{
    project->version = json_object_from_file(project->version_path);

    project->version = _json_unlink(project->version);
    if (!project->version)
        project->version = _new_version();

    return project->version != NULL;
}

static json_object * _new_meta(void)
{
    json_object * meta = orb_json_object(NULL, NULL);
    orb_json_string(meta, "recipe_hash", "");
    return meta;
}

static bool _json_meta_parse(struct orb_project * project)
{
    i32 r = asprintf(&project->meta_path,
                     "%s/build/%s/meta.json", context.root, project->name);
    if (r == -1) {
        orb_err("meta_path allocation failed");
        return false;
    }

    project->meta = json_object_from_file(project->meta_path);
    if (!project->meta)
        project->meta = _new_meta();

    return true;
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

static void _find_nexp_files(const char * dirpath,
                             json_object * array, const char * nexp)
{
    DIR * d = opendir(dirpath);
    char buff[ORB_PATH_SZ];
    if (d) {
        struct dirent * dir;
        while ((dir = readdir(d)) != NULL) {
            sprintf(buff, "%s/%s", dirpath, dir->d_name);
            if (_is_nexp(dir, nexp))          orb_json_string(array, NULL, buff);
            else if (orb_is_include_dir(dir)) _find_nexp_files(buff, array, nexp);
        }
        closedir(d);
    }
}

struct orb_project * orb_proj_create(const char * dname)
{
    struct orb_project * project;

    project = malloc(sizeof(struct orb_project));
    if (!project) {
        orb_err("project memory alloc error");
        return NULL;
    }

    project->built = false;
    project->compile_turn = false;

    if (!_proj_paths_init(project, dname))
        goto proj_add_fail;

    if (!_json_receipt_parse(project))
        goto proj_add_fail;

    if (context.goal != ORB_GOAL_BUILD)
        return project;

    if (!_json_version_parse(project))
        goto proj_add_fail;

    if (!_json_meta_parse(project))
        goto proj_add_fail;

    project->files.o     = json_object_new_array();

    project->files.h     = json_object_new_array();
    _find_nexp_files(project->root, project->files.h, ".h");

    project->files.c     = json_object_new_array();
    _find_nexp_files(project->root, project->files.c, ".c");

    project->files.o_old = json_object_new_array();
    _find_nexp_files(project->objs_path, project->files.o_old, ".o");

    return project;

proj_add_fail:
    orb_project_free(project);
    return NULL;
}

void orb_project_free(struct orb_project * project)
{
    if (project) {
        safe_free(project->root);
        safe_free(project->dirname);
        safe_free(project->meta_path);
        safe_free(project->recipe_path);
        safe_free(project->version_path);

        json_object_put(project->recipe.obj);
        project->recipe.obj = NULL;
        project->recipe.exp_incl = NULL;
        project->recipe.dependency_list = NULL;
        project->recipe.output_file = NULL;

        free(project);
    }
}
