#include <stdlib.h>
#include "../src/orb_log.h"
#include "../src/orb_args.h"
#include "../src/orb_utils.h"
#include "../src/orb_build.h"
#include "../src/orb_init.h"


static void _help(void) {
    orb_txt("Using: %s [opt]... ", context.appname);
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
    switch (context.goal) {
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

//    u8 sha[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
//    orb_inf("%s", orb_sha2str(orb_file_sha1("/home/razykov/orbi_test/projects/empty/func/sum.c")));
//    orb_inf("%s", orb_sha2str(sha));
//    orb_inf("%s", orb_get_dir("/home/razykov/atol5_sum.png"));
//    orb_inf("%s", orb_dir_exist("/home/razykov/atol5_sum.png") ? "true" : "false");
//    orb_mkdir_p("/tmp/test/abc/def");
//    json_object * json = json_object_from_file("/home/razykov/orbi_test/projects/empty/version.json");
//    orb_txt("%s",
//            json_object_to_json_string_ext(json, JSON_C_TO_STRING_SPACED |
//                                                 JSON_C_TO_STRING_PRETTY));

    _exec_goal();
//    ((void)_exec_goal);

    return EXIT_SUCCESS;
}
