#include <stdio.h>
#include "uart_hal.h"
#include "uart_hfrisc.h"
#include "uart_mock.h"

/* HF-RISC native */
void *uart0rx_handler(void);

static const uart_hfrisc_config_t hfrisc_cfg = {
    .port           = 0,
    .base_addr      = UART0_BASE,
    .int_base_addr  = UART_BASE,
    .clock_hz       = CPU_SPEED,
    .irq_mode       = UART_IRQ_ENABLE,
    .baud_rate      = UART_BAUD_57600,
};

static uart_hfrisc_dev_t hfrisc_dev = {
    .config = &hfrisc_cfg,
};

const uart_t uart0 = {
    .ops = &uart_hfrisc_ops,
    .dev = &hfrisc_dev,
};

void *uart0rx_handler(void)
{
    uart_irq_handle(&uart0);
    
    return 0;
}

/* UART mock driver */
static const uart_mock_config_t mock_cfg = {
    .base_addr = 0x40010000,        /* fictitious peripheral address */
    .clock_hz  = 25000000,
    .baud_rate = UART_BAUD_9600,
};

static uart_mock_dev_t mock_dev = {
    .config = &mock_cfg,
};

const uart_t uart1 = {
    .ops = &mock_uart_ops,
    .dev = &mock_dev,
};


int main(void)
{
    char buf[80];
    
    uart_init(&uart0);
    uart_init(&uart1);
    
    uart_write(&uart0, "hello from hfrisc\n", 18);
    // writing or reading to mock driver will hang the application
//    uart_write(&uart1, "hello from mock\n", 16);    

    while (1) {
        uart_read(&uart0, buf, 32);
        uart_write(&uart0, buf, 32);
//        uart_write(&uart1, buf, 32);
    }
    
    return 0;
}
