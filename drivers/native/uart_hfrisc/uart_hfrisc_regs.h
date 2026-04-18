/* 
 * These are HF-RISC register layout data structures. Only the
 * driver (and not the application or HAL) should see this. The
 * registers for UART interrupt control are not available along with
 * other peripheral registers, but in a separate address space, so
 * we define a separate structure to describe them.
 */

#ifndef UART_HFRISC_REGS_H
#define UART_HFRISC_REGS_H

/* HF-RISC UART registers */
typedef struct {
    union {
        volatile uint32_t RXR;          // receive register
        volatile uint32_t TXR;          // transmit register
    };
    volatile const uint32_t _reserved[3];
    volatile uint32_t DIV;              // baud rate divisor
} uart_hfrisc_regs_t;

typedef struct {
    volatile const uint32_t _reserved0[0x100];
    volatile uint32_t CAUSE;
    volatile const uint32_t _reserved1[0x0ff];
    volatile uint32_t CAUSEINV;
    volatile const uint32_t _reserved2[0x0ff];
    volatile uint32_t MASK;
} uart_hfrisc_irq_regs_t;

/* bit masks for UARTCAUSE, UARTCAUSE_INV and UARTMASK */
#define STATUS_RXDATA		    (1 << 0)
#define STATUS_TXBUSY		    (1 << 1)

#endif
