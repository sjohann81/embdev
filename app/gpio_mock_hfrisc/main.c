/*
 * Example: mock GPIO driver and external driver used via the GPIO HAL.
 *
 * Demonstrates pin direction, pull config, single-pin and port-wide
 * read/write, toggle, and interrupt-driven input with a callback.
 */

#include <stdio.h>
#include "gpio_hal.h"
#include "gpio_mock.h"
#include "gpio_pcf8574.h"

/* port A */
static const gpio_mock_config_t porta_cfg = {
    .base_addr = 0x50000000,
    .pin_count = 16,
};

static gpio_mock_dev_t porta_dev = {
    .config = &porta_cfg,
};

const gpio_t porta = {
    .ops = &mock_gpio_ops,
    .dev = &porta_dev,
};

/* port B */
static const gpio_mock_config_t portb_cfg = {
    .base_addr = 0x50001000,
    .pin_count = 8,
};

static gpio_mock_dev_t portb_dev = {
    .config = &portb_cfg,
};

const gpio_t portb = {
    .ops = &mock_gpio_ops,
    .dev = &portb_dev,
};


/* port C - external I2C controller */
static int i2c_write_func(uint8_t i2c_addr, const uint8_t *data, uint8_t len, void *user)
{
    printf("I2C write: addr %02x, len: %s\n");
    return 0;
}

static int i2c_read_func(uint8_t i2c_addr, uint8_t *buf, uint8_t len, void *user)
{
    printf("I2C read: addr %02x, len: %s\n");
    return 0;
}


static const pcf8574_config_t portc_cfg = {
    .i2c_addr = 0x20,
    .i2c_write = i2c_write_func,
    .i2c_read = i2c_read_func,
};

static pcf8574_dev_t portc_dev = {
    .config = &portc_cfg,
};

const gpio_t portc = {
    .ops = &pcf8574_gpio_ops,
    .dev = &portc_dev,
};

#define LED_PIN     3
#define BUTTON_PIN  0

static void button_pressed(uint8_t pin, void *user)
{
    const gpio_t *leds = (const gpio_t *)user;

    (void)pin;

    /* toggle the LED on every button press */
    gpio_toggle(leds, LED_PIN);
}

int main(void)
{
    gpio_init(&porta);
    gpio_init(&portb);

    /* configure LED pin as output, initially low */
    gpio_config(&porta, LED_PIN, &(gpio_config_t) {
        .dir = GPIO_OUTPUT,
    });
    gpio_write(&porta, LED_PIN, GPIO_LOW);

    /* configure button pin as input with pull-up, fire on falling edge */
    gpio_config(&portb, BUTTON_PIN, &(gpio_config_t) {
        .dir = GPIO_INPUT,
        .pull = GPIO_PULL_UP,
    });
    
    /* pass port A ref to the handler, so it can use it */
    gpio_irq_attach(&portb, BUTTON_PIN, GPIO_IRQ_FALLING_EDGE,
        button_pressed, (void *)&porta);

    /* write all 16 port A pins at once: set even pins high, odd pins low */
    gpio_config_port(&porta, 0xffff, &(gpio_config_t) {
        .dir = GPIO_OUTPUT,
    });
    gpio_write_port(&porta, 0xffff, 0x5555);

    /* read back the whole port */
    uint32_t state;
    gpio_read_port(&porta, &state);
    
    /* configure pin on external GPIO as output */
    gpio_config(&portc, 3, &(gpio_config_t) {
        .dir = GPIO_OUTPUT,
    });
    gpio_write(&portc, 3, GPIO_LOW);

    return 0;
}
