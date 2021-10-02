#include "orb_goal_list.h"
#include "../orb_utils/orb_log.h"
#include "../orb_build/orb_build_utils.h"
#include "../orb_types/orb_context.h"


static void _print_projects_list(void)
{
    struct orb_project * list = context.projects;

    while (list) {
        orb_txt("    %s", list->name);
        list = list->next;
    }
}

void orb_goal_list(void)
{
    if (context.projects) _print_projects_list();
    else orb_err("error while read projects recipes");
}
