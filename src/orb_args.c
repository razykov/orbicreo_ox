#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "../src/orb_args.h"

struct orb_ctx context;

static void _name_read(char ** argv) {
    char * ptr = argv[0];
    char * name = ptr;

    do {
        if (*ptr == '/')
            name = ++ptr;
    } while(*(ptr++)) ;

    strncpy(context.appname, name, ORB_APPNAME_SZ);
}

static void _get_root(struct orb_ctx * ctx) {
    if(!strlen(ctx->root))
        getcwd(ctx->root, sizeof(ctx->root));

    snprintf(ctx->proj_path, ORB_PATH_SZ, "%s/projects", ctx->root);
}

bool orb_args_parse(i32 argc, char ** argv) {
    i32 c;

    memset(&context, 0, sizeof(struct orb_ctx));

    _name_read(argv);
    _get_root(&context);

    while (true) {
        i32 option_index = 0;
        static struct option long_options[] = {
            { "help",  no_argument, 0, 'h' },
            { "init",  no_argument, 0, 'i' },
            { "build", no_argument, 0, 'b' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "bhi", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
        case 0:
            if (long_options[option_index].flag != 0) break;
            break;

        case 'h':
            context.goal = ORB_GOAL_HELP;
            break;

        case 'i':
            context.goal = ORB_GOAL_INIT;
            break;

        case 'b':
            context.goal = ORB_GOAL_BUILD;
            break;

        case '?':
            return false;
            break;

        default:
            abort();
        }
    }

    return true;
}
