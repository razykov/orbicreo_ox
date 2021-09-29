#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "orb_args.h"

struct orb_ctx * context;

bool orb_args_parse(i32 argc, char ** argv)
{
    i32 c;
    i32 iarg;

    if (!(context = orb_ctx_create(argv)))
        return false;

    while (true) {
        i32 option_index = 0;
        static struct option long_options[] = {
            { "help",       no_argument,       NULL, 'h' },
            { "init",       no_argument,       NULL, 'i' },
            { "build",      no_argument,       NULL, 'b' },
            { "list",       no_argument,       NULL, 'l' },
            { "clear",      no_argument,       NULL, 'c' },
            { "clear_deps", no_argument,       NULL, 'C' },
            { "project",    optional_argument, NULL, 'p' },
            { "jobs",       optional_argument, NULL, 'j' },
            { "verbose",    no_argument,       NULL, 'v' },
            { 0, 0, NULL, 0 }
        };

        c = getopt_long(argc, argv, "bhilcCp:j:v", long_options, &option_index);
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

        case 'c':
            context->clear = true;
            break;

        case 'C':
            context->clear_deps = true;
            break;

        case 'l':
            context->goal = ORB_GOAL_LIST;
            break;

        case 'p':
            context->target_proj = strdup(optarg);
            break;

        case 'j':
            iarg = atoi(optarg);
            context->njobs = iarg < 1 ? 1 : iarg > 255 ? 255 : iarg;
            break;

        case 'v':
            context->verbose = true;
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
