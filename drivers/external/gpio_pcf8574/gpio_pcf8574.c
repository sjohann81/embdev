#include <stddef.h>
#include "gpio_hal.h"
#include "gpio_pcf8574.h"

/* --------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------- */

static inline pcf8574_dev_t *dev_adapter(gpio_dev_t *dev)
{
    return (pcf8574_dev_t *)dev;
}

static gpio_status_t port_write(pcf8574_dev_t *device, uint8_t val)
{
    if (device->config->i2c_write(device->config->i2c_addr, &val, 1,
                             device->config->i2c_user) != 0)
        return GPIO_ERR_IO;

    device->shadow = val;

    return GPIO_OK;
}

static gpio_status_t port_read(pcf8574_dev_t *device, uint8_t *val)
{
    if (device->config->i2c_read(device->config->i2c_addr, val, 1,
                            device->config->i2c_user) != 0)
        return GPIO_ERR_IO;

    return GPIO_OK;
}

static gpio_status_t check_pin(uint8_t pin)
{
    if (pin >= PCF8574_PIN_COUNT)
        return GPIO_ERR_INVALID_PIN;

    return GPIO_OK;
}


/* --------------------------------------------------------------------
 * Driver API implementation
 * -------------------------------------------------------------------- */

static gpio_status_t driver_init(gpio_dev_t *dev)
{
    pcf8574_dev_t *device = dev_adapter(dev);
    gpio_status_t err;
    uint8_t i;

    /*
     * Drive all pins high: this is the correct reset state for the
     * PCF8574 and makes every pin a weak-pull-up input, ready to be
     * driven low individually when used as outputs.
     */
    err = port_write(device, 0xFF);
    if (err != GPIO_OK)
        return err;

    for (i = 0; i < PCF8574_PIN_COUNT; i++) {
        device->callbacks[i].fn   = NULL;
        device->callbacks[i].user = NULL;
    }

    return GPIO_OK;
}

static gpio_status_t driver_config(gpio_dev_t *dev, uint8_t pin,
                                   const gpio_config_t *cfg)
{
    pcf8574_dev_t *device = dev_adapter(dev);
    gpio_status_t err;
    uint8_t bit = (uint8_t)(1u << pin);

    err = check_pin(pin);
    if (err != GPIO_OK)
        return err;

    /*
     * Pull-down resistors and push-pull drive do not exist on this
     * device. The only valid combination for an input is PULL_UP
     * (or PULL_NONE, treated identically) with DRIVE_OPENDRAIN, which
     * is the hardware's native mode.
     */
    if (cfg->pull == GPIO_PULL_DOWN || cfg->drive == GPIO_DRIVE_PUSHPULL)
        return GPIO_ERR_INVALID_CFG;

    /*
     * To configure a pin as input: write 1 (enables weak pull-up).
     * To configure a pin as output: write 1 here too -- the caller
     * will drive it low with gpio_write() when needed.
     * Either way the pin starts high, which is the safe idle state.
     */
    return port_write(device, (uint8_t)(device->shadow | bit));
}

static gpio_status_t driver_config_port(gpio_dev_t *dev, uint32_t mask,
                                        const gpio_config_t *cfg)
{
    pcf8574_dev_t *device = dev_adapter(dev);

    /* apply the same constraints as single-pin config */
    if (cfg->pull == GPIO_PULL_DOWN || cfg->drive == GPIO_DRIVE_PUSHPULL)
        return GPIO_ERR_INVALID_CFG;

    /* set all masked pins high (input/idle state) */
    return port_write(device, (uint8_t)(device->shadow | (uint8_t)mask));
}

static gpio_status_t driver_write(gpio_dev_t *dev, uint8_t pin,
                                  gpio_val_t val)
{
    pcf8574_dev_t *device = dev_adapter(dev);
    gpio_status_t err;
    uint8_t bit = (uint8_t)(1u << pin);

    err = check_pin(pin);
    if (err != GPIO_OK)
        return err;

    if (val == GPIO_HIGH)
        return port_write(device, (uint8_t)(device->shadow |  bit));
    else
        return port_write(device, (uint8_t)(device->shadow & ~bit));
}

static gpio_status_t driver_read(gpio_dev_t *dev, uint8_t pin,
                                 gpio_val_t *val)
{
    pcf8574_dev_t *device = dev_adapter(dev);
    gpio_status_t err;
    uint8_t raw;

    err = check_pin(pin);
    if (err != GPIO_OK)
        return err;

    err = port_read(device, &raw);
    if (err != GPIO_OK)
        return err;

    *val = (raw >> pin) & 1u ? GPIO_HIGH : GPIO_LOW;

    return GPIO_OK;
}

static gpio_status_t driver_toggle(gpio_dev_t *dev, uint8_t pin)
{
    pcf8574_dev_t *device = dev_adapter(dev);
    gpio_status_t err;

    err = check_pin(pin);
    if (err != GPIO_OK)
        return err;

    return port_write(device, (uint8_t)(device->shadow ^ (1u << pin)));
}

static gpio_status_t driver_write_port(gpio_dev_t *dev, uint32_t mask,
                                       uint32_t vals)
{
    pcf8574_dev_t *device = dev_adapter(dev);
    uint8_t m = (uint8_t)mask;

    return port_write(device, (uint8_t)((device->shadow & ~m) | ((uint8_t)vals & m)));
}

static gpio_status_t driver_read_port(gpio_dev_t *dev, uint32_t *vals)
{
    pcf8574_dev_t *device = dev_adapter(dev);
    gpio_status_t err;
    uint8_t raw;

    err = port_read(device, &raw);
    if (err != GPIO_OK)
        return err;

    *vals = raw;

    return GPIO_OK;
}

static gpio_status_t driver_toggle_port(gpio_dev_t *dev, uint32_t mask)
{
    pcf8574_dev_t *device = dev_adapter(dev);

    return port_write(device, (uint8_t)(device->shadow ^ (uint8_t)mask));
}

static gpio_status_t driver_irq_attach(gpio_dev_t *dev, uint8_t pin,
                                       gpio_irq_trigger_t trig,
                                       gpio_cb_t callback, void *user)
{
    pcf8574_dev_t *device = dev_adapter(dev);
    gpio_status_t err;

    err = check_pin(pin);
    if (err != GPIO_OK)
        return err;

    /*
     * The PCF8574 fires its INT line on any pin change; there is no
     * hardware support for selecting a specific edge or level.
     * Only BOTH_EDGES (any change) and DISABLE are accepted.
     */
    if (trig != GPIO_IRQ_BOTH_EDGES && trig != GPIO_IRQ_DISABLE)
        return GPIO_ERR_INVALID_CFG;

    device->callbacks[pin].fn   = (trig == GPIO_IRQ_DISABLE) ? NULL : callback;
    device->callbacks[pin].user = (trig == GPIO_IRQ_DISABLE) ? NULL : user;

    return GPIO_OK;
}

static gpio_status_t driver_irq_handler(gpio_dev_t *dev)
{
    pcf8574_dev_t *device = dev_adapter(dev);
    gpio_status_t err;
    uint8_t current, changed;
    uint8_t pin;

    /*
     * Read the current pin state and compare against the shadow to
     * find which pins changed. The shadow reflects what was last
     * written; for input pins it also represents their last known
     * state since we wrote 1 to enable them.
     */
    err = port_read(device, &current);
    if (err != GPIO_OK)
        return err;

    changed = current ^ device->shadow;

    /* update the shadow to the current state before firing callbacks */
    device->shadow = current;

    pin = 1u;
    while (changed) {
        if (device->callbacks[pin].fn != NULL)
            device->callbacks[pin].fn(pin, device->callbacks[pin].user);
        pin <<= 1u;
        changed >>= 1u;
    }

    return GPIO_OK;
}


/* --------------------------------------------------------------------
 * Driver ops table
 * -------------------------------------------------------------------- */

const gpio_ops_t pcf8574_gpio_ops = {
    .init        = driver_init,
    .config      = driver_config,
    .config_port = driver_config_port,
    .write       = driver_write,
    .read        = driver_read,
    .toggle      = driver_toggle,
    .write_port  = driver_write_port,
    .read_port   = driver_read_port,
    .toggle_port = driver_toggle_port,
    .irq_attach  = driver_irq_attach,
    .irq_handler = driver_irq_handler,
};
