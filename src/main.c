#include <stdlib.h>
#include "orb_utils/orb_log.h"
#include "orb_utils/orb_args.h"
#include "orb_utils/orb_utils.h"
#include "orb_init/orb_goal_init.h"
#include "orb_list/orb_goal_list.h"
#include "orb_build/orb_goal_build.h"
#include "orb_build/orb_build_utils.h"
#include "orb_job_agent/orb_job_agent.h"


static void _help(void)
{
    orb_txt("Using: %s [opt]... ", context->appname);
    orb_txt("");
    orb_txt("Options:");
    orb_txt("    -h, --help           Print this message");
    orb_txt("    -i, --init           Initialization orbicreo monorepo in the current directory");
    orb_txt("    -b, --build          Build projects in the current directory");
    orb_txt("    -l, --list           List of projects found in monorepo");
    orb_txt("    -p, --project [name] Project name to build");
    orb_txt("    -j, --jobs [count]   Number of build threads");
    orb_txt("");
    orb_txt("Bug report email <v.razykov@gmail.com>");

    exit(EXIT_SUCCESS);
}

static void _exec_goal()
{
    switch (context->goal) {
    case ORB_GOAL_BUILD: orb_goal_build(); break;
    case ORB_GOAL_INIT : orb_goal_init();  break;
    case ORB_GOAL_LIST : orb_goal_list();  break;
    case ORB_GOAL_HELP : _help();          break;
    default:
        orb_err("Fatal error. Unknown goal");
        exit(EXIT_FAILURE);
    }
}

i32 main (i32 argc, char ** argv)
{
    if (!orb_args_parse(argc, argv))
        return EXIT_FAILURE;

    orb_agents_start(context->njobs);
    _exec_goal();

    orb_agent_stop();
    orb_ctx_destroy(context);

    (void)argc;
    (void)argv;

    return EXIT_SUCCESS;
}
