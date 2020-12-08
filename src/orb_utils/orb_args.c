#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "orb_args.h"

struct orb_ctx * context;

bool orb_args_parse(i32 argc, char ** argv) {
    i32 c;

    if (!(context = orb_ctx_create(argv)))
        return false;

    while (true) {
        i32 option_index = 0;
        static struct option long_options[] = {
            { "help",    no_argument,       0, 'h' },
            { "init",    no_argument,       0, 'i' },
            { "build",   no_argument,       0, 'b' },
            { "list",    no_argument,       0, 'l' },
            { "project", optional_argument, 0, 'p' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "bhilp:", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
        case 0:
            if (long_options[option_index].flag != 0) break;
            break;

        case 'h':
            context->goal = ORB_GOAL_HELP;
            break;

        case 'i':
            context->goal = ORB_GOAL_INIT;
            break;

        case 'b':
            context->goal = ORB_GOAL_BUILD;
            break;

        case 'l':
            context->goal = ORB_GOAL_LIST;
            break;

        case 'p':
            context->target_proj = strdup(optarg);
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
