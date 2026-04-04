#include <stdint.h>
#include <stdarg.h>
#include <bsp.h>
#include <stdio.h>

/*
 * QEMU RISC-V virt machine uses CLINT (Core-Local Interruptor)
 * for machine timer interrupts
 */

#define CLINT_BASE      0x2000000
#define MTIME           (*(volatile unsigned long *)(CLINT_BASE + 0xbff8))
#define MTIMECMP        (*(volatile unsigned long *)(CLINT_BASE + 0x4000))
#define TIMEBASE_FREQ   10000000UL  /* 10 MHz timebase for QEMU virt */

#define ENABLE_MEIE()  __asm__ volatile("csrs mie, %0" :: "r"(0x80))

static unsigned int timer_interval = 0;
static volatile int timer_enabled = 0;


uint64_t bsp_read_us(void)
{
	return (MTIME / (TIMEBASE_FREQ / 1000000));
}

void bsp_timer_reset(void)
{
    if (!timer_enabled)
        return;

    MTIMECMP = MTIME + timer_interval;
}

void bsp_timer_init(uint32_t tick_rate_hz)
{
    timer_interval = TIMEBASE_FREQ / tick_rate_hz;
    timer_enabled = 0;
    MTIMECMP = MTIME + 0xFFFFFFFF;
    ENABLE_MEIE();
}

void bsp_timer_start(void)
{
    MTIMECMP = MTIME + timer_interval;
    timer_enabled = 1;
}

void bsp_timer_stop(void)
{
    timer_enabled = 0;
    MTIMECMP = MTIME + 0xFFFFFFFF;
}


/* QEMU virt machine UART (16550) */
#define UART_BASE    0x10000000
#define UART_RBR    ((volatile uint8_t *)(UART_BASE + 0))    /* Receiver Buffer Register */
#define UART_THR    ((volatile uint8_t *)(UART_BASE + 0))    /* Transmitter Holding Register */
#define UART_IER    ((volatile uint8_t *)(UART_BASE + 1))    /* Interrupt Enable Register */
#define UART_FCR    ((volatile uint8_t *)(UART_BASE + 2))    /* FIFO Control Register */
#define UART_LCR    ((volatile uint8_t *)(UART_BASE + 3))    /* Line Control Register */
#define UART_MCR    ((volatile uint8_t *)(UART_BASE + 4))    /* Modem Control Register */
#define UART_LSR    ((volatile uint8_t *)(UART_BASE + 5))    /* Line Status Register */
#define UART_MSR    ((volatile uint8_t *)(UART_BASE + 6))    /* Modem Status Register */

/* Line Status Register bits */
#define UART_LSR_DR     0x01  /* Data Ready */
#define UART_LSR_OE     0x02  /* Overrun Error */
#define UART_LSR_PE     0x04  /* Parity Error */
#define UART_LSR_FE     0x08  /* Framing Error */
#define UART_LSR_BI     0x10  /* Break Interrupt */
#define UART_LSR_THRE   0x20  /* Transmit Holding Register Empty */
#define UART_LSR_TEMT   0x40  /* Transmitter Empty */
#define UART_LSR_RXFE   0x80  /* RX FIFO Error */

void uart_init(void);

void uart_init(void)
{
    /* For QEMU virt machine, UART is already initialized */
    /* But we can configure it if needed */
    *UART_LCR = 0x03;    /* 8 bits, no parity, 1 stop bit */
    *UART_FCR = 0x07;    /* Enable FIFO, clear TX/RX */
    *UART_IER = 0x00;    /* Disable interrupts */
}

__attribute__((weak)) int bsp_init(void)
{
    uart_init();
    
    return 0;
}

/* hook to libc stdio (weak symbols) */
int putchar(int c)
{
    /* Wait until THR is empty */
    while ((*UART_LSR & UART_LSR_THRE) == 0);
    
    *UART_THR = c;
    
    return 0;
}

/* hook to libc stdio (weak symbols) */
int getchar(void)
{
    while ((*UART_LSR & UART_LSR_DR) == 0);

    return *UART_RBR;
}
