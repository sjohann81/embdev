/*
 * Mock driver for a fictitious native (memory-mapped) GPIO port.
 *
 * Each port supports up to 32 pins. All HAL features are supported:
 * direction, pull-up/down, open-drain drive, port-wide operations,
 * and edge-triggered interrupts. Callbacks are registered directly
 * through gpio_irq_attach(), with no separate registration step.
 *
 * Applications should:
 *   a) Include gpio_hal.h and gpio_mock.h
 *   b) Declare a gpio_mock_config_t and fill in its fields
 *   c) Declare a gpio_mock_dev_t pointing to the config
 *   d) Declare a gpio_t handle referencing mock_gpio_ops
 *      and the device
 */

#ifndef GPIO_MOCK_H
#define GPIO_MOCK_H

#include "gpio_hal.h"

/* maximum pins per port */
#define GPIO_MOCK_PIN_COUNT  32

/* per-pin callback entry */
typedef struct {
    gpio_cb_t fn;
    void *user;
} gpio_mock_pin_cb_t;

/* static device configuration */
typedef struct {
    uintptr_t base_addr;    /* base address of the GPIO peripheral */
    uint8_t pin_count;    /* number of usable pins (<= 32)       */
} gpio_mock_config_t;

/* device instance */
typedef struct gpio_mock_dev {
    const gpio_mock_config_t *config;
    gpio_mock_pin_cb_t callbacks[GPIO_MOCK_PIN_COUNT];
} gpio_mock_dev_t;

/* driver ops table */
extern const gpio_ops_t mock_gpio_ops;

#endif /* GPIO_MOCK_H */
