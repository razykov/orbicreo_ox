#define _GNU_SOURCE
#include <dirent.h>
#include <string.h>
#include <json-c/json.h>
#include "orb_goal_build.h"
#include "orb_build_link.h"
#include "orb_build_utils.h"
#include "orb_build_compile.h"
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_args.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_job_agent/orb_job_agent.h"
#include "../orb_build/orb_build_mkinclude.h"


static bool _build_project(json_object * project);

static bool _init_dirs(void)
{
    if (!orb_mkdir_p(orb_cat(context->root, "build")))
        return false;
    if (!orb_mkdir_p(orb_cat(context->root, "bin")))
        return false;
    return true;
}

static json_object * _files_array(const char * proj_path, const char * ext)
{
    json_object * array = orb_json_array(NULL, NULL);
    orb_find_nexp_files(proj_path, array, ext);
    return array;
}

static bool _build_depends(json_object * project)
{
    json_object * dep_list = orb_dependency_list(project);

    if(dep_list)
        for(size_t i = 0; i < json_object_array_length(dep_list); ++i) {
            json_object * dep = json_object_array_get_idx(dep_list, i);
            const char * depstr = json_object_get_string(dep);

            dep = orb_json_find(context->proj_set_json, depstr);
            if (dep) {
                if(_build_project(dep)) {
                    json_object * obj = orb_json_find(dep, "compile_turn");
                    if(json_object_get_boolean(obj))
                        orb_json_bool(project, "compile_turn", true);
                } else {
                    orb_err("project '%s' build failed", depstr);
                    return false;
                }
            }
        }
    return true;
}

static const char * _objs_path(json_object * project)
{
    static __thread char objdir[ORB_PATH_SZ] = { 0 };
    sprintf(objdir, "%s/build/%s/obj", context->root,
                                       orb_json_get_string(project, "dirname"));
    return objdir;
}

static const char * _directory_dest(json_object * project)
{
    const char * dist;
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    dist = orb_json_get_string(project, "directory_dest");
    return dist ? dist : "";
}

static const char * _dest(json_object * project)
{
    static __thread char buff[ORB_PATH_SZ];

    sprintf(buff, "%s/bin/%s", context->root, _directory_dest(project));
    if (buff[strlen(buff) - 1] != '/')
        buff[strlen(buff)] = '/';

    return buff;
}

static const char * _output_file_path(json_object * project)
{
    static __thread char buff[ORB_ROOT_SZ];
    const char * type = orb_proj_type(project);
    const char * name = orb_json_get_string(project, "project_name");

    if (!strcmp(type, "shared"))
        sprintf(buff, "%slib%s.so", _dest(project), name);
    else
        sprintf(buff, "%s%s", _dest(project), name);

    return buff;
}

static bool _clear_project(json_object * project) {
    const char * output_file = orb_json_get_string(project, "output_file");
    const char * objs_path = orb_json_get_string(project, "objs_path");

    orb_try(orb_rmrf(output_file));
    orb_try(orb_rmrf(objs_path));

    return true;
}

static void _meta_update(json_object * project, const char * meta_path,
                         json_object * meta, const char * new_cache)
{
    orb_json_bool(project, "compile_turn", true);
    orb_json_string(meta, "cfg_cache", new_cache);

    json_object_to_file(meta_path, meta);
}

static bool _clear_project_by_context(json_object * project)
{
    json_object * meta = orb_json_find(project, "meta");
    const char * cfg_cache = orb_json_get_string(meta, "cfg_cache");
    const char * project_name = orb_json_get_string(project, "project_name");
    const char * cfg_path = orb_json_get_string(project, "recipe_path");
    const char * meta_path = orb_json_get_string(project, "meta_path");
    const char * cfg_cache_new;

    if (!strcmp(project_name, context->target_proj)) {
        if (context->clear)      orb_try(_clear_project(project));
    } else {
        if (context->clear_deps) orb_try(_clear_project(project));
    }

    cfg_cache_new = orb_sha2str(orb_file_sha1(cfg_path));
    if (strcmp(cfg_cache, cfg_cache_new)) {
        _clear_project(project);
        _meta_update(project, meta_path, meta, cfg_cache_new);
    }

    return true;
}

static bool _build_project(json_object * project)
{
    bool ret;
    const char * project_path;
    const char * objs_path;
    const char * project_name;

    ret = json_object_get_boolean(orb_json_find(project, "built"));
    if (ret)
        return true;

    project_path = orb_json_get_string(project, "project_path");
    project_name = orb_json_get_string(project, "project_name");

    orb_try(_build_depends(project));

    orb_ret();
    orb_inf("Project %s%s%s build started",
            ORB_COL(ORB_COLOUR_CYAN), project_name, ORB_COL(ORB_COLOUR_WHITE));
    orb_stat(CYN, "Directory of project", "%s", project_path);

    objs_path = _objs_path(project);

    orb_json_string(project, "output_file", _output_file_path(project));
    orb_json_string(project, "objs_path", objs_path);

    orb_try(_clear_project_by_context(project));

    orb_try(orb_mkdir_p(objs_path));
    orb_json_move(project, _files_array(project_path, ".c"), "c_files");
    orb_json_move(project, _files_array(objs_path, ".o"), "o_files_old");

    orb_try(orb_compile_project(project));
    orb_try(orb_link_project(project));
    orb_try(orb_mkinclude(project));

    orb_json_bool(project, "built", true);

    return true;
}

bool orb_goal_build(void)
{
    if (!_init_dirs())
        return false;

    context->proj_set_json = orb_projects_set();

    orb_agents_start(context->njobs);

    if (context->target_proj) {
        json_object * project;
        project = orb_json_find(context->proj_set_json, context->target_proj);
        if (project) {
            orb_try(_build_project(project));
        } else orb_usrerr("project '%s' not found in monorepo", context->target_proj);
    } else {
        orb_usrerr("Use flag -p for give the name of the project. "
                   "Use flag -l to show list of projects");
    }

    orb_agent_stop();

    return true;
}
