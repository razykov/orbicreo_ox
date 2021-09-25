#define _GNU_SOURCE
#include <dirent.h>
#include <string.h>
#include <json-c/json.h>
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_args.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_build/orb_goal_build.h"
#include "../orb_build/orb_build_link.h"
#include "../orb_build/orb_build_utils.h"
#include "../orb_build/orb_build_compile.h"
#include "../orb_build/orb_build_mkinclude.h"


static bool _build_project(json_object * project);

static bool _init_dirs(void)
{
    if (!orb_mkdir_p(orb_cat(context->root, "build/obj")))
        return false;
    if (!orb_mkdir_p(orb_cat(context->root, "bin")))
        return false;
    return true;
}

static json_object * _src_files(const char * proj_path)
{
    json_object * array = orb_json_array(NULL, NULL);
    orb_find_nexp_files(proj_path, array, ".c");
    return array;
}

static bool _mkobjdir(json_object * project)
{
    char objdir[ORB_PATH_SZ] = { 0 };
    sprintf(objdir, "%s/build/obj/%s", context->root,
                                       orb_json_get_string(project, "dirname"));
    orb_try(orb_mkdir_p(objdir));
    orb_json_string(project, "objs_path", objdir);

    return true;
}

static bool _build_depends(json_object * project)
{
    json_object * dep_list = orb_dependency_list(project);

    if(dep_list)
        for(size_t i = 0; i < json_object_array_length(dep_list); ++i) {
            json_object * dep = json_object_array_get_idx(dep_list, i);
            const char * depstr = json_object_get_string(dep);

            dep = orb_json_find(context->proj_set_json, depstr);
            if (dep)
                if(!_build_project(dep)) {
                    orb_err("project '%s' build failed", depstr);
                    return false;
                }
        }
    return true;
}

static bool _build_project(json_object * project)
{
    bool ret = json_object_get_boolean(orb_json_find(project, "built"));
    if (ret)
        return true;

    orb_try(_build_depends(project));

    orb_ret(1);
    orb_inf("Project %s%s%s build started",
            ORB_COL(ORB_COLOUR_CYAN),
            orb_json_get_string(project, "project_name"),
            ORB_COL(ORB_COLOUR_WHITE));
    orb_stat(CYN, "Directory of project", "%s",
                  orb_json_get_string(project, "project_path"));

    orb_try(_mkobjdir(project));
    orb_json_move(project,
                  _src_files(orb_json_get_string(project, "project_path")), "c_files");

    orb_try(orb_compile_project(project));
    orb_try(orb_link_project(project));
    orb_try(orb_mkinclude(project));

    orb_json_bool(project, "built", true);

    return true;
}

static void _rm_build(void)
{
    char buff[B_KB(5)];
    sprintf(buff, "rm -rf %s/build", context->root);
    system(buff);
}

bool orb_goal_build(void)
{
    _rm_build();
    if (!_init_dirs())
        return false;

    context->proj_set_json = orb_projects_set();

    if (context->target_proj) {
        json_object * proj = orb_json_find(context->proj_set_json, context->target_proj);
        if (proj) _build_project(proj);
        else orb_usrerr("project '%s' not found in monorepo", context->target_proj);
    } else {
        orb_usrerr("Use flag -p for give the name of the project. "
                   "Use flag -l to show list of projects");
    }

    return true;
}
