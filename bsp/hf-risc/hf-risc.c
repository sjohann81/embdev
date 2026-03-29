#include <hf-risc.h>
#include <interrupt.h>
#include <stdio.h>

#ifndef DEBUG_PORT
#include <uart_hfrisc.h>

/* UART configuration */
static const hfrisc_uart_config_t uart0_config = {
    .base_addr  = UART0_BASE,
    .baud_rate  = BAUD_57600,
    .clock_hz   = CPU_SPEED,
    .tx_mask    = UART0_TXBUSY,
    .rx_mask    = UART0_RXDATA,
    .irq_mask   = UART0_RXDATA,     // comment this line for polling mode
};

static hfrisc_uart_dev_t uart0_dev = {
    .config = &uart0_config,
};

static uart_t uart_console = {
    .ops = &hfrisc_uart_ops,
    .dev = (uart_dev_t *)&uart0_dev,
};

void *uart0rx_handler(void)
{
    uart_irq_handle(&uart_console);
    
    return 0;
}

/* hook to libc stdio (weak symbols) */
int putchar(int c)
{
    uart_tx(&uart_console, c);
   
    return 0;
}

/* hook to libc stdio (weak symbols) */
int getchar(void)
{
    char c;
    
    uart_rx(&uart_console, &c);
    
    return c;
}

__attribute__((weak)) int bsp_init(void)
{
    /* select UART0 pins */
   	PAALTCFG0 |= (MASK_UART0_TX | MASK_UART0_RX);
    uart_init(&uart_console);
    
    return 0;
}

#else

/* hook to libc stdio (weak symbols) */
int putchar(int c)
{
    DEBUG_ADDR = c;
    
    return 0;
}

int getchar(void)
{
    return DEBUG_ADDR;
}

__attribute__((weak)) int bsp_init(void)
{
    return 0;
}

#endif

void bsp_delay_ms(uint32_t msec)
{
    volatile uint32_t cur, last, delta, msecs;
    uint32_t cycles_per_msec;

    last = TIMER0;
    delta = msecs = 0;
    cycles_per_msec = CPU_SPEED / 1000;
    while (msec > msecs) {
        cur = TIMER0;
        if (cur < last)
            delta += (cur + (CPU_SPEED - last));
        else
            delta += (cur - last);
        last = cur;
        if (delta >= cycles_per_msec) {
            msecs += delta / cycles_per_msec;
            delta %= cycles_per_msec;
        }
    }
}

void bsp_delay_us(uint32_t usec)
{
    volatile uint32_t cur, last, delta, usecs;
    uint32_t cycles_per_usec;

    last = TIMER0;
    delta = usecs = 0;
    cycles_per_usec = CPU_SPEED / 1000000;
    while (usec > usecs) {
        cur = TIMER0;
        if (cur < last)
            delta += (cur + (CPU_SPEED - last));
        else
            delta += (cur - last);
        last = cur;
        if (delta >= cycles_per_usec) {
            usecs += delta / cycles_per_usec;
            delta %= cycles_per_usec;
        }
    }
}
