#include <hf-risc.h>
#include <kos.h>
#include <port.h>
#include <stdio.h>

/* disable / enable global interrupts */
void port_di(void)
{
	IRQ_STATUS &= ~0x1;
}

void port_ei(void)
{
	IRQ_STATUS |= 0x1;
}

/* initialize task context */
void port_init_context(void *tcb, void *(*task_ptr)(void *), void *arg, unsigned long sp)
{
	task_t *task = (struct task_s *)tcb;
	
	task->ctx.sp = sp;             		// x2 is sp
	task->ctx.a0 = (unsigned long)arg;	// a0
	task->ctx.epc = (unsigned long)task_ptr;
}

/* task yield sleeps the cpu - a task will be scheduled after a timer
 * interrupt fires the interrupt and trap handler */
void port_yield(void)
{
    unsigned int currticks = sys_ticks();
    
    while (currticks == sys_ticks());
}

void port_halt(void)
{
	printf("SYS HALT\n");

	bsp_timer_stop();
	while (1) {
	}
}

void *timer1ctc_handler(void)
{
    task_t *task;
    
    sys_schedule();
    task = task_current();
    
    return &task->ctx;
}
