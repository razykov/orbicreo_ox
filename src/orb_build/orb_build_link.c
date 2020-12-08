#include <string.h>
#include <sys/wait.h>
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_args.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_utils/orb_utils_str.h"
#include "../orb_build/orb_build_link.h"
#include "../orb_build/orb_build_utils.h"


static const char * _shared(json_object * project) {
    return strcmp(orb_proj_type(project), "shared") ? " " : " -shared";
}

static const char * _linker(json_object * project) {
    const char * linker;
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    linker = orb_json_get_string(project, "linker_name");
    return linker ? linker : "cc";
}

static const char * _directory_dest(json_object * project) {
    const char * dist;
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    dist = orb_json_get_string(project, "directory_dest");
    return dist ? dist : "";
}

static const char * _dest(json_object * project) {
    static __thread char buff[ORB_PATH_SZ];
    sprintf(buff, "%s/bin/%s/", context->root, _directory_dest(project));
    return buff;
}

static char * _output_file(json_object * project, char * cmd, size_t * len) {
    const char * type = orb_proj_type(project);
    const char * name = orb_json_get_string(project, "project_name");

    cmd = orb_strexp(cmd, len, " -o ");

    cmd = orb_strexp(cmd, len, _dest(project));

    if (!strcmp(type, "shared"))
        cmd = orb_strexp(cmd, len, "lib");
    cmd = orb_strexp(cmd, len, name);
    if (!strcmp(type, "shared"))
        cmd = orb_strexp(cmd, len, ".so");

    return cmd;
}

static char * _ofiles(json_object * project, char * cmd, size_t * len) {
    json_object * ofiles = orb_json_find(project, "o_files");

    for(size_t i = 0; i < json_object_array_length(ofiles); ++i) {
        json_object * ofile = json_object_array_get_idx(ofiles, i);

        cmd = orb_strexp(cmd, len, " ");
        cmd = orb_strexp(cmd, len, json_object_get_string(ofile));
    }

    return cmd;
}

static char * _liblinks(json_object * project, char * cmd, size_t * len) {
    json_object * dep_list = orb_dependency_list(project);

    if(dep_list)
        for(size_t i = 0; i < json_object_array_length(dep_list); ++i) {
            json_object * dep = json_object_array_get_idx(dep_list, i);

            cmd = orb_strexp(cmd, len, " -l");
            cmd = orb_strexp(cmd, len, json_object_get_string(dep));
        }

    return cmd;
}

bool orb_link_project(json_object * project) {
    size_t len = 0;
    char * cmd = calloc(1, sizeof(char));

    cmd = orb_strexp(cmd, &len, _linker(project));
    cmd = orb_strexp(cmd, &len, _shared(project));
    orb_monorepo_libs(&cmd, &len);
    cmd = _ofiles(project, cmd, &len);
    cmd = _output_file(project, cmd, &len);
    cmd = _liblinks(project, cmd, &len);

    orb_inf(cmd);

    orb_mkdir_p(_dest(project));

    orb_try(WEXITSTATUS(system(cmd)) == 0);

    return true;
}
