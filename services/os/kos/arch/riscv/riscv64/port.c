#include <kos.h>
#include <port.h>
#include <stdio.h>

/* disable / enable global interrupts */
void port_di(void)
{
	__asm volatile ("csrci mstatus, 0x8" ::: "memory");
}

void port_ei(void)
{
	__asm volatile ("csrsi mstatus, 0x8" ::: "memory");
}

/* initialize task context - global interrupts are enabled automatically
 * after a task context switch */
void port_init_context(void *tcb, void *(*task_ptr)(void *), void *arg, unsigned long sp)
{
	task_t *task = (struct task_s *)tcb;
	
	task->ctx.sp = sp;             		// x2 is sp
	task->ctx.a0 = (unsigned long)arg;	// a0
	task->ctx.mepc = (unsigned long)task_ptr;
#if HAS_USER_MODE == 0
	/* mstatus: MPP=3 (Machine mode), MPIE=1 (Enable interrupts after mret) */
	task->ctx.mstatus = (3 << 11) | (1 << 7);
#else
    /* mstatus: MPP=0 (User mode), MPIE=1 (Enable interrupts after mret) */
	task->ctx.mstatus = (0 << 11) | (1 << 7);
#endif
}

#define SYS_YIELD   1
#define SYS_WFI     2
#define SYS_DIE     3

/* task yield calls the scheduler - a task will be scheduled now */
void port_yield(void)
{
    register long syscall_id __asm__("a7") = SYS_YIELD;
    __asm volatile ("ecall" : : "r"(syscall_id) : "memory");
}

/* task wait sleeps the cpu - a task will be scheduled after a timer
 * interrupt fires the interrupt and trap handler */
void port_wait(void)
{
#if HAS_USER_MODE == 0
	__asm volatile ("wfi");
#else
    register long syscall_id __asm__("a7") = SYS_WFI;
    __asm volatile ("ecall" : : "r"(syscall_id) : "memory");
#endif
}

void port_halt(void)
{
	puts("SYS HALT\n");

	bsp_timer_stop();
	while (1) {
#if HAS_USER_MODE == 0
		__asm volatile ("wfi");
#else
        register long syscall_id __asm__("a7") = SYS_DIE;
        __asm volatile ("ecall" : : "r"(syscall_id) : "memory");
#endif
	}
}

void *timer_handler(void)
{
    task_t *task;
    
    sys_schedule();
    task = task_current();
    
    return &task->ctx;
}
