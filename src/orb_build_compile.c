#include <sys/wait.h>
#include "../src/orb_log.h"
#include "../src/orb_utils.h"
#include "../src/orb_threads.h"
#include "../src/orb_utils_str.h"
#include "../src/orb_build_utils.h"
#include "../src/orb_build_compile.h"

struct cmpl_payload {
    bool res;

    json_object * project;
    const char * cfile;
};

static const char * _compiler(json_object * project) {
    const char * compiler;
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    compiler  = orb_json_get_string(project, "compiler_name");
    return compiler ? compiler : "cc";
}

static const char * _obj_path(struct cmpl_payload * pld) {
    static __thread char buff[ORB_PATH_SZ];
    sprintf(buff, "%s/%s.o", orb_json_get_string(pld->project, "objs_path"),
                             orb_sha2str(orb_file_sha1(pld->cfile)) );
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

static void _compiler_options(json_object * project, char ** cmd, size_t * len) {
    const char * comp_std;
    json_object * json = _comp_opt(project);
    if (!json) return;

    for(size_t i = 0; i < json_object_array_length(json); ++i) {
        json_object * elem = json_object_array_get_idx(json, i);

        *cmd = orb_strexp(*cmd, len, "-");
        *cmd = orb_strexp(*cmd, len, json_object_get_string(elem));
        *cmd = orb_strexp(*cmd, len, " ");
    }

    comp_std = _comp_std(project);
    if (comp_std) {
        *cmd = orb_strexp(*cmd, len, "-std=");
        *cmd = orb_strexp(*cmd, len, comp_std);
    }
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

        *cmd = orb_strexp(*cmd, len, " -I");
        *cmd = orb_strexp(*cmd, len, json_object_get_string(elem));
    }
}

static char * _file_compile_command(struct cmpl_payload * pld) {
    size_t len = 0;
    const char * ofile;
    char * cmd = calloc(1, sizeof(char));

    if (!cmd) return NULL;

    ofile = _obj_path(pld);

    cmd = orb_strexp(cmd, &len, _compiler(pld->project));
    cmd = orb_strexp(cmd, &len, " -c ");

    _compiler_options  (pld->project, &cmd, &len);
    _dependency_include(pld->project, &cmd, &len);
    orb_monorepo_libs(&cmd, &len);
    cmd = orb_strexp(cmd, &len, " ");

    cmd = orb_strexp(cmd, &len, pld->cfile);
    cmd = orb_strexp(cmd, &len, " -o ");
    cmd = orb_strexp(cmd, &len, ofile);

    orb_json_string(orb_json_find(pld->project, "o_files"), NULL, ofile);
    orb_inf("%s", cmd);

    return cmd;
}

static void * _file_compile(void * payload) {
    struct cmpl_payload * _payload = payload;
    char * command = _file_compile_command(_payload);

    if (!command) {
        _payload->res = false;
        return _payload;
    }

    _payload->res = WEXITSTATUS(system(command)) == 0;

    free(command);
    return _payload;
}

static struct cmpl_payload * _payload(json_object * project, const char * cfile) {
    struct cmpl_payload * payload = malloc(sizeof(struct cmpl_payload));

    if (!payload) return NULL;

    payload->res = false;
    payload->project = project;
    payload->cfile = cfile;

    return payload;
}

static bool _compile_ret_parse(void * ret) {
    bool res;
    struct cmpl_payload * payload = ret;

    res = payload->res;
    free(payload);

    return res;
}

bool orb_compile_project(json_object * project) {
    orb_thrds_t thrds = orb_thrd_create();
    json_object * comp_opt = _comp_opt(project);
    json_object * c_files = orb_json_find(project, "c_files");

    orb_try(comp_opt);

    orb_json_array(project, "o_files");
    orb_json_string(comp_opt, NULL, "fPIC");
    orb_json_string(_dep_incl(project), NULL, "includes");

    for(size_t i = 0; i < json_object_array_length(c_files); ++i) {
        const char * cfile;
        struct cmpl_payload * pld;

        cfile = json_object_get_string(json_object_array_get_idx(c_files, i));
        pld = _payload(project, cfile);

        orb_thrds_append(thrds, _file_compile, pld);
    }

    return orb_thrds_join(thrds, _compile_ret_parse);
}
