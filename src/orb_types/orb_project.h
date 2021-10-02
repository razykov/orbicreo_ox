#ifndef ORB_PROJECT_H
#define ORB_PROJECT_H

#include "../orb_types/orb_json.h"
#include "../orb_types/orb_types.h"

/*!
 * \brief Project context struture
 */
struct orb_project {
    const char * name;   //! Project name
    const char * type;   //! Project type ("shared", "app")
    char * root;         //! Project's root directory path
    char * dirname;      //! Project's root directory name
    char * meta_path;    //! meta.json file path
    char * recipe_path;  //! Project recipe file path
    char * version_path; //! Project version file path
    char * objs_path;    //! Project objects directory path

    json_object * meta;    //! Project's meta-info
    json_object * version; //! Project's version info

    struct recipe {
        json_object * obj; //! Project's recipe

        json_object * exp_incl;           //! Exported includes
        json_object * dependency_list;    //! Project's dependency list
        json_object * dependency_include; //! Project's looks .h files directories

        char * output_file;  //! Binary file path
        char * bin_file_dir; //! Binary file directory path
    } recipe;

    //! Paths project files
    struct files {
        json_object * h;
        json_object * c;
        json_object * o;
        json_object * o_old;
    } files;

    bool built;        //! Project built flag
    bool compile_turn; //! Project should be rebuild flag

    struct orb_project * next; //! Next project in list
};

/*!
 * \brief Create new project instance from recipe file
 * \param dname Project directory name
 * \return Project instance
 */
struct orb_project * orb_proj_create(const char * dname);

/*!
 * \brief Free up project instance
 * \param project Project instance
 */
void orb_project_free(struct orb_project * project);

#endif /* ORB_PROJECT_H */
