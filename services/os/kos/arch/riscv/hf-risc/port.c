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
    
#if HAS_USER_MODE != 0
#error "HAS_USER_MODE should be 0 on this port"
#endif
}

#define SYS_YIELD   1
#define SYS_WFI     2
#define SYS_DIE     3

/* task yield calls the scheduler - a task will be scheduled after a syscall.
 * exception hardware is, as other things in this CPU, uninplemented,
 * badly implemented, broken, or all three. we have to disable interrupts
 * before the ecall and insert two NOPs after it. NOPs are mandatory,
 * otherwise the ecall will break on return. */
void port_yield(void)
{
    port_di();
    register long syscall_id __asm__("a0") = SYS_YIELD;
    __asm volatile ("ecall" : : "r"(syscall_id) : "memory");
    __asm volatile ("nop");
    __asm volatile ("nop");
}

/* task wait sleeps the cpu (in a busy wait =/) - a task will be
 * scheduled after a timer interrupt fires the interrupt and trap handler */
void port_wait(void)
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
