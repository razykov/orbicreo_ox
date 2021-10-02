#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "orb_json.h"
#include "orb_context.h"
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_args.h"
#include "../orb_utils/orb_utils.h"

#define DEFAULT_JOBS_COUNT (1)

struct orb_ctx context;

static bool _name_read(char ** argv)
{
    char * ptr = argv[0];
    char * name = ptr;

    do {
        if (*ptr == '/')
            name = ++ptr;
    } while(*(ptr++)) ;

    context.appname = strdup(name);

    return context.appname != NULL;
}

static void _project_append(struct orb_project * project)
{
    project->next = context.projects;
    context.projects = project;
}

static bool _project_add(const char * dname)
{
    struct orb_project * project = orb_proj_create(dname);
    if (project)
        _project_append(project);
    return project != NULL;
}

static bool _projects_read(void)
{
    DIR * d;
    struct dirent * dir;

    d = opendir(context.repo_projects);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (!orb_is_include_dir(dir)) continue;
            _project_add(dir->d_name);
        }
        closedir(d);
    }

    return true;
}

static bool _get_root(void)
{
    i32 r;
    char buf[PATH_MAX];

    getcwd(buf, PATH_MAX);
    context.root = strdup(buf);
    if (!context.root) {
        orb_err("root path error");
        return false;
    }
    context.rt_off = strlen(context.root);

    r = asprintf(&context.repo_projects, "%s/projects", context.root);
    if (r <= 0) {
        orb_err("repo_projects error");
        return false;
    }

    return true;
}

bool orb_ctx_init(i32 argc, char ** argv)
{
    memset(&context, 0, sizeof(context));

    context.goal = ORB_GOAL_BUILD;
    context.target_proj = NULL;

    context.projects = NULL;

    context.njobs = DEFAULT_JOBS_COUNT;

    context.verbose    = false;
    context.clear      = false;
    context.clear_deps = false;
    context.release    = false;

    orb_try(_name_read(argv));
    orb_try(_get_root());

    orb_try(orb_args_parse(argc, argv));

    orb_try(_projects_read());

    return true;
}

void orb_ctx_destroy(void)
{
    safe_free(context.appname);
    safe_free(context.root);
    safe_free(context.repo_projects);
    safe_free(context.target_proj);

    while (context.projects) {
        struct orb_project * proj = context.projects;
        context.projects = proj->next;
        orb_project_free(proj);
    }
}

struct orb_project * orb_ctx_get_project(const char * name)
{
    struct orb_project * proj;

    if (!name)
        return NULL;

    proj = context.projects;
    while (proj) {
        if (!strcmp(proj->name, name))
            return proj;
        proj = proj->next;
    }
    return NULL;
}
