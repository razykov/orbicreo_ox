#define _POSIX_C_SOURCE (200809L)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/orb_args.h"
#include "../src/orb_log.h"
#include "../src/orb_utils.h"
#include "../src/orb_utils_str.h"
#include "../src/orb_build_utils.h"
#include "../src/orb_build_mkinclude.h"


static json_object * _h_files(json_object * project) {
    json_object * array = orb_json_array(project, "h_files");
    orb_find_nexp_files(orb_json_get_string(project, "path"), array, ".h");
    return array;
}

static void _hfile_exports(const char * hfile, FILE * file) {
    FILE * fp;
    bool export = false;
    ssize_t nread;
    size_t len = 0;
    char * line = NULL;

    fp = fopen(hfile, "r");
    if (!fp) return;

    while ((nread = getline(&line, &len, fp)) != -1) {
        char * toexport = NULL;

        if (strstr(line, "EXPORT_FROM")) { export = true;  continue; }
        if (strstr(line, "EXPORT_TO"  )) { export = false; continue; }
        if (strstr(line, "EXPORT "    )) toexport = line + strlen("EXPORT ");

        if (export)        fprintf(file, "%s", line);
        else if (toexport) fprintf(file, "%s", toexport);
    }

    fclose(fp);
}

static void _hfiles_iter(json_object * h_files, FILE * file) {
    for(size_t i = 0; i < json_object_array_length(h_files); ++i) {
        const char * hfile;
        hfile = json_object_get_string(json_object_array_get_idx(h_files, i));
        _hfile_exports(hfile, file);
    }
}

static const char * _include_fname(json_object * project) {
    static __thread char buff[ORB_PATH_SZ];
    const char * pname = orb_json_get_string(project, "project_name");
    sprintf(buff, "%s/includes/lib%s.h", context->root, pname);
    return buff;
}

static const char * _incduard(json_object * project) {
    static __thread char incduard[256] = { 0 };

    if (incduard[0] == '\0') {
        sprintf(incduard, "%s_H", orb_json_get_string(project, "project_name"));
        orb_upstr(incduard);
    }

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
    const char * type = orb_proj_type(project);
    if (!strcmp(type, "shared")) {
        orb_try(orb_mkdir_p(orb_cat(context->root, "includes")));
        orb_try(_mkinclude(project));
    }
    return true;
}
