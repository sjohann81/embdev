/*
 * Mock UART driver for a fictitious platform with a conventional
 * per-device status register layout. 
 *
 * Applications should:
 *   a) Include uart_hal.h and uart_mock.h
 *   b) Declare a uart_mock_config_t and fill in its fields
 *   c) Declare a uart_mock_dev_t and point its config to the above
 *   d) Declare a uart_t handle referencing mock_uart_ops and the device
 */

#ifndef UART_MOCK_H
#define UART_MOCK_H

#include "uart_hal.h"

/* mock UART configuration  */
typedef struct {
    uintptr_t base_addr;
    uint32_t clock_hz;
    uint32_t baud_rate;
} uart_mock_config_t;

/* mock UART device  */
typedef struct uart_mock_dev {
    const uart_mock_config_t *config;
} uart_mock_dev_t;

/* driver ops table */
extern const uart_ops_t mock_uart_ops;

#endif /* UART_MOCK_H */
