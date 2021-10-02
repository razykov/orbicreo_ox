#ifndef ORB_JOB_AGENT_H
#define ORB_JOB_AGENT_H

#include "../orb_types/orb_types.h"

//! Task function
typedef i32 (*orb_agent_func)(void *);

/*!
 * \brief Task structure for bulding agent
 */
struct orb_agent_task_t {
    i32 res;             //! Result of call function
    orb_agent_func func; //! Call function
    void * payload;      //! Payload for function

    struct orb_agent_task_t * next; //! Next task ptr
};

/*!
 * \brief Start building agents threads
 * \param njobs Number of threads
 * \return true on success
 */
bool orb_agents_start(u8 njobs);

/*!
 * \brief Append build task to stack
 * \param func Task function
 * \param payload Payload for task function
 * \return true on success
 */
bool orb_agent_task_append(orb_agent_func func, void * payload);

/*!
 * \brief Start build tasks processing and wait finish
 * \return true on success
 */
bool orb_agent_wait(void);

/*!
 * \brief Stop building agents threads
 * \return true on success
 */
bool orb_agent_stop(void);

#endif /* ORB_JOB_AGENT_H */
