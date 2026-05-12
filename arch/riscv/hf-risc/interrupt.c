#include <hf-risc.h>
#include <stdio.h>
#include <interrupt.h>
#include <bsp.h>

/* Trap frame, trap handler and interrupt handlers
 * 
 * default_trap_frame stores the CPU registers while a trap is being
 * handled. The trap handler is a function that handles all interrupt
 * or exceptions, and the handlers it calls can be redefined according
 * to the application needs. HF-RISC does not differentiate interrupts
 * from exceptions, so the same trap handler is used. A RTOS can define
 * one trap frame per task/thread.
 * 
 * In the current hardware implementation, all internal core peripherals
 * are wired to the same interrupt (irq 0) and handled by irq0_handler.
 * External peripherals can use interrupts 1 to 7.
 * 
 * All interrupt handlers for irq 0 are defined as weak, so an application
 * can redefine them directly when they need to be used. If an interrupt
 * is enabled and no handler is defined, a default dummy handler is used
 * instead.
 */
 
/* the default trap frame data structure */
trap_frame_t default_trap_frame;
static trap_frame_t *frame;

/* interrupt handlers */
void *irq0_handler(void);
void *irq1_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *irq2_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *irq3_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *irq4_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *irq5_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *irq6_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *irq7_handler(void) __attribute__ ((weak, alias ("dummy_handler")));

void *porta_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *portb_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *portc_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *portd_handler(void) __attribute__ ((weak, alias ("dummy_handler")));

void *timer0a_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer0b_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer1ctc_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer1ocr_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer1ovf_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer2ctc_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer2ocr_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer2ovf_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer3ctc_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer3ocr_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *timer3ovf_handler(void) __attribute__ ((weak, alias ("dummy_handler")));

void *uart0rx_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *uart0tx_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *uart1rx_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *uart1tx_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *uart2rx_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *uart2tx_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *uart3rx_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *uart3tx_handler(void) __attribute__ ((weak, alias ("dummy_handler")));

void *spi0_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *spi1_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *spi2_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *spi3_handler(void) __attribute__ ((weak, alias ("dummy_handler")));

void *i2c0_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *i2c1_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *i2c2_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *i2c3_handler(void) __attribute__ ((weak, alias ("dummy_handler")));

void *adc0_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *adc1_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *adc2_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *adc3_handler(void) __attribute__ ((weak, alias ("dummy_handler")));

void *dac0_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *dac1_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *dac2_handler(void) __attribute__ ((weak, alias ("dummy_handler")));
void *dac3_handler(void) __attribute__ ((weak, alias ("dummy_handler")));

/* interrupt vectors */
static void *(*irq_vector[])(void) = {
    irq0_handler,
    irq1_handler,
    irq2_handler,
    irq3_handler,
    irq4_handler,
    irq5_handler,
    irq6_handler,
    irq7_handler
};

static void *(*gpio_vector[])(void) = {
    porta_handler,
    portb_handler,
    portc_handler,
    portd_handler
};

static void *(*timer_vector[])(void) = {
    timer0a_handler,
    timer0b_handler,
    timer1ctc_handler,
    timer1ocr_handler,
    timer1ovf_handler,
    timer2ctc_handler,
    timer2ocr_handler,
    timer2ovf_handler,
    timer3ctc_handler,
    timer3ocr_handler,
    timer3ovf_handler,
};

static void *(*uart_vector[])(void) = {
    uart0rx_handler,
    uart0tx_handler,
    uart1rx_handler,
    uart1tx_handler,
    uart2rx_handler,
    uart2tx_handler,
    uart3rx_handler,
    uart3tx_handler,
};

static void *(*spi_vector[])(void) = {
    spi0_handler,
    spi1_handler,
    spi2_handler,
    spi3_handler
};

static void *(*i2c_vector[])(void) = {
    i2c0_handler,
    i2c1_handler,
    i2c2_handler,
    i2c3_handler
};

static void *(*adc_vector[])(void) = {
    adc0_handler,
    adc1_handler,
    adc2_handler,
    adc3_handler
};

static void *(*dac_vector[])(void) = {
    dac0_handler,
    dac1_handler,
    dac2_handler,
    dac3_handler
};

void *dummy_handler(void)
{
    printf("irq!");
    
    return 0;
}

__attribute__ ((weak)) void *irq0_handler(void)
{
    uint32_t irq, k, i, pins;

    k = 1;
    do {
        i = 0;
        switch (S0CAUSE & k) {
        case 0:
            break;
        case MASK_S0CAUSE_BASE:
            break;
        case MASK_S0CAUSE_GPIO:
            irq = (GPIOCAUSE ^ GPIOCAUSEINV) & GPIOMASK;

            do {
                if (irq & 0x1) {
                    /* toggle interrupt cause */
                    switch (i) {
                    case 0:
                        pins = (PAIN ^ PAININV) & PAINMASK;
                        PAININV ^= pins;
                        break;
                    case 1:
                        pins = (PBIN ^ PBININV) & PBINMASK;
                        PBININV ^= pins;
                        break;
                    case 2:
                        pins = (PCIN ^ PCININV) & PCINMASK;
                        PCININV ^= pins;
                        break;
                    case 3:
                        pins = (PDIN ^ PDININV) & PDINMASK;
                        PDININV ^= pins;
                        break;
                    default:
                        break;
                    };
                    /* call irq handler */
                    gpio_vector[i]();
                }
                irq >>= 1;
                ++i;
            } while (irq);
            break;
        case MASK_S0CAUSE_TIMER:
            irq = (TIMERCAUSE ^ TIMERCAUSEINV) & TIMERMASK;

            do {
                if (irq & 0x1) {
                    /* toggle interrupt cause */
                    TIMERCAUSEINV ^= (irq & 0x1) << i;
                    /* call irq handler */
                    if (!frame)
                        frame = (trap_frame_t *)timer_vector[i]();
                    else
                        timer_vector[i]();
                }
                irq >>= 1;
                ++i;
            } while (irq);
            break;
        case MASK_S0CAUSE_UART:
            irq = (UARTCAUSE ^ UARTCAUSEINV) & UARTMASK;

            do {
                if (irq & 0x1) {
                    /* call irq handler */
                    uart_vector[i]();
                }
                irq >>= 1;
                ++i;
            } while (irq);
            break;
        case MASK_S0CAUSE_SPI:
            (void)spi_vector;
            break;
        case MASK_S0CAUSE_I2C:
            (void)i2c_vector;
            break;
        case MASK_S0CAUSE_DAC:
            (void)dac_vector;
            break;
        case MASK_S0CAUSE_ADC:
            (void)adc_vector;
            break;
        default:
            printf("unknown irq source for S0CAUSE %08x\n", S0CAUSE);
            break;
        }
        k <<= 1;
    } while (S0CAUSE && k);
    
    return 0;
}

void irq_enable(void)
{
    /* enable mask for Segment 0 (tied to IRQ0 line) */
    IRQ_MASK = MASK_IRQ0;
    /* global interrupts enable */
    IRQ_STATUS = 1;
}

__attribute__ ((weak)) trap_frame_t *trap_handler(uint32_t cause)
{
    int32_t i = 0;
    trap_frame_t *old_frame;
    
    __asm__ volatile ("mv %0, tp" : "=r"(old_frame));
    frame = NULL;
    
    if (!cause) {               // exception
        if (old_frame) {
            switch (old_frame->a0) {
            case 1:             // SYS_YIELD
                frame = (trap_frame_t *)timer1ctc_handler();
                break;
            case 2:             // SYS_WFI
                frame = old_frame; 
                break;
            case 3:             // SYS_DIE
                while (1) {
                }
                break;
            default:
                printf("\nexception at %08x\n", old_frame->epc - 4);
                printf("a0: %08x, a1: %08x, a2: %08x, a3: %08x ra: %08x\n",
                    old_frame->a0, old_frame->a1, old_frame->a2, old_frame->a3, old_frame->ra);
            }
        }
    } else {                    // interrupt
        do {
            if (cause & 0x1) {
                if (irq_vector[i]) {
                    irq_vector[i]();
                }
            }
            cause >>= 1;
            ++i;
        } while (cause);
    }
    
    return frame;
}
