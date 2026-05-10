/*
 * Driver for a PCF8574-like I2C GPIO expander.
 *
 * The device exposes 8 quasi-bidirectional I/O pins accessed as a
 * single byte over I2C. There are no sub-registers: every read or
 * write transfers the full port state in one transaction.
 *
 * Pin model:
 *   Writing 1 to a pin configures it as a weak-pull-up input.
 *   Writing 0 to a pin drives it low as an output.
 *   There is no explicit direction register.
 *
 * As a consequence of the hardware model, the following HAL operations
 * are not supported and will return GPIO_ERR_INVALID_CFG:
 *   - gpio_config: pull and drive fields are ignored; the only valid
 *     direction is GPIO_INPUT (write 1) or GPIO_OUTPUT (write 0 later).
 *     Requesting GPIO_PULL_DOWN or GPIO_DRIVE_PUSHPULL returns an error.
 *   - gpio_set_irq trigger modes other than GPIO_IRQ_BOTH_EDGES and
 *     GPIO_IRQ_DISABLE: the hardware fires on any pin change only.
 *
 * The driver keeps a shadow copy of the last byte written to the device
 * for safe single-pin read-modify-write and for change detection in the
 * IRQ handler.
 *
 * Applications should:
 *   a) Include gpio_hal.h and gpio_pcf8574.h
 *   b) Implement I2C functions matching pcf8574_i2c_write_t and
 *      pcf8574_i2c_read_t
 *   c) Declare a pcf8574_config_t pointing to those functions
 *   d) Declare a pcf8574_dev_t pointing to the config
 *   e) Declare a gpio_t handle referencing pcf8574_gpio_ops and the device
 */

#ifndef GPIO_PCF8574_H
#define GPIO_PCF8574_H

#include "gpio_hal.h"

/* number of pins on this device */
#define PCF8574_PIN_COUNT   8

/*
 * I2C transfer callbacks.
 *
 * write: transmit `len` bytes from `data` to the device at `i2c_addr`.
 * read:  receive `len` bytes into `buf` from the device at `i2c_addr`.
 *        (The PCF8574 has no register address; reads begin immediately.)
 * Both return 0 on success, non-zero on error.
 * `user` is the opaque pointer from the config, forwarded as-is.
 */
typedef int (*pcf8574_i2c_write_t)(uint8_t i2c_addr,
    const uint8_t *data, uint8_t len, void *user);

typedef int (*pcf8574_i2c_read_t)(uint8_t i2c_addr,
    uint8_t *buf, uint8_t len, void *user);

/* static device configuration */
typedef struct {
    uint8_t i2c_addr;
    pcf8574_i2c_write_t i2c_write;
    pcf8574_i2c_read_t i2c_read;
    void *i2c_user;
} pcf8574_config_t;

/* per-pin callback entry */
typedef struct {
    gpio_cb_t fn;
    void *user;
} pcf8574_pin_cb_t;

/*
 * Device instance.
 *
 * `shadow` mirrors the last byte written to the device, used for
 * single-pin read-modify-write and for change detection in the
 * IRQ handler.
 */
typedef struct gpio_pcf8574_dev {
    const pcf8574_config_t *config;
    uint8_t shadow;
    pcf8574_pin_cb_t callbacks[PCF8574_PIN_COUNT];
} pcf8574_dev_t;

/* driver ops table */
extern const gpio_ops_t pcf8574_gpio_ops;

#endif /* GPIO_PCF8574_H */
