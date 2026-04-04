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
	/* mstatus: MPP=3 (Machine mode), MPIE=1 (Enable interrupts after mret) */
	task->ctx.mstatus = (3 << 11) | (1 << 7);
}

/* task yield sleeps the cpu - a task will be scheduled after a timer
 * interrupt fires the interrupt and trap handler */
void port_yield(void)
{
	__asm volatile ("wfi");
}

void port_halt(void)
{
	puts("SYS HALT\n");

	bsp_timer_stop();
	while (1) {
		__asm volatile ("wfi");
	}
}

void *timer_handler(void)
{
    task_t *task;
    
    sys_schedule();
    task = task_current();
    
    return &task->ctx;
}
