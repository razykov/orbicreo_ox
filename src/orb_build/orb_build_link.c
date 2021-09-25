#include <string.h>
#include <sys/wait.h>
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_args.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_utils/orb_utils_str.h"
#include "../orb_build/orb_build_link.h"
#include "../orb_build/orb_build_utils.h"


inline static const char * _shared(json_object * project)
{
    return strcmp(orb_proj_type(project), "shared") ? " " : " -shared";
}

static const char * _linker(json_object * project)
{
    const char * linker;
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    linker = orb_json_get_string(project, "linker_name");
    return linker ? linker : "cc";
}

static const char * _directory_dest(json_object * project)
{
    const char * dist;
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    dist = orb_json_get_string(project, "directory_dest");
    return dist ? dist : "";
}

static const char * _dest(json_object * project)
{
    static __thread char buff[ORB_PATH_SZ];

    sprintf(buff, "%s/bin/%s", context->root, _directory_dest(project));
    if (buff[strlen(buff) - 1] != '/')
        buff[strlen(buff)] = '/';

    return buff;
}

static const char * _output_file_path(json_object * project)
{
    static __thread char buff[ORB_ROOT_SZ];
    const char * type = orb_proj_type(project);
    const char * name = orb_json_get_string(project, "project_name");

    if (!strcmp(type, "shared"))
        sprintf(buff, "%slib%s.so", _dest(project), name);
    else
        sprintf(buff, "%s%s", _dest(project), name);

    return buff;
}

static char * _output_file(json_object * project, char * cmd, size_t * len)
{
    cmd = orb_strexp(cmd, len, " -o ");
    cmd = orb_strexp(cmd, len, _output_file_path(project));
    return cmd;
}

static char * _ofiles(json_object * project, char * cmd, size_t * len)
{
    json_object * ofiles = orb_json_find(project, "o_files");

    for(size_t i = 0; i < json_object_array_length(ofiles); ++i) {
        json_object * ofile = json_object_array_get_idx(ofiles, i);

        cmd = orb_strexp(cmd, len, " ");
        cmd = orb_strexp(cmd, len, json_object_get_string(ofile));
    }

    return cmd;
}

static char * _liblinks(json_object * project, char * cmd, size_t * len)
{
    json_object * dep_list = orb_dependency_list(project);

    if(dep_list)
        for(size_t i = 0; i < json_object_array_length(dep_list); ++i) {
            json_object * dep = json_object_array_get_idx(dep_list, i);

            cmd = orb_strexp(cmd, len, " -l");
            cmd = orb_strexp(cmd, len, json_object_get_string(dep));
        }

    return cmd;
}

static const char * _liblist(json_object * project)
{
    static __thread char list[B_KB(4)];
    u32 off = 0;
    json_object * dep_list = orb_dependency_list(project);

    list[0] = '\0';

    if(dep_list)
        for(size_t i = 0; i < json_object_array_length(dep_list); ++i) {
            json_object * dep = json_object_array_get_idx(dep_list, i);
            const char * lib = json_object_get_string(dep);
            size_t len = strlen(lib);

            if (off + len + 1 > B_KB(4)) {
                list[off] = '\0';
                return list;
            }

            memcpy(list + off, lib, len);
            list[off + len] = ' ';
            off += len + 1;
        }
    list[off - 1] = '\0';

    return list;
}

static size_t utf8_strlen(const char *s)
{
    size_t count = 0;
    while (*s)
        count += (*s++ & 0xC0) != 0x80;
    return count;
}

static void _ofiles_print(json_object * project)
{
    bool first = true;
    json_object * ofiles = orb_json_find(project, "o_files");
    u32 root_off = strlen(orb_json_get_string(project, "repo_root")) + sizeof(char);
    const char * output_file_path = _output_file_path(project) + root_off;
    size_t ofp_len = utf8_strlen(output_file_path);

    for(size_t i = 0; i < json_object_array_length(ofiles); ++i) {
        json_object * ofile = json_object_array_get_idx(ofiles, i);

        orb_stat(PPL, NULL, "%s %s",
                      first ? "  ┌" : "  ├",
                      json_object_get_string(ofile) + root_off);
        first = false;
    }
    orb_stat(PPL, NULL, "  │  ┌─%*s─┐", ofp_len);
    orb_stat(PPL, NULL, "  └──┤ %s │", output_file_path);
    orb_stat(PPL, NULL, "     └─%*s─┘", ofp_len);
}

bool orb_link_project(json_object * project)
{
    bool res;
    size_t len = 0;
    char * cmd = calloc(1, sizeof(char));

    orb_inf("Linking");
    orb_stat(CYN, "Linkable libraries", "%s", _liblist(project));

    cmd = orb_strexp(cmd, &len, _linker(project));
    cmd = orb_strexp(cmd, &len, _shared(project));
    orb_monorepo_libs(&cmd, &len);
    cmd = _ofiles(project, cmd, &len);
    cmd = _output_file(project, cmd, &len);
    cmd = _liblinks(project, cmd, &len);

    orb_mkdir_p(_dest(project));
    res = WEXITSTATUS(system(cmd)) == 0;

    if (res)
        _ofiles_print(project);

    free(cmd);
    return res;
}
