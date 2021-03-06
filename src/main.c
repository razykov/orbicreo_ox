#include <stdlib.h>
#include "../src/orb_log.h"
#include "../src/orb_args.h"
#include "../src/orb_utils.h"
#include "../src/orb_build.h"
#include "../src/orb_init.h"
#include "../src/orb_build_utils.h"


static void _help(void) {
    orb_txt("Using: %s [opt]... ", context->appname);
    orb_txt("");
    orb_txt("Options:");
    orb_txt("    -h, --help     Print this message");
    orb_txt("    -i, --init     Initialization orbicreo project in the current directory");
    orb_txt("    -b, --build    Build projects in the current directory");
    orb_txt("");
    orb_txt("Bug report email <v.razykov@gmail.com>");

    exit(EXIT_SUCCESS);
}

static void _exec_goal() {
    switch (context->goal) {
    case ORB_GOAL_BUILD: orb_build(); break;
    case ORB_GOAL_INIT : orb_init();  break;
    case ORB_GOAL_HELP : _help();     break;
    default:
        orb_err("Fatal error. Unknown goal");
        exit(EXIT_FAILURE);
    }
}

i32 main (i32 argc, char ** argv) {
    if (!orb_args_parse(argc, argv))
        return EXIT_FAILURE;

    size_t len = 0;
    char * str = calloc(1, 1);
    orb_monorepo_libs(&str, &len);
    orb_txt("%s", str);

    _exec_goal();
//    ((void)_exec_goal);

    orb_ctx_destroy(context);

    return EXIT_SUCCESS;
}
