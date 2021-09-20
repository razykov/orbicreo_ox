#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE (200809L)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../orb_utils/orb_args.h"
#include "../orb_utils/orb_log.h"
#include "../orb_utils/orb_utils.h"
#include "../orb_utils/orb_utils_str.h"
#include "../orb_build/orb_build_utils.h"
#include "../orb_build/orb_build_mkinclude.h"


static void _hfile_exports(const char * hfile,
                           FILE * file, json_object * exported);

static json_object * _h_files(json_object * project) {
    json_object * array = orb_json_array(project, "h_files");
    orb_find_nexp_files(orb_json_get_string(project, "project_path"), array, ".h");
    return array;
}

static bool _continue_export(const char * hfile, json_object * exported)  {
    json_object * exist = orb_json_find(exported, hfile);
    return exist == NULL;
}

static const char * _dep_hfile_name(const char * line) {
    char * q1;
    char * q2;
    static __thread char buff[B_KB(1)];

    memset(buff, 0, B_KB(1));
    q1 = strchr(line, '"');
    if (!q1) return NULL;
    q2 = strchr(q1 + 1, '"');
    if (!q2) return NULL;
    memcpy(buff, q1 + 1, q2 - q1 - 1);

    return buff;
}

static const char * _dep_hfile_dir(const char * hfile) {
    const char * slh = hfile;
    const char * ptr = hfile;
    static __thread char buff[B_KB(2)];

    do if (*ptr == '/') slh = ptr;
    while (*(ptr++));

    strncpy(buff, hfile, slh - hfile);

    return buff;
}

static void export_depends(const char * hfile,
                           FILE * file, json_object * exported) {
    FILE * fp;
    ssize_t nread;
    size_t len = 0;
    char * line = NULL;
    char * inc;

    fp = fopen(hfile, "r");
    if (!fp) return;

    while ((nread = getline(&line, &len, fp)) != -1) {
        const char * deph;
        const char * dep_file;
        char rpath[B_KB(4)];

        if ( !(inc = strstr(line, "#include")) )
            continue;

        deph = _dep_hfile_name(inc + strlen("#include"));
        if (!deph) continue;
        dep_file = orb_cat(_dep_hfile_dir(hfile), deph);
        if (realpath(dep_file, rpath) == NULL)
            continue;

        _hfile_exports(rpath, file, exported);
    }

    ((void)file);
    ((void)exported);

    free(line);
    fclose(fp);
}

static void _hfile_exports(const char * hfile,
                           FILE * file, json_object * exported) {
    FILE * fp;
    bool export = false;
    ssize_t nread;
    size_t len = 0;
    char * line = NULL;

    if (!_continue_export(hfile, exported)) return;
    export_depends(hfile, file, exported);

    fp = fopen(hfile, "r");
    if (!fp) return;

    while ((nread = getline(&line, &len, fp)) != -1) {
        char * toexport = NULL;

        if (strstr(line, "EXPORT_FROM")) { export = true;  continue; }
        if (strstr(line, "EXPORT_TO"  )) { export = false; continue; }
        if (strstr(line, "EXPORT "    ))
            toexport = line + strlen("EXPORT ");

        if (export)        fprintf(file, "%s", line);
        else if (toexport) fprintf(file, "%s", toexport);
    }

    free(line);
    fclose(fp);

    orb_json_bool(exported, hfile, true);
}

static void _hfiles_iter(json_object * h_files, FILE * file) {
    json_object * exported = orb_json_object(NULL, NULL);

    for(size_t i = 0; i < json_object_array_length(h_files); ++i) {
        const char * hfile;
        hfile = json_object_get_string(json_object_array_get_idx(h_files, i));
        _hfile_exports(hfile, file, exported);
    }

    json_object_put(exported);
}

static const char * _include_fname(json_object * project) {
    static __thread char buff[ORB_PATH_SZ];
    const char * pname = orb_json_get_string(project, "project_name");
    sprintf(buff, "%s/includes/lib%s.h", context->root, pname);
    return buff;
}

static const char * _incduard(json_object * project) {
    static __thread char incduard[256] = { 0 };
    sprintf(incduard, "%s_H", orb_json_get_string(project, "project_name"));
    orb_upstr(incduard);
    return incduard;
}

static json_object * _export_includes(json_object * project) {
    project = orb_json_find(project, "recipe");
    project = orb_json_find(project, "general");
    project = orb_json_find(project, "export_includes");
    return project;
}

static void _hfiles_append(json_object * project, FILE * file) {
    json_object * eicnl = _export_includes(project);

    if(!eicnl) return;

    for(size_t i = 0; i < json_object_array_length(eicnl); ++i) {
        const char * str;
        str = json_object_get_string(json_object_array_get_idx(eicnl, i));
        fprintf(file, "#include %s\n", str);
    }
    fprintf(file, "\n");
}

static void _start_proj_header(json_object * project, FILE * file)  {
    fprintf(file, "#ifndef %s\n",   _incduard(project));
    fprintf(file, "#define %s\n\n", _incduard(project));

    _hfiles_append(project, file);
}

static void _end_proj_header(json_object * project, FILE * file)  {
    fprintf(file, "#endif /* %s */\n", _incduard(project));
}

static bool _mkinclude(json_object * project) {
    FILE * file;
    const char * fname = _include_fname(project);
    json_object * h_files = _h_files(project);

    file = fopen(fname, "w+");
    if(!file) return false;

    _start_proj_header(project, file);
    _hfiles_iter(h_files, file);
    _end_proj_header(project, file);

    return fclose(file) == 0;
}

bool orb_mkinclude(json_object * project) {
     bool res = false;
    const char * type = orb_proj_type(project);

    if (!strcmp(type, "shared")) {
        u32 root_off = strlen(orb_json_get_string(project, "repo_root")) + sizeof(char);

        orb_inf("Project header file generating");

        if (orb_mkdir_p(orb_cat(context->root, "includes")))
            if (_mkinclude(project))
                res = true;

        orb_stat(res ? PPL : RED, NULL,
                                  "  %s", _include_fname(project) + root_off);
    }
    return true;
}
