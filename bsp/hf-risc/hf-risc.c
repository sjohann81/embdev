#include <hf-risc.h>

#ifndef DEBUG_PORT
static void uart_putc(char c)
{
    while (UARTCAUSE & MASK_UART0_WRITEBUSY);
    UART0 = c;
}

static int uart_getc_nonblocking(void)
{
    return UARTCAUSE & MASK_UART0_DATAAVAIL;
}

static char uart_getc(void)
{
    while (!uart_getc_nonblocking());
    return UART0;
}

#else

static void uart_putc(char c)
{
    DEBUG_ADDR = c;
}

static int uart_getc_nonblocking(void)
{
    return 0;
}

static char uart_getc(void)
{
    return DEBUG_ADDR;
}
#endif

/* hook to libc stdio (weak symbols) */
int putchar(int c)
{
    uart_putc(c);
    
    return 0;
}

/* hook to libc stdio (weak symbols) */
int getchar(void)
{
    return uart_getc();
}

void delay_ms(uint32_t msec)
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

void delay_us(uint32_t usec)
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
