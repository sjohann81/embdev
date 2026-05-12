#ifndef _TASK_H
#define _TASK_H

/* task states */
typedef enum {
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED
} task_state_t;

/* task control block */
typedef struct task_s {
    struct task_s *next;
    void *(*task_func)(void *);
    trap_frame_t ctx;
    void *stack;
    unsigned long *stack_guard;
    unsigned long stack_sz;
    unsigned int id;
    unsigned char priority;
    unsigned char pcounter;
    task_state_t state;
    event_wait_t event_wait;
} task_t;

typedef unsigned char stack_t;

/* task control API */
int task_add(task_t *task, void *(*task_ptr)(void *), void *arg, 
    unsigned char priority, void *stack, unsigned int stack_sz);
int task_delete(unsigned int task_id);
int task_id(void);
task_t *task_current(void);
void task_yield(void);
void task_wait(void);
void task_delay(unsigned int ticks);
int task_suspend(unsigned int task_id);
int task_resume(unsigned int task_id);
void task_suspend_self(void);
task_state_t task_get_state(unsigned int task_id);

#endif
