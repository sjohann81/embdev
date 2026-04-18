/*
 * Definitions of the HF-RISC UART driver. This should be used by the
 * driver itself and by an application, along with the generic UART HAL.
 * Any application the want to use this driver should:
 * a) Include uart_hal.h and uart_hfrisc.h
 * b) Declare a configuration and its parameters for this driver
 * c) Declare device for this driver, and reference the configuration
 *    in it
 * d) Declare a uart_t handle which references the driver API and the
 *    device for this driver.
 */

#ifndef UART_HFRISC_H
#define UART_HFRISC_H

/* 
 * redefinition of UART base addresses and masks for HF-RISC
 * check hf-risc.h and interrupt.c for reference
 */
#define UART0_BASE              (UART_BASE + 0x4000)
#define UART1_BASE              (UART_BASE + 0x4400)
#define UART2_BASE              (UART_BASE + 0x4800)
#define UART3_BASE              (UART_BASE + 0x4c00)

/* software FIFO size, should be a power of two */
#define UART_FIFO_SIZE          128

/* HF-RISC UART configuration */
typedef struct {
    uint8_t port;
    uintptr_t base_addr;
    uintptr_t int_base_addr;
    uint32_t clock_hz;
    uart_irq_t irq_mode;
    uint32_t baud_rate;
} uart_hfrisc_config_t;

/* software FIFO data type */
typedef struct {
    char data[UART_FIFO_SIZE];
    volatile uint32_t head, tail;
} uart_hfrisc_fifo_t;

/* HF-RISC UART device
 * 
 * This device has only a configuration (static) and a RX data
 * FIFO (state). The state of the driver is kept along with the
 * instance, and not the driver itself.
 */
typedef struct uart_hfrisc_dev {
    const uart_hfrisc_config_t *config;
    uart_hfrisc_fifo_t rx_fifo;
} uart_hfrisc_dev_t;

extern const uart_ops_t uart_hfrisc_ops;

#endif
