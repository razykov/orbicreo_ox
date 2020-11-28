#include <stdio.h>
#include <string.h>
#include "../src/orb_log.h"
#include "../src/orb_init.h"
#include "../src/orb_utils.h"
#include "../src/orb_args.h"

#define SHARE_DIR "/usr/share/orbicreo/"
#define EMPTY_PRJ "projects/empty_app/"

bool orb_init(void) {
    const char * root = context.root;

    orb_inf("orbicreo init");

    if (!orb_dir_exist(root))
        return false;

    orb_mkdir_p(orb_cat(root, "includes"));
    orb_mkdir_p(orb_cat(root, "docs"));
    orb_mkdir_p(orb_cat(root, "run"));
    orb_mkdir_p(orb_cat(root, EMPTY_PRJ));

    orb_copy(SHARE_DIR"main.c",       orb_cat(root, EMPTY_PRJ"main.c"));
    orb_copy(SHARE_DIR"recipe.json",  orb_cat(root, EMPTY_PRJ"recipe.json"));
    orb_copy(SHARE_DIR"version.json", orb_cat(root, EMPTY_PRJ"version.json"));

    return true;
}
