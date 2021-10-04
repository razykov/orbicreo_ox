#ifndef ORB_CONTEXT_H
#define ORB_CONTEXT_H

#include "../orb_types/orb_project.h"

//! Utility execution goal
enum orb_goal {
    ORB_GOAL_BUILD = 0, //! Build project
    ORB_GOAL_HELP,      //! Print help message
    ORB_GOAL_INIT,      //! Create new monorepo
    ORB_GOAL_LIST,      //! Print list of projects in monorepo
    ORB_GOAL_VERS       //! Print program version
};

/*!
 * \brief Utility context
 */
struct orb_ctx {
    char * appname;       //! Utility name
    char * root;          //! Monorepo root directory path
    char * repo_projects; //! Monorepo projects directory
    char * target_proj;   //! Target project name

    size_t rt_off; //! Monorepo root directory path offset

    struct orb_project * projects; //! Monorepo projects list

    enum orb_goal goal; //! Utility execution goal

    bool verbose;    //! Verbose output
    bool clear;      //! Clear target project
    bool clear_deps; //! Clear dependencies projects
    bool release;    //! Release build

    u8 njobs; //! Number of building agents
};

//! Utility context instance
extern struct orb_ctx context;

/*!
 * \brief Initialization of utility context instance
 * \param argc CLI arguments count
 * \param argv CLI arguments
 * \return true on success
 */
bool orb_ctx_init(i32 argc, char ** argv);

/*!
 * \brief Free up resources for context instance
 */
void orb_ctx_destroy(void);

/*!
 * \brief Find project context by project name
 * \param name Project name
 * \return Project context, NULL on failure
 */
struct orb_project * orb_ctx_get_project(const char * name);

#endif /* ORB_CONTEXT_H */
