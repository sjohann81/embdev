/*
 * HF-RISC UART is a very poor implementation of a UART in hardware.
 * It does not have a way to change data bits, stop bits and more
 * flexible interrupt system. The registers of each device are very
 * limited (only a shared TX/RX data register and a baud rate divisor).
 * The way UARTs are wired on the platform is very limiting, because the
 * status of UARTs is not defined on a per peripheral status and mask
 * registers, mapped on each device address space, which would be more
 * common. This is handled on a *_regs.h file.
 * 
 * To make matters worse, there is no TX or RX FIFO in hardware,
 * so TX interrupts are useless. We will only use a RX software FIFO
 * when operating this driver in interrupt mode, and hope for the best.
 * TX will be handled in polling mode only, although technically
 * supported by the hardware.
 * 
 * This driver assumes that only one thread will access an instance
 * at a time (this can be guaranteed with the use of a mutex if an
 * RTOS is used). As just one execution context puts data in the FIFO
 * (ISR) and another context gets data from it (application) the 
 * implementation of FIFO is lockless.
 */

#include "uart_hal.h"
#include "uart_hfrisc.h"
#include "uart_hfrisc_regs.h"
#include <stdbool.h>
#include <stdatomic.h>

/* driver internal function prototypes */
static inline uart_hfrisc_regs_t *get_regs(uart_dev_t *dev);
static inline uart_hfrisc_irq_regs_t *get_irq_regs(uart_dev_t *dev);

static bool fifo_full(uart_hfrisc_fifo_t *fifo);
static bool fifo_empty(uart_hfrisc_fifo_t *fifo);
static uart_status_t fifo_put(uart_hfrisc_fifo_t *fifo, char c);
static uart_status_t fifo_get(uart_hfrisc_fifo_t *fifo, char *c);

static uart_status_t driver_irq_handler(uart_dev_t *dev);
static uart_status_t driver_init(uart_dev_t *dev);
static uart_status_t driver_tx_busy(uart_dev_t *dev);
static uart_status_t driver_rx_data(uart_dev_t *dev);
static uart_status_t driver_tx(uart_dev_t *dev, char ch);
static uart_status_t driver_rx(uart_dev_t *dev, char *ch);

/* abstract type adapter */
static inline uart_hfrisc_dev_t *dev_adapter(uart_dev_t *dev)
{
    return (uart_hfrisc_dev_t *)dev;
}

/* port and interrupt registers access */
static inline uart_hfrisc_regs_t *get_regs(uart_dev_t *dev)
{
    return (uart_hfrisc_regs_t *)dev_adapter(dev)->config->base_addr;
}

static inline uart_hfrisc_irq_regs_t *get_irq_regs(uart_dev_t *dev)
{
    return (uart_hfrisc_irq_regs_t *)dev_adapter(dev)->config->int_base_addr;
}


/* software FIFO control */
static bool fifo_full(uart_hfrisc_fifo_t *fifo)
{
    return (((fifo->tail + 1) & (UART_FIFO_SIZE - 1)) == fifo->head);
}

static bool fifo_empty(uart_hfrisc_fifo_t *fifo)
{
    return fifo->head == fifo->tail;
}

static uart_status_t fifo_put(uart_hfrisc_fifo_t *fifo, char c)
{
    if (!fifo_full(fifo)) {
        fifo->data[fifo->tail] = c;
        atomic_signal_fence(memory_order_release);
        fifo->tail = (fifo->tail + 1) & (UART_FIFO_SIZE - 1);
        
        return UART_OK;
    } else {

        return UART_ERR_OVF;
    }
}

static uart_status_t fifo_get(uart_hfrisc_fifo_t *fifo, char *c)
{
    if (!fifo_empty(fifo)) {
        *c = fifo->data[fifo->head];
        atomic_signal_fence(memory_order_release);
        fifo->head = (fifo->head + 1) & (UART_FIFO_SIZE - 1);
        
        return UART_OK;
    } else {

        return UART_ERR_OVF;
    }
}


/* generic interrupt handler */
static uart_status_t driver_irq_handler(uart_dev_t *dev)
{
    uart_hfrisc_regs_t *regs = get_regs(dev);
    uart_hfrisc_irq_regs_t *irq_regs = get_irq_regs(dev);
    uart_hfrisc_dev_t *device = dev_adapter(dev);
    uint32_t rx_mask = STATUS_RXDATA << (device->config->port << 1);
    uart_status_t err;
    char ch;
    
    /* if RX interrupts are enabled and RX FIFO is not full,
     * take one char received from the wire and put it on FIFO */
    if (device->config->irq_mode == INT_ENABLE) {
        while (irq_regs->CAUSE & rx_mask) {
            if (!fifo_full(&device->rx_fifo)) {
                ch = regs->RXR;
                err = fifo_put(&device->rx_fifo, ch);
                
                if (err != UART_OK) return err;
            }
        }
    }
    
    return UART_OK;
}

/* low level driver API implementation */
static uart_status_t driver_init(uart_dev_t *dev)
{
    uart_hfrisc_regs_t *regs = get_regs(dev);
    uart_hfrisc_irq_regs_t *irq_regs = get_irq_regs(dev);
    uart_hfrisc_dev_t *device = dev_adapter(dev);
    uint32_t rx_mask = STATUS_RXDATA << (device->config->port << 1);
    
	regs->DIV = device->config->clock_hz / device->config->baud_rate;
	regs->TXR = 0;

    device->rx_fifo.head = 0;
    device->rx_fifo.tail = 0;
    
    if (device->config->irq_mode == INT_ENABLE)
        irq_regs->MASK |= rx_mask;
    else
        irq_regs->MASK &= ~rx_mask;
        
    return UART_OK;
}

static uart_status_t driver_tx_busy(uart_dev_t *dev)
{
    uart_hfrisc_irq_regs_t *irq_regs = get_irq_regs(dev);
    uart_hfrisc_dev_t *device = dev_adapter(dev);
    uint32_t tx_mask = STATUS_TXBUSY << (device->config->port << 1);
    
    if (irq_regs->CAUSE & tx_mask)
            return UART_TX_BUSY;

    return UART_OK;
}

static uart_status_t driver_rx_data(uart_dev_t *dev)
{
    uart_hfrisc_irq_regs_t *irq_regs = get_irq_regs(dev);
    uart_hfrisc_dev_t *device = dev_adapter(dev);
    uint32_t rx_mask = STATUS_RXDATA << (device->config->port << 1);
    
    if (device->config->irq_mode == INT_ENABLE) {
        if (!fifo_empty(&device->rx_fifo))
            return UART_RX_DATA;
    } else {
        if (irq_regs->CAUSE & rx_mask)
            return UART_RX_DATA;
    }

    return UART_OK;
}

static uart_status_t driver_tx(uart_dev_t *dev, char ch)
{
    uart_hfrisc_regs_t *regs = get_regs(dev);
    uart_hfrisc_irq_regs_t *irq_regs = get_irq_regs(dev);
    uart_hfrisc_dev_t *device = dev_adapter(dev);
    uint32_t tx_mask = STATUS_TXBUSY << (device->config->port << 1);
    
    while (irq_regs->CAUSE & tx_mask);
    regs->TXR = ch;
    
    return UART_OK;
}

static uart_status_t driver_rx(uart_dev_t *dev, char *ch)
{
    uart_hfrisc_regs_t *regs = get_regs(dev);
    uart_hfrisc_irq_regs_t *irq_regs = get_irq_regs(dev);
    uart_hfrisc_dev_t *device = dev_adapter(dev);
    uint32_t rx_mask = STATUS_RXDATA << (device->config->port << 1);
    uart_status_t err;
    
    if (device->config->irq_mode == INT_ENABLE) {
        while (fifo_empty(&device->rx_fifo));
        err = fifo_get(&device->rx_fifo, ch);
        
        if (err != UART_OK) return err;
    } else {
        while (!(irq_regs->CAUSE & rx_mask));
        *ch = regs->RXR;
    }
    
    return UART_OK;
}

/* low level driver callbacks */
const uart_ops_t uart_hfrisc_ops = {
    .init = driver_init,
    .tx_busy = driver_tx_busy,
    .rx_data = driver_rx_data,
    .tx = driver_tx,
    .rx = driver_rx,
    .irq_handler = driver_irq_handler,
};
