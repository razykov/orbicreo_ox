#include <string.h>
#include <sys/wait.h>
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_utils/orb_utils_str.h"
#include "../orb_build/orb_build_utils.h"
#include "../orb_build/orb_build_compile.h"
#include "../orb_job_agent/orb_job_agent.h"

struct cmpl_payload {
    json_object * project;
    size_t root_offset;

    const char * flags_str;

    const char * cfile_path;
    const char * cfile_name;
};

static const char * _compiler(json_object * project) {
    const char * compiler;
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    compiler  = orb_json_get_string(project, "compiler_name");
    return compiler ? compiler : "cc";
}

static const char * _obj_file(struct cmpl_payload * pld) {
    static __thread char buff[ORB_PATH_SZ];
    sprintf(buff, "%s.o", orb_sha2str(orb_file_sha1(pld->cfile_path)) );
    return buff;
}

static json_object * _comp_opt(json_object * project) {
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    project = orb_json_find(project, "compiler_options");
    return project;
}

static const char * _comp_std(json_object * project) {
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    return orb_json_get_string(project, "compiler_std");
}

static json_object * _dep_incl(json_object * project) {
    json_object * gen;
    project = orb_json_find(project, "recipe");
    gen = project = orb_json_find(project, "general");
    project = orb_json_find(project, "dependency_include");
    return project ? project : orb_json_array(gen, "dependency_include");
}

static char * _compiler_options(json_object * project) {
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

    comp_std = _comp_std(project);
    if (comp_std) {
        flags = orb_strexp(flags, &len, "-std=");
        flags = orb_strexp(flags, &len, comp_std);
    }

    return flags;
}

static void _dependency_include(json_object * project, char ** cmd, size_t * len) {
    json_object * gnrl;
    json_object * json = orb_json_find(project, "recipe");

    gnrl = json = orb_json_find(json, "general");
    json = orb_json_find(json, "dependency_include");

    if (!json)
        json = orb_json_array(gnrl, "dependency_include");

    for(size_t i = 0; i < json_object_array_length(json); ++i) {
        json_object * elem = json_object_array_get_idx(json, i);
        const char * elem_str = json_object_get_string(elem);

        if (strlen(elem_str)) {
            *cmd = orb_strexp(*cmd, len, " -I");
            *cmd = orb_strexp(*cmd, len, elem_str);
        }
    }
}

static char * _file_compile_command(struct cmpl_payload * pld,
                                    const char * ofile_path) {
    char * cmd;
    size_t len = 0;

    cmd = calloc(1, sizeof(char));
    if (!cmd) return NULL;

    cmd = orb_strexp(cmd, &len, _compiler(pld->project));
    cmd = orb_strexp(cmd, &len, " -c ");

    cmd = orb_strexp(cmd, &len, pld->flags_str);
    _dependency_include(pld->project, &cmd, &len);
    orb_monorepo_libs(&cmd, &len);
    cmd = orb_strexp(cmd, &len, " ");

    cmd = orb_strexp(cmd, &len, pld->cfile_path);
    cmd = orb_strexp(cmd, &len, " -o ");
    cmd = orb_strexp(cmd, &len, ofile_path);

    orb_json_string(orb_json_find(pld->project, "o_files"), NULL, ofile_path);

    return cmd;
}

static i32 _file_compile(void * payload) {
    i32 res;
    char * command;
    struct cmpl_payload * _payload = payload;
    const char * ofile_path;

    if (!_payload)
        return -1;

    ofile_path = orb_cat(orb_json_get_string(_payload->project, "objs_path"),
                         _obj_file(_payload));

    command = _file_compile_command(_payload, ofile_path);
    if (!command)
        return -1;

    res = WEXITSTATUS(system(command));
    if (res == 0) {
        const char * ofile = ofile_path + _payload->root_offset;
        orb_stat(PPL, NULL, "  %s ⟶ %s", _payload->cfile_name, ofile);
    } else
        orb_stat(RED, NULL, "  %s ⟶ X",  _payload->cfile_name);

    free(command);
    free(payload);
    return res;
}

static struct cmpl_payload * _payload(json_object * project,
                                      const char * repo_root,
                                      const char * cfile,
                                      char * flags) {
    struct cmpl_payload * payload = malloc(sizeof(struct cmpl_payload));

    if (!payload) return NULL;

    payload->project = project;
    payload->root_offset = strlen(repo_root) + sizeof(char);
    payload->cfile_path = cfile;
    payload->cfile_name = cfile + payload->root_offset;
    payload->flags_str = flags;

    return payload;
}

bool orb_compile_project(json_object * project) {
    bool res;
    char * flags;
    json_object * comp_opt = _comp_opt(project);
    json_object * c_files = orb_json_find(project, "c_files");
    const char * repo_root = orb_json_get_string(project, "repo_root");

    orb_try(comp_opt);

    orb_json_array(project, "o_files");
    orb_json_string(comp_opt, NULL, "fPIC");
    orb_json_string(_dep_incl(project), NULL, "includes");

    flags = _compiler_options(project);

    orb_inf("Сompilation");
    orb_stat(CYN, "Flags", "%s", flags);

    for(size_t i = 0; i < json_object_array_length(c_files); ++i) {
        const char * cfile;
        struct cmpl_payload * pld;

        cfile = json_object_get_string(json_object_array_get_idx(c_files, i));
        pld = _payload(project, repo_root, cfile, flags);

        orb_agent_task_append(_file_compile, pld);
    }

    res = orb_agent_wait();

    free(flags);
    return res;
}
