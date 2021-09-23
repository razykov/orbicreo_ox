#ifndef ORB_JOB_AGENT_H
#define ORB_JOB_AGENT_H

#include "../orb_types/orb_types.h"

struct orb_agent_task_t {
    i32 res;
    i32 (*func)(void *);
    void * payload;

    struct orb_agent_task_t * next;
};

bool orb_agents_start(u8 njobs);
bool orb_agent_task_append(i32 (*func)(void *), void * payload);
bool orb_agent_wait(void);
bool orb_agent_stop(void);

#endif /* ORB_JOB_AGENT_H */
