/*
 * Mock UART driver implementation.
 *
 * Unlike the HF-RISC UART, this fictitious device has a conventional
 * per-device status register that includes TX/RX flags alongside the
 * data and divisor registers.
 */

#include "uart_hal.h"
#include "uart_mock.h"

/* Register layout for this fictitious device */
typedef struct {
    volatile uint32_t DATA;
    volatile uint32_t DIV;
    volatile uint32_t STATUS;
} uart_mock_regs_t;

/* STATUS register bits */
#define MOCK_STATUS_RXRDY   (1u << 0)
#define MOCK_STATUS_TXBUSY  (1u << 1)

/* Internal helpers */
static inline uart_mock_dev_t *dev_adapter(uart_dev_t *dev)
{
    return (uart_mock_dev_t *)dev;
}

static inline uart_mock_regs_t *get_regs(uart_dev_t *dev)
{
    return (uart_mock_regs_t *)dev_adapter(dev)->config->base_addr;
}

/* Driver API implementation */
static uart_status_t driver_init(uart_dev_t *dev)
{
    uart_mock_dev_t  *device = dev_adapter(dev);
    uart_mock_regs_t *regs = get_regs(dev);

    regs->DIV  = device->config->clock_hz / device->config->baud_rate;
    regs->DATA = 0;

    return UART_OK;
}

static uart_status_t driver_tx_busy(uart_dev_t *dev)
{
    uart_mock_regs_t *regs = get_regs(dev);

    if (regs->STATUS & MOCK_STATUS_TXBUSY)
        return UART_TX_BUSY;

    return UART_OK;
}

static uart_status_t driver_rx_data(uart_dev_t *dev)
{
    uart_mock_regs_t *regs = get_regs(dev);

    if (regs->STATUS & MOCK_STATUS_RXRDY)
        return UART_RX_DATA;

    return UART_OK;
}

static uart_status_t driver_tx(uart_dev_t *dev, char ch)
{
    uart_mock_regs_t *regs = get_regs(dev);

    while (regs->STATUS & MOCK_STATUS_TXBUSY);
    regs->DATA = (uint32_t)ch;

    return UART_OK;
}

static uart_status_t driver_rx(uart_dev_t *dev, char *ch)
{
    uart_mock_regs_t *regs = get_regs(dev);

    while (!(regs->STATUS & MOCK_STATUS_RXRDY));
    *ch = (char)regs->DATA;

    return UART_OK;
}

static uart_status_t driver_irq_handler(uart_dev_t *dev)
{
    (void)dev;
    return UART_ERR;
}

const uart_ops_t mock_uart_ops = {
    .init        = driver_init,
    .tx_busy     = driver_tx_busy,
    .rx_data     = driver_rx_data,
    .tx          = driver_tx,
    .rx          = driver_rx,
    .irq_handler = driver_irq_handler,
};
