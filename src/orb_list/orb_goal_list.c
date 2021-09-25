#include "orb_goal_list.h"
#include "../orb_utils/orb_log.h"
#include "../orb_build/orb_build_utils.h"

static void _print_projects_list(json_object * set)
{
    json_object_object_foreach(set, key, val) {
        orb_txt("    %s", orb_json_get_string(val, "project_name"));
        ((void)key);
    }
}

void orb_goal_list(void)
{
    json_object * set = orb_projects_set();

    if (set) _print_projects_list(set);
    else orb_err("error while read projects recipes");
}
