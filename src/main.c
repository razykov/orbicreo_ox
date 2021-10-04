#include <stdlib.h>
#include "orb_utils/orb_log.h"
#include "orb_utils/orb_utils.h"
#include "orb_types/orb_context.h"
#include "orb_init/orb_goal_init.h"
#include "orb_list/orb_goal_list.h"
#include "orb_build/orb_goal_build.h"


static void _help(void)
{
    orb_txt("Using: %s [opt]... ", context.appname);
    orb_txt("");
    orb_txt("Options:");
    orb_txt("    -h, --help           Print this message");
    orb_txt("    -i, --init           Initialization orbicreo monorepo "
                                     "in the current directory");
    orb_txt("    -b, --build          Build projects in the current directory");
    orb_txt("    -l, --list           List of projects found in monorepo");
    orb_txt("    -c, --clear          Clear project");
    orb_txt("    -C, --clear_deps     Clear project dependencies");
    orb_txt("    -p, --project [name] Project name to build");
    orb_txt("    -j, --jobs [count]   Number of build threads");
    orb_txt("    -r, --release        Release build");
    orb_txt("    -v, --verbose        Verbose outputs");
    orb_txt("        --version        Print version info and exit");
    orb_txt("");
    orb_txt("Bug report email <v.razykov@gmail.com>");

    exit(EXIT_SUCCESS);
}

static void _version(void)
{
    orb_txt("Orbicreo "VERSION_MAJOR"."
                       VERSION_MINOR"."
                       VERSION_BUILD, context.appname);
    orb_txt("License GPLv3+: GNU GPL version 3 or later "
            "<http://gnu.org/licenses/gpl.html>");
    orb_txt("This is free software:"
            "you are free to change and redistribute it.");
    orb_txt("There is NO WARRANTY, to the extent permitted by law.");

    exit(EXIT_SUCCESS);
}

static void _exec_goal()
{
    switch (context.goal) {
    case ORB_GOAL_BUILD: orb_goal_build(); break;
    case ORB_GOAL_INIT : orb_goal_init();  break;
    case ORB_GOAL_LIST : orb_goal_list();  break;
    case ORB_GOAL_HELP : _help();          break;
    case ORB_GOAL_VERS : _version();       break;
    default:
        orb_err("Fatal error. Unknown goal");
        exit(EXIT_FAILURE);
    }
}

i32 main (i32 argc, char ** argv)
{
    if (!orb_ctx_init(argc, argv))
        return EXIT_FAILURE;

    _exec_goal();

    orb_ctx_destroy();

    return EXIT_SUCCESS;
}
