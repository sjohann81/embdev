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
    uint64_t is_interrupt = mcause & (1ULL << 63);
    uint64_t code = mcause & ~(1ULL << 63);
    uint64_t mepc, mtval;
    frame = NULL;
    
    __asm__ volatile ("csrr %0, mtval" : "=r"(mtval));
    
    if (is_interrupt) {
        switch (code) {
            case 3:                 // Machine Software Interrupt
                break;
            case 7:                 // Machine Timer Interrupt
                bsp_timer_reset();
                frame = (trap_frame_t *)timer_handler();
                break;
            case 11:                // Machine External Interrupt
                break;
            default:
                printf("Unhandled interrupt code: %llu\n", code);
                break;
        }
    } else {
        switch (code) {
            case 8:                 // Environment Call from U-mode (ecall)
                trap_frame_t *caller_frame;
                __asm__ volatile ("csrr %0, mscratch" : "=r"(caller_frame));
                caller_frame->mepc += 4;
                
                switch (caller_frame->a7) {
                    case 1: // SYS_YIELD
                        frame = (trap_frame_t *)timer_handler();  
                        break;
                    case 2: // SYS_WFI
                        __asm__ volatile("wfi");
                        frame = caller_frame; 
                        break;
                    case 3: // SYS_DIE
                        while (1) {
                            __asm__ volatile("wfi");
                        }
                        break;
                    default:
                        printf("Unknown syscall: %lu\n", caller_frame->a7);
                        frame = caller_frame;
                        break;
                }
                break;
            default:
                printf("Unhandled exception code: %llu\n", code);
                __asm__ volatile ("csrr %0, mepc" : "=r"(mepc));
                printf("mepc=%016llx mtval=%016llx\n", mepc, mtval);
                while(1) { __asm__ volatile("wfi"); }
                break;
        }
    }
    
    return frame;
}
