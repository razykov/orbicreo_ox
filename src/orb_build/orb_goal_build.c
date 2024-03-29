#define _GNU_SOURCE
#include <dirent.h>
#include <string.h>
#include <json-c/json.h>
#include "orb_goal_build.h"
#include "orb_build_link.h"
#include "orb_build_compile.h"
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_types/orb_context.h"
#include "../orb_job_agent/orb_job_agent.h"
#include "../orb_build/orb_build_mkinclude.h"


static bool _build_project(struct orb_project * project);

static bool _init_dirs(void)
{
    if (!orb_mkdir_p(orb_cat(context.root, "build")))
        return false;
    if (!orb_mkdir_p(orb_cat(context.root, "bin")))
        return false;
    return true;
}

static bool _build_depends(struct orb_project * project)
{
    json_object * dep_list = project->recipe.dependency_list;

    for(size_t i = 0; i < json_object_array_length(dep_list); ++i) {
        json_object * tmp = json_object_array_get_idx(dep_list, i);
        const char * depstr = json_object_get_string(tmp);
        struct orb_project * dep = orb_ctx_get_project(depstr);

        if (dep) {
            if(_build_project(dep)) {
                if (dep->compile_turn)
                    project->compile_turn = true;
            } else {
                orb_err("project '%s' build failed", depstr);
                return false;
            }
        }
    }

    return true;
}

static inline bool _clear_project(struct orb_project * project) {
    orb_try(orb_rmrf(project->recipe.output_file));
    orb_try(orb_rmrf(project->objs_path));

    orb_try(orb_mkdir_p(project->objs_path));
    return true;
}

static void _meta_update(struct orb_project * project, const char * new_hash)
{
    project->compile_turn = true;
    orb_json_string(project->meta, "recipe_hash", new_hash);

    json_object_to_file(project->meta_path, project->meta);
}

static bool _clear_project_by_context(struct orb_project * project)
{
    const char * recipe_hash_new;
    const char * recipe_hash = orb_json_get_string(project->meta, "recipe_hash");

    if (!strcmp(project->name, context.target_proj)) {
        if (context.clear)      orb_try(_clear_project(project));
    } else {
        if (context.clear_deps) orb_try(_clear_project(project));
    }

    recipe_hash_new = orb_sha2str(orb_file_sha1(project->recipe_path));
    if (strcmp(recipe_hash, recipe_hash_new)) {
        _clear_project(project);
        _meta_update(project, recipe_hash_new);
    }

    return true;
}

static bool _version_increment(struct orb_project * project)
{
    i32 r;
    if (project->compile_turn && context.release) {
        json_object * build = orb_json_find(project->version, "build");

        if (!json_object_int_inc(build, 1)) {
            orb_err("build increment failed");
            return false;
        }

        r = json_object_to_file_ext(project->version_path, project->version,
                                    JSON_C_TO_STRING_PRETTY);
        if (r == -1) {
            orb_err("version.json write error (%s)", json_util_get_last_err());
            return false;
        }
    }
    return true;
}

static bool _version_update(struct orb_project * project)
{
    i32 r;

    r = json_object_to_file_ext(project->version_path,
                                project->version, JSON_C_TO_STRING_PRETTY);
    if (r == -1) {
        orb_err("version.json write error (%s)", json_util_get_last_err());
        return false;
    }
    return true;
}

static bool _build_project(struct orb_project * project)
{
    if (project->built)
        return true;

    orb_try(_build_depends(project));

    orb_ret();
    orb_inf("Project %s%s%s build started",
            ORB_COL(ORB_COLOUR_CYAN), project->name, ORB_COL(ORB_COLOUR_WHITE));
    orb_stat(CYN, "Directory of project",
                  ".%s", project->root + context.rt_off);

    orb_try(orb_mkdir_p(project->objs_path));

    orb_try(_clear_project_by_context(project));

    orb_try(orb_compile_project(project));
    orb_try(_version_increment(project));
    orb_try(orb_link_project(project));
    orb_try(orb_mkinclude(project));

    orb_try(_version_update(project));

    project->built = true;

    return true;
}

bool orb_goal_build(void)
{
    if (!_init_dirs())
        return false;

    orb_agents_start(context.njobs);

    if (context.target_proj) {
        struct orb_project * project = orb_ctx_get_project(context.target_proj);
        if (project) {
            orb_try(_build_project(project));
        } else
            orb_usrerr("project '%s' not found in monorepo", context.target_proj);
    } else {
        orb_usrerr("Use flag -p for give the name of the project. "
                   "Use flag -l to show list of projects");
    }

    orb_agent_stop();

    return true;
}
