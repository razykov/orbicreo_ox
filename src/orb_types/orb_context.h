#ifndef ORB_CONTEXT_H
#define ORB_CONTEXT_H

#include "../orb_types/orb_project.h"

enum orb_goal {
    ORB_GOAL_BUILD = 0,
    ORB_GOAL_HELP,
    ORB_GOAL_INIT,
    ORB_GOAL_LIST
};

struct orb_ctx {
    char * appname;
    char * root;
    char * repo_projects;
    char * target_proj;

    size_t rt_off;

    json_object * proj_set_json;

    struct orb_project * projects;

    enum orb_goal goal;

    bool verbose;
    bool clear;
    bool clear_deps;
    bool release;

    u8 njobs;
};

extern struct orb_ctx context;

bool orb_ctx_init(i32 argc, char ** argv);
void orb_ctx_destroy(void);

struct orb_project * orb_ctx_get_project(const char * name);

#endif /* ORB_CONTEXT_H */
