#define _GNU_SOURCE
#include <dirent.h>
#include <string.h>
#include <json-c/json.h>
#include "../src/orb_log.h"
#include "../src/orb_args.h"
#include "../src/orb_utils.h"
#include "../src/orb_build.h"
#include "../src/orb_build_link.h"
#include "../src/orb_build_compile.h"


static bool _init_dirs(void) {
    if (!orb_mkdir_p(orb_cat(context.root, "build/obj")))
        return false;
    if (!orb_mkdir_p(orb_cat(context.root, "bin")))
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

static void _proj_add(const char * dname, json_object * array) {
    json_object * tmp;
    json_object * json;
    char * path    = strdup(orb_cat(context.proj_path, dname));
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

    orb_json_move(array, json, NULL);

proj_add_fail:
    free(path);
    free(recipe);
}

static bool _is_include_dir(struct dirent * dir) {
    if (dir->d_type != DT_DIR)      return false;
    if (!strcmp(dir->d_name, "." )) return false;
    if (!strcmp(dir->d_name, "..")) return false;
    return true;
}

static json_object * _proj_list(void) {
    DIR * d;
    struct dirent * dir;
    json_object * array = orb_json_array(NULL, NULL);

    d = opendir(context.proj_path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (!_is_include_dir(dir)) continue;
            _proj_add(dir->d_name, array);
        }
        closedir(d);
    }

    return array;
}

static bool _is_c_file(struct dirent * dir) {
    if (!dir && dir->d_type != DT_REG) return false;

    size_t len = strlen(dir->d_name);
    if (dir->d_name[len - 1] == 'c' && dir->d_name[len - 2] == '.')
        return true;

    return false;
}

static void _find_c_files(const char * dirpath, json_object * array) {
    DIR * d = opendir(dirpath);
    char buff[ORB_PATH_SZ];
    if (d) {
        struct dirent * dir;
        while ((dir = readdir(d)) != NULL) {
            sprintf(buff, "%s/%s", dirpath, dir->d_name);
            if (_is_c_file(dir))           orb_json_string(array, NULL, buff);
            else if (_is_include_dir(dir)) _find_c_files(buff, array);
        }
        closedir(d);
    }
}

static json_object * _code_files(const char * proj_path) {
    json_object * array = orb_json_array(NULL, NULL);
    _find_c_files(proj_path, array);
    return array;
}

static void _build_project(json_object * proj) {
    char objdir[ORB_PATH_SZ] = { 0 };
    sprintf(objdir, "%s/build/obj/%s", context.root,
                                       orb_json_get_string(proj, "dirname"));
    orb_mkdir_p(objdir);
    orb_json_string(proj, "objs_path", objdir);

    orb_json_move(proj, _code_files(orb_json_get_string(proj, "path")), "c_files");

    orb_compile_project(proj);
    orb_link_project(proj);

    orb_txt("%s", json_object_to_json_string_ext(proj, JSON_C_TO_STRING_PRETTY));
}

static void _build_projects(json_object * projs) {
    for(size_t i = 0; i < json_object_array_length(projs); ++i)
        _build_project(json_object_array_get_idx(projs, i));
}

static void _rm_build(void) {
    char buff[B_KB(5)];
    sprintf(buff, "rm -rf %s/build", context.root);
    system(buff);
}

bool orb_build(void) {
    json_object * projs;

    _rm_build();
    if (!_init_dirs())
        return false;

    projs = _proj_list();

//    orb_txt("\n%s", json_object_to_json_string_ext(projs, JSON_C_TO_STRING_PRETTY));

    _build_projects(projs);

    json_object_put(projs);

    return true;
}
