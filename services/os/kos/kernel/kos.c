#include "kos.h"

static task_t *ptasks;
static task_t *pcurrtask;
static unsigned task_count;
static unsigned int systicks;

/* the idle task */
static task_t idle_task_tcb;
static unsigned char idle_task_stack[MINIMAL_STACK_SIZE] __attribute__((aligned(16)));

static void *idle_task(void *arg)
{
    while (1) {
        idle_hook();
#if PREEMPTIVE_SCHED == 1
        task_wait();
#else
        task_yield();
#endif
    }
    
    // never reached
    return 0;
}

/* task control API */
int task_add(task_t *task, void *(*task_ptr)(void *), void *arg,
    unsigned char priority, void *stack, unsigned int stack_sz) 
{
    task_t *t;
    unsigned long sp;
    unsigned i;
    
    if (stack_sz < MINIMAL_STACK_SIZE)
        return -1;

    if (ptasks == 0) {
        ptasks = task;
        task->next = 0;
    } else {
        t = ptasks;
        while (t->next)
            t = t->next;

        t->next = task;
    }

    task->task_func = task_ptr;
    task->id = task_count + 1;
    task->priority = priority;
    task->pcounter = priority;
    task->state = TASK_READY;
    task->stack_guard = (unsigned long *)stack;
    
    for (i = 0; i < STACK_GUARD_WORDS; i++)
        *(task->stack_guard + i) = STACK_GUARD;

    sp = (unsigned long)stack + stack_sz;
    port_init_context(task, task_ptr, arg, sp);
    task_count++;
    pcurrtask = task;
    
    return task_count;
}

int task_delete(unsigned int task_id)
{
    task_t *task, *last_task;
    
    if (task_id == 0)
        return -1;
        
    task = ptasks;
    last_task = ptasks;
    while (task) {
        if (task->id == task_id) {
            if (task->state != TASK_READY)
                return -2;
            
            last_task->next = task->next;

            return 0;
        }
        last_task = task;
        task = task->next;
    }
    
    return -1;
}

int task_id(void)
{
    return pcurrtask->id;
}

task_t *task_current(void)
{
    return pcurrtask;
}

void task_yield()
{
    port_yield();
}

void task_wait()
{
    port_wait();
}

void task_delay(unsigned int ticks)
{
    unsigned int readticks = systicks;
    
    while (readticks + ticks > systicks)
        task_yield();
}

int task_suspend(unsigned int task_id)
{
    task_t *task;

    if (task_id == 0 || task_id > task_count)
        return -1;

    task = ptasks;
    while (task) {
        if (task->id == task_id) {
            if (task->state == TASK_READY) {
                task->state = TASK_SUSPENDED;

                return 0;
            } else {
                return -2;
            }
        }
        task = task->next;
    }

    return -1;
}

int task_resume(unsigned int task_id)
{
    task_t *task;

    if (task_id == 0 || task_id > task_count)
        return -1;

    task = ptasks;
    while (task) {
        if (task->id == task_id) {
            if (task->state == TASK_SUSPENDED) {
                task->state = TASK_READY;
                task->pcounter = task->priority;

                return 0;
            } else {
                return -2;
            }
        }
        task = task->next;
    }

    return -1;
}

void task_suspend_self(void)
{
    pcurrtask->state = TASK_SUSPENDED;
    task_yield();
}

task_state_t task_get_state(unsigned int task_id)
{
    task_t *task;

    if (task_id > task_count)
        return -1;

    task = ptasks;
    while (task) {
        if (task->id == task_id) {
            return task->state;
        }
        task = task->next;
    }

    return -1;
}

/* kernel control API */
void sys_init(void)
{
    init_hook();
#if PREEMPTIVE_SCHED == 1
    bsp_timer_init(DEFAULT_TICK_FREQ);
#endif
#if HAS_IDLE_TASK == 1
    task_add(&idle_task_tcb, idle_task, "idle", DEFAULT_IDLE_PRIO,
        idle_task_stack, MINIMAL_STACK_SIZE);
#endif
}

void sys_start(void)
{
    pcurrtask = ptasks;
    sys_schedule();
#if PREEMPTIVE_SCHED == 1
    bsp_timer_start();
#endif
    _restore_context(&pcurrtask->ctx);
}

unsigned int sys_ticks(void)
{
    return systicks;
}

/* internal scheduler function */
task_t *sys_schedule(void)
{
    unsigned i;
    
    for (i = 0; i < STACK_GUARD_WORDS; i++) {
        if (*(pcurrtask->stack_guard + i) != STACK_GUARD) {
            stackoverflow_hook();
            port_halt();
        }
    }
    
    if (pcurrtask->state == TASK_RUNNING)
        pcurrtask->state = TASK_READY;
    
    systicks++;
    tick_hook();

    while (1) {
        pcurrtask = pcurrtask->next;
        if (pcurrtask == 0)
            pcurrtask = ptasks;
            
        if (pcurrtask->state != TASK_READY)
            continue;

        if (!--pcurrtask->pcounter) {
            pcurrtask->pcounter = pcurrtask->priority;
            pcurrtask->state = TASK_RUNNING;
            
            return pcurrtask;
        }
    }
}

/* application defined hook functions (called from kernel) */
__attribute__((weak)) void init_hook(void)
{
}

__attribute__((weak)) void tick_hook(void)
{
}

__attribute__((weak)) void idle_hook(void)
{
}

__attribute__((weak)) void stackoverflow_hook(void)
{
}

__attribute__((weak)) void exception_hook(void)
{
}


static event_group_t event_groups[MAX_EVENT_GROUPS];

static int event_condition_met(task_t *task)
{
    event_group_t *evt_grp;
    unsigned int matched_flags;
    
    if (!task || task->state != TASK_BLOCKED)
        return 0;
    
    if (task->event_wait.event_group_id == 0 || 
        task->event_wait.event_group_id > MAX_EVENT_GROUPS)
        return 0;
    
    evt_grp = &event_groups[task->event_wait.event_group_id - 1];
    if (!evt_grp->in_use)
        return 0;
    
    matched_flags = evt_grp->flags & task->event_wait.wait_flags;
    
    if (task->event_wait.wait_option & EVENT_WAIT_AND) {
        return (matched_flags == task->event_wait.wait_flags);
    } else {
        return (matched_flags != 0);
    }
}

static void check_waiting_tasks(void)
{
    task_t *task;
    event_group_t *evt_grp;
    
    task = ptasks;
    while (1) {
        if (!task->task_func || task->state != TASK_BLOCKED) {
            task = task->next;
            continue;
        }
        
        if (event_condition_met(task)) {
            if (task->event_wait.clear_on_exit) {
                evt_grp = &event_groups[task->event_wait.event_group_id - 1];
                evt_grp->flags &= ~task->event_wait.wait_flags;
            }
            
            task->state = TASK_READY;
            task->pcounter = task->priority;
        }
        task = task->next;
    }
}

int event_create(void)
{
    int i;
    
    for (i = 0; i < MAX_EVENT_GROUPS; i++) {
        if (!event_groups[i].in_use) {
            event_groups[i].in_use = 1;
            event_groups[i].id = i + 1;
            event_groups[i].flags = 0;
            
            return event_groups[i].id;
        }
    }

    return -1;
}

int event_delete(unsigned int event_id)
{
    event_group_t *evt_grp;
    task_t *task;
    
    if (event_id == 0 || event_id > MAX_EVENT_GROUPS)
        return -1;
    
    evt_grp = &event_groups[event_id - 1];
    
    if (!evt_grp->in_use)
        return -2;
    
    task = ptasks;
    while (1) {
        if (task->state == TASK_BLOCKED && task->event_wait.event_group_id == event_id) {
            task->state = TASK_READY;
            task->pcounter = task->priority;
        }
        task = task->next;
    }
    
    evt_grp->in_use = 0;
    evt_grp->flags = 0;
    evt_grp->id = 0;
    
    return 0;
}

int event_set(unsigned int event_id, unsigned int flags)
{
    event_group_t *evt_grp;
    
    if (event_id == 0 || event_id > MAX_EVENT_GROUPS)
        return -1;
    
    evt_grp = &event_groups[event_id - 1];
    
    if (!evt_grp->in_use)
        return -2;
    
    evt_grp->flags |= flags;
    check_waiting_tasks();
    
    return 0;
}

int event_clear(unsigned int event_id, unsigned int flags)
{
    event_group_t *evt_grp;
    
    if (event_id == 0 || event_id > MAX_EVENT_GROUPS)
        return -1;
    
    evt_grp = &event_groups[event_id - 1];
    
    if (!evt_grp->in_use)
        return -2;
    
    evt_grp->flags &= ~flags;
    
    return 0;
}

unsigned int event_get(unsigned int event_id)
{
    event_group_t *evt_grp;
    
    if (event_id == 0 || event_id > MAX_EVENT_GROUPS)
        return 0;
    
    evt_grp = &event_groups[event_id - 1];
    
    if (!evt_grp->in_use)
        return 0;
    
    return evt_grp->flags;
}

int event_wait(unsigned int event_id, unsigned int flags, unsigned char options)
{
    event_group_t *evt_grp;
    unsigned int matched_flags;
    
    if (event_id == 0 || event_id > MAX_EVENT_GROUPS)
        return -1;
    
    evt_grp = &event_groups[event_id - 1];
    
    if (!evt_grp->in_use)
        return -2;
    
    matched_flags = evt_grp->flags & flags;
    
    if (options & EVENT_WAIT_AND) {
        if (matched_flags == flags) {
            if (options & EVENT_CLEAR) {
                evt_grp->flags &= ~flags;
            }
            return 0;
        }
    } else {
        if (matched_flags != 0) {
            if (options & EVENT_CLEAR) {
                evt_grp->flags &= ~matched_flags;
            }
            return 0;
        }
    }
    
    pcurrtask->state = TASK_BLOCKED;
    pcurrtask->event_wait.event_group_id = event_id;
    pcurrtask->event_wait.wait_flags = flags;
    pcurrtask->event_wait.wait_option = options;
    pcurrtask->event_wait.clear_on_exit = (options & EVENT_CLEAR) ? 1 : 0;
    task_yield();
    
    return 0;
}

