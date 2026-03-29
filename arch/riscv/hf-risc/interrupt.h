void *irq0_handler(void);
void *irq1_handler(void);
void *irq2_handler(void);
void *irq3_handler(void);
void *irq4_handler(void);
void *irq5_handler(void);
void *irq6_handler(void);
void *irq7_handler(void);

void *porta_handler(void);
void *portb_handler(void);
void *portc_handler(void);
void *portd_handler(void);

void *timer0a_handler(void);
void *timer0b_handler(void);
void *timer1ctc_handler(void);
void *timer1ocr_handler(void);
void *timer1ovf_handler(void);
void *timer2ctc_handler(void);
void *timer2ocr_handler(void);
void *timer2ovf_handler(void);
void *timer3ctc_handler(void);
void *timer3ocr_handler(void);
void *timer3ovf_handler(void);

void *uart0rx_handler(void);
void *uart0tx_handler(void);
void *uart1rx_handler(void);
void *uart1tx_handler(void);
void *uart2rx_handler(void);
void *uart2tx_handler(void);
void *uart3rx_handler(void);
void *uart3tx_handler(void);

void *spi0_handler(void);
void *spi1_handler(void);
void *spi2_handler(void);
void *spi3_handler(void);

void *i2c0_handler(void);
void *i2c1_handler(void);
void *i2c2_handler(void);
void *i2c3_handler(void);

void *adc0_handler(void);
void *adc1_handler(void);
void *adc2_handler(void);
void *adc3_handler(void);

void *dac0_handler(void);
void *dac1_handler(void);
void *dac2_handler(void);
void *dac3_handler(void);

void *dummy_handler(void);
void irq_enable(void);

// x0 (zero) - hardwired zero, never saved
// x4 (thread pointer) - reserved in HF-RISC for trap frame pointer
// x15 (a5) - reserved in HF-RISC for trap handling (scratch register)
typedef struct {
    uint32_t zero;      // x0
    uint32_t ra;        // x1
    uint32_t sp;        // x2
    uint32_t gp;        // x3
    uint32_t tp;        // x4
    uint32_t t0;        // x5
    uint32_t t1;        // x6
    uint32_t t2;        // x7
    uint32_t s0;        // x8  (fp)
    uint32_t s1;        // x9
    uint32_t a0;        // x10
    uint32_t a1;        // x11
    uint32_t a2;        // x12
    uint32_t a3;        // x13
    uint32_t a4;        // x14
    uint32_t a5;        // x15
    uint32_t epc;
} trap_frame_t;

trap_frame_t *trap_handler(uint32_t cause);

void syscall(void *arg0, ...);
