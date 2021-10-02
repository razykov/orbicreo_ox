#ifndef ORB_PROJECT_H
#define ORB_PROJECT_H

#include "../orb_types/orb_types.h"

struct orb_project {
    const char * name;
    const char * type;
    char * root;
    char * dirname;
    char * meta_path;
    char * recipe_path;
    char * version_path;
    char * objs_path;

    json_object * meta;
    json_object * version;

    struct recipe {
        json_object * obj;

        json_object * exp_incl;
        json_object * dependency_list;
        json_object * dependency_include;

        char * output_file;
        char * bin_file_dir;
    } recipe;

    struct files {
        json_object * h;
        json_object * c;
        json_object * o;
        json_object * o_old;
    } files;

    bool built;
    bool compile_turn;

    struct orb_project * next;
};

struct orb_project * orb_proj_create(const char * dname);

void orb_project_free(struct orb_project * project);

#endif /* ORB_PROJECT_H */
