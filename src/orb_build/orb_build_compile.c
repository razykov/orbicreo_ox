#define _GNU_SOURCE
#include <string.h>
#include <sys/wait.h>
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_utils/orb_utils_str.h"
#include "../orb_build/orb_build_utils.h"
#include "../orb_build/orb_build_compile.h"
#include "../orb_job_agent/orb_job_agent.h"

struct cmpl_payload {
    struct orb_project * project;

    const char * compile_fmt;
    const char * cfile_path;
};

static const char * _compiler(struct orb_project * project)
{
    const char * compiler;
    compiler  = orb_json_get_string(project->recipe.obj, "compiler_name");
    return compiler ? compiler : "cc";
}

static const char * _obj_file(struct cmpl_payload * pld)
{
    static __thread char buff[ORB_PATH_SZ];
    sprintf(buff, "%s.o", orb_sha2str(orb_file_sha1(pld->cfile_path)) );
    return buff;
}

static json_object * _comp_opt(struct orb_project * project)
{
    json_object * opts;
    opts = orb_json_find(project->recipe.obj, "compiler_options");
    return opts;
}

static char * _compiler_options(struct orb_project * project)
{
    size_t len = 0;
    char * flags;
    const char * comp_std;
    json_object * json;

    json = _comp_opt(project);
    if (!json) return NULL;

    flags = malloc(sizeof(u8));
    if (!flags) return NULL;

    flags[0] = '\0';

    for(size_t i = 0; i < json_object_array_length(json); ++i) {
        json_object * elem = json_object_array_get_idx(json, i);

        flags = orb_strexp(flags, &len, "-");
        flags = orb_strexp(flags, &len, json_object_get_string(elem));
        flags = orb_strexp(flags, &len, " ");
    }

    comp_std = orb_json_get_string(project->recipe.obj, "compiler_std");
    if (comp_std) {
        flags = orb_strexp(flags, &len, "-std=");
        flags = orb_strexp(flags, &len, comp_std);
    }

    return flags;
}

static void _dependency_include(struct orb_project * project,
                                char ** cmd, size_t * len)
{
    json_object * json = project->recipe.dependency_include;

    for(size_t i = 0; i < json_object_array_length(json); ++i) {
        json_object * elem = json_object_array_get_idx(json, i);
        const char * elem_str = json_object_get_string(elem);

        if (strlen(elem_str)) {
            *cmd = orb_strexp(*cmd, len, " -I");
            *cmd = orb_strexp(*cmd, len, elem_str);
        }
    }
}

static char * _file_compile_cmd_fmt(struct orb_project * project,
                                    const char * flags)
{
    char * cmd;
    size_t len = 0;

    cmd = calloc(1, sizeof(char));
    if (!cmd) return NULL;

    cmd = orb_strexp(cmd, &len, _compiler(project));
    cmd = orb_strexp(cmd, &len, " -c ");

    cmd = orb_strexp(cmd, &len, flags);
    _dependency_include(project, &cmd, &len);
    orb_monorepo_libs(&cmd, &len);

    cmd = orb_strexp(cmd, &len, " %s -o %s");

    return cmd;
}

static i32 _file_compile(void * payload)
{
    i32 res = 0;
    struct cmpl_payload * _payload = payload;
    const char * ofile_path;
    const char * ofile;

    if (!_payload)
        return -1;

    ofile_path = orb_cat(_payload->project->objs_path, _obj_file(_payload));
    ofile = ofile_path + context.rt_off;

    if (orb_file_exist(ofile_path)) {
        orb_stat(PPL, NULL, "  %s already exists", ofile);
        orb_json_string(_payload->project->files.o, NULL, ofile_path);
    } else {
        char * command = NULL;
        const char * cfile_name = _payload->cfile_path + context.rt_off;

        asprintf(&command, _payload->compile_fmt, _payload->cfile_path, ofile_path);
        if (!command) {
            orb_err("compilation command asprintf error");
            free(payload);
        }

        res = WEXITSTATUS(system(command));
        if (res == 0) {
            orb_json_string(_payload->project->files.o, NULL, ofile_path);
            orb_stat(PPL, NULL, "  %s ⟶ %s", cfile_name, ofile);
        } else
            orb_stat(RED, NULL, "  %s ⟶ X",  cfile_name);

        free(command);

        _payload->project->compile_turn = true;
    }

    free(payload);
    return res;
}

static struct cmpl_payload * _payload(struct orb_project * project,
                                      const char * cfile,
                                      const char * cmd_fmt)
{
    struct cmpl_payload * payload = malloc(sizeof(struct cmpl_payload));

    if (!payload) return NULL;

    payload->project = project;
    payload->cfile_path = cfile;
    payload->compile_fmt = cmd_fmt;

    return payload;
}

static bool _is_outdated(json_object * new, size_t off, const char * file)
{
    const char * file_new;

    for(size_t i = 0; i < json_object_array_length(new); ++i) {
        file_new = json_object_get_string(json_object_array_get_idx(new, i));
        file_new += off;

        if (!strcmp(file, file_new))
            return false;
    }

    return true;
}

static void _clear_old_o_files(struct orb_project * project)
{
    const char * file;
    const char * dirname = project->dirname;
    size_t off = context.rt_off + strlen("/build/obj/") + strlen(dirname);
    json_object * new = project->files.o;
    json_object * old = project->files.o_old;

    for(size_t i = 0; i < json_object_array_length(old); ++i) {
        file = json_object_get_string(json_object_array_get_idx(old, i));
        if (_is_outdated(new, off, file + off))
            orb_rmrf(file);
    }
}

bool orb_compile_project(struct orb_project * project)
{
    bool res;
    char * flags;
    char * cmd_fmt;
    json_object * comp_opt = _comp_opt(project);
    json_object * c_files = project->files.c;

    orb_try(comp_opt);

    orb_json_string(comp_opt, NULL, "fPIC");
    orb_json_string(project->recipe.dependency_include, NULL, "includes");

    flags = _compiler_options(project);
    cmd_fmt = _file_compile_cmd_fmt(project, flags);

    orb_inf("Сompilation");
    orb_stat(CYN, "Flags", "%s", flags);

    for(size_t i = 0; i < json_object_array_length(c_files); ++i) {
        const char * cfile;
        struct cmpl_payload * payload;

        cfile = json_object_get_string(json_object_array_get_idx(c_files, i));
        payload = _payload(project, cfile, cmd_fmt);

        orb_agent_task_append(_file_compile, payload);
    }

    res = orb_agent_wait();

    _clear_old_o_files(project);

    free(cmd_fmt);
    free(flags);
    return res;
}
