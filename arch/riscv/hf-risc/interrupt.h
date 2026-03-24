void irq0_handler(void);
void irq1_handler(void);
void irq2_handler(void);
void irq3_handler(void);
void irq4_handler(void);
void irq5_handler(void);
void irq6_handler(void);
void irq7_handler(void);

void porta_handler(void);
void portb_handler(void);
void portc_handler(void);
void portd_handler(void);

void timer0a_handler(void);
void timer0b_handler(void);
void timer1ctc_handler(void);
void timer1ocr_handler(void);
void timer1ovf_handler(void);
void timer2ctc_handler(void);
void timer2ocr_handler(void);
void timer2ovf_handler(void);
void timer3ctc_handler(void);
void timer3ocr_handler(void);
void timer3ovf_handler(void);
void timer4ctc_handler(void);
void timer4ocr_handler(void);
void timer4ovf_handler(void);
void timer5ctc_handler(void);
void timer5ocr_handler(void);
void timer5ovf_handler(void);
void timer6ctc_handler(void);
void timer6ocr_handler(void);
void timer6ovf_handler(void);
void timer7ctc_handler(void);
void timer7ocr_handler(void);
void timer7ovf_handler(void);

void uart0rx_handler(void);
void uart0tx_handler(void);
void uart1rx_handler(void);
void uart1tx_handler(void);
void uart2rx_handler(void);
void uart2tx_handler(void);
void uart3rx_handler(void);
void uart3tx_handler(void);
void uart4rx_handler(void);
void uart4tx_handler(void);
void uart5rx_handler(void);
void uart5tx_handler(void);
void uart6rx_handler(void);
void uart6tx_handler(void);
void uart7rx_handler(void);
void uart7tx_handler(void);

void spi0_handler(void);
void spi1_handler(void);
void spi2_handler(void);
void spi3_handler(void);

void i2c0_handler(void);
void i2c1_handler(void);
void i2c2_handler(void);
void i2c3_handler(void);

void adc0_handler(void);
void adc1_handler(void);
void adc2_handler(void);
void adc3_handler(void);

void dac0_handler(void);
void dac1_handler(void);
void dac2_handler(void);
void dac3_handler(void);

void dummy_handler(void);
void irq_enable(void);
void irq_handler(uint32_t cause, uint32_t *stack);
