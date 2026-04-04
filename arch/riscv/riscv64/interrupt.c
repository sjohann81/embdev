#include <stdio.h>
#include <stdint.h>
#include <interrupt.h>
#include <bsp.h>

/* the default trap frame data structure */
trap_frame_t default_trap_frame;
static trap_frame_t *frame;

__attribute__ ((weak)) void *timer_handler(void)
{
    return NULL;
}

__attribute__ ((weak)) trap_frame_t *trap_handler(uint64_t mcause)
{
	uint32_t is_interrupt = mcause & 0x80000000;
	uint32_t exception_code = mcause & 0x7FFFFFFF;
	uint32_t mepc, mtval;
    
    frame = NULL;
	__asm__ volatile ("csrr %0, mtval" : "=r"(mtval));
	if (is_interrupt || mtval == 0) {
		/* Handle interrupt */
		switch (exception_code) {
		case 3:				// machine software interrupt
			break;
		case 7:				// machine timer interrupt
			bsp_timer_reset();
            frame = (trap_frame_t *)timer_handler();
			break;
		case 11:			// machine external interrupt
			break;
		default:
			printf("Unhandled interrupt: %08x\n", exception_code);
		}
	} else {
        printf("Unhandled interrupt: %08x\n", exception_code);
		__asm__ volatile ("csrr %0, mepc" : "=r"(mepc));
		printf("mepc=%08x mtval=%08x\n", mepc, mtval);
	}
    
    return frame;
}
