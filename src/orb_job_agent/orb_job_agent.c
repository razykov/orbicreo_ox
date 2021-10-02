#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include "orb_job_agent.h"
#include "../orb_utils/orb_log.h"

#define MAX_JOBS 256

static bool is_run = false;

static struct orb_agent_task_t * todo_stack = NULL;
static pthread_mutex_t todo_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t todo_cond = PTHREAD_COND_INITIALIZER;

static struct orb_agent_task_t * done_stack = NULL;
static pthread_mutex_t done_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;

static u8 wait_jobs_count = 0;
static u8 total_jobs = 0;
static pthread_t jobs[MAX_JOBS] = { 0 };


static struct orb_agent_task_t * _next_task(void)
{
    struct orb_agent_task_t * task = NULL;

    if (pthread_mutex_lock(&todo_mtx) != 0) {
        orb_err("pthread_mutex_lock");
        return NULL;
    }

    while (is_run && (todo_stack == NULL)) {
        ++wait_jobs_count;

        if (wait_jobs_count == total_jobs)
            pthread_cond_signal(&done_cond);

        if (pthread_cond_wait(&todo_cond, &todo_mtx) != 0) {
            orb_err("pthread_cond_wait");
            return NULL;
        }
        --wait_jobs_count;
    }

    if (is_run) {
        task = todo_stack;
        todo_stack = todo_stack->next;
    }

    if (pthread_mutex_unlock(&todo_mtx) != 0)
        orb_err("pthread_mutex_unlock");

    return task;
}

static void _task_append(struct orb_agent_task_t ** stack,
                         struct orb_agent_task_t * task, pthread_mutex_t * mtx)
{
    pthread_mutex_lock(mtx);
    task->next = *stack;
    *stack = task;
    pthread_mutex_unlock(mtx);
}

bool orb_agent_task_append(orb_agent_func func, void * payload)
{
    struct orb_agent_task_t * task;

    if (!func)
        return false;

    task = malloc(sizeof(struct orb_agent_task_t));
    if (!task)
        return false;

    task->func = func;
    task->payload = payload;
    _task_append(&todo_stack, task, &todo_mtx);

    return true;
}

static void * _job_routine(void * payload)
{
    struct orb_agent_task_t * task;

    while (is_run) {
        if ( (task = _next_task()) ) {
            task->res = task->func(task->payload);
            _task_append(&done_stack, task, &done_mtx);
        }
    }

    ((void)payload);
    return NULL;
}

bool orb_agents_start(u8 njobs)
{
    is_run = true;

    for (u8 i = 0; i < njobs; ++i) {
        if (pthread_create(&jobs[i], NULL, _job_routine, NULL) != 0) {
            orb_err("job create failed");
            return false;
        }
    }
    total_jobs = njobs;

    return true;
}

static bool _stack_clean(struct orb_agent_task_t ** stack)
{
    i32 res = 0;
    struct orb_agent_task_t * task;

    while (*stack) {
        task = *stack;
        *stack = task->next;

        res |= task->res;

        free(task);
    }

    return res == 0;
}

bool orb_agent_wait(void)
{
    bool res;

    pthread_mutex_lock(&done_mtx);

    pthread_cond_broadcast(&todo_cond);
    do {
        i32 r;
        struct timespec tm;

        tm.tv_sec =  time(NULL) + 2;
        tm.tv_nsec = 0;

        r = pthread_cond_timedwait(&done_cond, &done_mtx, &tm);
        if (r == ETIMEDOUT)
            pthread_cond_broadcast(&todo_cond);

    } while(todo_stack || (wait_jobs_count != total_jobs) );

    res = _stack_clean(&done_stack);

    pthread_mutex_unlock(&done_mtx);

    return res;
}

bool orb_agent_stop(void)
{
    i32 res = 0;

    is_run = false;
    pthread_cond_broadcast(&todo_cond);
    for (u8 i = 0; i < total_jobs; ++i)
        res |= pthread_join(jobs[i], NULL);

    return res == 0;
}
