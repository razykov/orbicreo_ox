#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_types/orb_context.h"
#include "../orb_utils/orb_utils_str.h"
#include "../orb_build/orb_build_link.h"
#include "../orb_build/orb_build_utils.h"

enum out_path {
    OPATH_FULL,
    OPATH_SHORT,
    OPATH_CLEAN
};

inline static const char * _shared(struct orb_project * project)
{
    return strcmp(project->type, "shared") ? " " : " -shared";
}

static const char * _linker(struct orb_project * project)
{
    const char * linker;
    linker = orb_json_get_string(project->recipe.obj, "linker_name");
    return linker ? linker : "cc";
}

static const char * _version_suffix(struct orb_project * project, bool full)
{
    static __thread char buff[32];
    buff[0] = '\0';
    if (!strcmp(project->type, "shared")) {
        i32 major = orb_json_get_int(project->version, "major");
        i32 minor = orb_json_get_int(project->version, "minor");
        i32 build = orb_json_get_int(project->version, "build");

        if (full)
            sprintf(buff, ".%d.%d.%d", major, minor, build);
        else
            sprintf(buff, ".%d", major);
    }
    return buff;
}

static char * _output_file(const char * file, char * cmd, size_t * len)
{
    cmd = orb_strexp(cmd, len, " -o ");
    cmd = orb_strexp(cmd, len, file);
    return cmd;
}

static char * _ofiles(struct orb_project * project, char * cmd, size_t * len)
{
    json_object * ofiles = project->files.o;

    for(size_t i = 0; i < json_object_array_length(ofiles); ++i) {
        json_object * ofile = json_object_array_get_idx(ofiles, i);

        cmd = orb_strexp(cmd, len, " ");
        cmd = orb_strexp(cmd, len, json_object_get_string(ofile));
    }

    return cmd;
}

static char * _liblinks(struct orb_project * project, char * cmd, size_t * len)
{
    json_object * dep_list = project->recipe.dependency_list;

    if(dep_list)
        for(size_t i = 0; i < json_object_array_length(dep_list); ++i) {
            json_object * dep = json_object_array_get_idx(dep_list, i);

            cmd = orb_strexp(cmd, len, " -l");
            cmd = orb_strexp(cmd, len, json_object_get_string(dep));
        }

    return cmd;
}

static const char * _liblist(struct orb_project * project)
{
    static __thread char list[B_KB(4)];
    u32 off = 0;
    json_object * dep_list = project->recipe.dependency_list;

    list[0] = '\0';

    if(dep_list) {
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
    }
    if (off != 0)
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

static const char * _restrict_path(const char * path, u32 max_len) {
    const char * ptr;
    static __thread char buff[B_KB(4)];
    size_t pathlen = strlen(path);

    if (pathlen > max_len) {
        snprintf(buff, max_len, "..%s", path + pathlen - max_len);
        ptr = buff;
    } else
        ptr = path;
    return ptr;
}

static void _ofiles_print(struct orb_project * project)
{
    // repeat "─" fifty times
    const u32 max_line_len = 50;
    const char * line = "──────────────────────────────────────────────────";

    bool first = true;
    u32 root_off;
    size_t ofp_len;
    const char * output_file_path;
    json_object * ofiles = project->files.o;

    root_off = strlen(context.root) + sizeof(char);
    output_file_path = project->recipe.output_file + context.rt_off;
    output_file_path = _restrict_path(output_file_path, max_line_len);
    ofp_len = utf8_strlen(output_file_path) * strlen("─");

    for(size_t i = 0; i < json_object_array_length(ofiles); ++i) {
        json_object * ofile = json_object_array_get_idx(ofiles, i);

        orb_stat(PPL, NULL, "%s %s",
                      first ? "  ┌" : "  ├",
                      json_object_get_string(ofile) + root_off);
        first = false;
    }
    orb_stat(PPL, NULL, "  │  ┌─%.*s─┐", ofp_len, line);
    orb_stat(PPL, NULL, "  └──┤ %s │",   output_file_path);
    orb_stat(PPL, NULL, "     └─%.*s─┘", ofp_len, line);
}

static char * _output_file_path(struct orb_project * project, enum out_path op)
{
    char * buff;
    const char * basic = project->recipe.output_file;

    switch (op) {
    case OPATH_FULL:
        asprintf(&buff, "%s%s", basic, _version_suffix(project, true));
        break;
    case OPATH_SHORT:
        asprintf(&buff, "%s%s", basic, _version_suffix(project, false));
        break;
    default:
        asprintf(&buff, "%s", basic);
    }
    return buff;
}

static void _bin_clear(struct orb_project * project)
{
    char buff[ORB_PATH_SZ];
    const char * dirpath = project->recipe.bin_file_dir;
    DIR * d = opendir(dirpath);
    if (d) {
        struct dirent * dir;
        while ((dir = readdir(d)) != NULL) {
            sprintf(buff, "%s/%s", dirpath, dir->d_name);
            if (strstr(dir->d_name, project->name))
                orb_rmrf(buff);
        }
        closedir(d);
    }
}

static bool _lib_symlink(const char * file, const char * link)
{
    orb_stat(PPL, "", "  symlink %s ⟶ %s",
                         file + context.rt_off, link + context.rt_off);
    return symlink(file, link) == 0;
}

bool orb_link_project(struct orb_project * project)
{
    bool res;
    char * cmd;
    size_t len = 0;

    char * bin_full  = _output_file_path(project, OPATH_FULL);

    orb_inf("Linking");
    if (json_object_array_length(project->recipe.dependency_list) != 0)
        orb_stat(CYN, "Linkable libraries", "%s", _liblist(project));

    if (!project->compile_turn && orb_file_exist(bin_full)) {
        orb_stat(PPL, NULL, "  .%s already exist", bin_full + context.rt_off);
        free(bin_full);
        return true;
    }

    cmd = calloc(1, sizeof(char));
    cmd = orb_strexp(cmd, &len, _linker(project));
    cmd = orb_strexp(cmd, &len, _shared(project));
    cmd = orb_monorepo_libs(cmd, &len);
    cmd = _ofiles(project, cmd, &len);
    cmd = _output_file(bin_full, cmd, &len);
    cmd = _liblinks(project, cmd, &len);

    orb_mkdir_p(project->recipe.bin_file_dir);
    _bin_clear(project);

    res = (WEXITSTATUS(system(cmd)) == 0);
    if (res) {
        _ofiles_print(project);

        if (!strcmp(project->type, "shared")) {
            char * bin_short = _output_file_path(project, OPATH_SHORT);
            _lib_symlink(bin_full, bin_short);
            _lib_symlink(bin_full, project->recipe.output_file);
            free(bin_short);
        }
    }

    free(cmd);
    free(bin_full);
    return res;
}
