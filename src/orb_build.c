#define _GNU_SOURCE
#include <dirent.h>
#include <string.h>
#include <json-c/json.h>
#include "../src/orb_log.h"
#include "../src/orb_args.h"
#include "../src/orb_utils.h"
#include "../src/orb_build.h"
#include "../src/orb_build_link.h"
#include "../src/orb_build_utils.h"
#include "../src/orb_build_compile.h"
#include "../src/orb_build_mkinclude.h"


static bool _build_project(json_object * project);

static bool _init_dirs(void) {
    if (!orb_mkdir_p(orb_cat(context->root, "build/obj")))
        return false;
    if (!orb_mkdir_p(orb_cat(context->root, "bin")))
        return false;
    return true;
}

static json_object * _new_version(void) {
    json_object * json = orb_json_object(NULL, NULL);
    orb_json_i32(json, "major", 0);
    orb_json_i32(json, "minor", 0);
    orb_json_i32(json, "build", 0);
    return json;
}

static void _receipt_parse(json_object * proj, json_object * recipe) {
    json_object * tmp;

    orb_json_move(proj, recipe, "recipe");

    tmp = orb_json_find(recipe, "general");
    orb_json_string(proj, "project_name",
                          orb_json_get_string(tmp, "project_name"));
}

static void _proj_add(const char * dname, json_object * proj_set) {
    json_object * tmp;
    json_object * json;
    char * path    = strdup(orb_cat(context->proj_path, dname));
    char * recipe  = strdup(orb_cat(path, "recipe.json"));
    char * version = strdup(orb_cat(path, "version.json"));

    tmp = json_object_from_file(recipe);
    if (!tmp) goto proj_add_fail;

    json = orb_json_object(NULL, NULL);
    orb_json_string(json, "dirname", dname);
    orb_json_string(json, "path", path);

    _receipt_parse(json, tmp);

    tmp = json_object_from_file(version);
    if (!tmp) tmp = _new_version();
    orb_json_move(json, tmp, "version");

    orb_json_move(proj_set, json, orb_json_get_string(json, "project_name"));

proj_add_fail:
    free(path);
    free(recipe);
}

static json_object * _proj_set(void) {
    DIR * d;
    struct dirent * dir;
    json_object * list = orb_json_object(NULL, NULL);

    d = opendir(context->proj_path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (!orb_is_include_dir(dir)) continue;
            _proj_add(dir->d_name, list);
        }
        closedir(d);
    }

    return list;
}

static json_object * _code_files(const char * proj_path) {
    json_object * array = orb_json_array(NULL, NULL);
    orb_find_nexp_files(proj_path, array, ".c");
    return array;
}

static bool _mkobjdir(json_object * project) {
    char objdir[ORB_PATH_SZ] = { 0 };
    sprintf(objdir, "%s/build/obj/%s", context->root,
                                       orb_json_get_string(project, "dirname"));
    orb_try(orb_mkdir_p(objdir));
    orb_json_string(project, "objs_path", objdir);

    return true;
}

static bool _build_depends(json_object * project) {
    json_object * dep_list = orb_dependency_list(project);

    if(dep_list)
        for(size_t i = 0; i < json_object_array_length(dep_list); ++i) {
            json_object * dep = json_object_array_get_idx(dep_list, i);
            const char * depstr = json_object_get_string(dep);

            dep = orb_json_find(context->proj_set, depstr);
            if (dep)
                orb_try(_build_project(dep));
        }
    return true;
}

static bool _build_project(json_object * project) {
    _build_depends(project);

    orb_inf("%s build start", orb_json_get_string(project, "project_name"));

    orb_try(_mkobjdir(project));
    orb_json_move(project,
                  _code_files(orb_json_get_string(project, "path")), "c_files");

    orb_txt("%s", json_object_to_json_string_ext(project, JSON_C_TO_STRING_PRETTY));

    orb_txt("compile");
    orb_try(orb_compile_project(project));
    orb_txt("linking");
    orb_try(orb_link_project(project));
    orb_txt("project include generate");
    orb_try(orb_mkinclude(project));


    return true;
}

static void _rm_build(void) {
    char buff[B_KB(5)];
    sprintf(buff, "rm -rf %s/build", context->root);
    system(buff);
}

bool orb_build(void) {
    _rm_build();
    if (!_init_dirs())
        return false;

    context->proj_set = _proj_set();
//    orb_txt("\n%s", json_object_to_json_string_ext(context->proj_set,
//                                                   JSON_C_TO_STRING_PRETTY));

    if (context->target_proj) {
        json_object * proj = orb_json_find(context->proj_set, context->target_proj);
        if (proj) _build_project(proj);
        else orb_err("project '%s' not found in monorepo", context->target_proj);
    }

    return true;
}
