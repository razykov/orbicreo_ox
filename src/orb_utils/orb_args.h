#ifndef ORB_ARGS_H
#define ORB_ARGS_H

#include "../orb_types/orb_types.h"

extern struct orb_ctx * context;

bool orb_args_parse(i32 argc, char ** argv);

#endif /* ORB_ARGS_H */