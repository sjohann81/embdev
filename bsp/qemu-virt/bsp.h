#ifndef _BSP_BSP_H
#define _BSP_BSP_H

void uart_putc(char c);
char uart_getc(void);
int uart_getc_nonblocking(char *c);
int uart_kbhit(void);

int bsp_init(void);
void bsp_delay_ms(uint32_t msec);
void bsp_delay_us(uint32_t usec);
uint64_t bsp_read_us(void);
void bsp_timer_reset(void);
void bsp_timer_init(uint32_t tick_rate_hz);
void bsp_timer_start(void);
void bsp_timer_stop(void);

#endif
