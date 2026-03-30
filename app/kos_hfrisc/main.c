#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kos.h>

static task_t task1_tcb, task2_tcb, task3_tcb;
static stack_t task1_stack[2048] __attribute__((aligned(16)));
static stack_t task2_stack[2048] __attribute__((aligned(16)));
static stack_t task3_stack[2048] __attribute__((aligned(16)));

void *t1(void *arg);
void *t2(void *arg);
void *t3(void *arg);


void *t1(void *arg)
{
    char *name = (char *)arg;

    while (1) {
        printf("%s (id: %d) Spinning...\n", name, task_id());
        task_yield();
    }
}

void *t2(void *arg)
{
    char *name = (char *)arg;
    
    while (1) {
        printf("%s (id: %d) Spinning...\n", name, task_id());
        task_yield();
    }
}

void *t3(void *arg)
{
    char *name = (char *)arg;
    int count = 0;
    
    while (1) {
        if (count++ == 10)
            task_delay(50);
        if (count == 40)
            task_suspend(3);
        if (count == 70)
            task_resume(3);

        printf("%s (id: %d) Spinning...\n", name, task_id());
        task_yield();
    }
}

void idle_hook(void)
{
    printf("IDLE TASK\n");
}

int main(void) {
    sys_init();
    printf("Starting KOS/RT...\n");
    
    task_add(&task1_tcb, t1, "task A", 20, task1_stack, 2048);
    task_add(&task2_tcb, t2, "task B", 10, task2_stack, 2048);
    task_add(&task3_tcb, t3, "task C", 30, task3_stack, 2048);
    
    sys_start();
    
    return 0;
}
