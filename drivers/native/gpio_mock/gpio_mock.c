#include <stddef.h>
#include "gpio_hal.h"
#include "gpio_mock.h"

/* --------------------------------------------------------------------
 * Register layout
 *
 * All registers are 32-bit, one bit per pin.
 * -------------------------------------------------------------------- */

typedef struct {
    volatile uint32_t DATA;         /* output latch (write) / pin state (read) */
    volatile uint32_t DIR;          /* direction: 1 = output, 0 = input        */
    volatile uint32_t PULL_UP;      /* pull-up enable                          */
    volatile uint32_t PULL_DN;      /* pull-down enable                        */
    volatile uint32_t OPENDRAIN;    /* open-drain enable                       */
    volatile uint32_t IRQ_RISE;     /* rising-edge IRQ enable                  */
    volatile uint32_t IRQ_FALL;     /* falling-edge IRQ enable                 */
    volatile uint32_t IRQ_STAT;     /* IRQ status: set by HW, cleared by w1c   */
} gpio_mock_regs_t;


/* --------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------- */

static inline gpio_mock_dev_t *dev_adapter(gpio_dev_t *dev)
{
    return (gpio_mock_dev_t *)dev;
}

static inline gpio_mock_regs_t *get_regs(gpio_dev_t *dev)
{
    return (gpio_mock_regs_t *)dev_adapter(dev)->config->base_addr;
}

static gpio_status_t check_pin(gpio_dev_t *dev, uint8_t pin)
{
    if (pin >= dev_adapter(dev)->config->pin_count)
        return GPIO_ERR_INVALID_PIN;

    return GPIO_OK;
}

/* apply a gpio_config_t to a single bitmask within the register set */
static gpio_status_t apply_config(gpio_mock_regs_t *regs,
    uint32_t mask, const gpio_config_t *cfg)
{
    if (cfg->dir == GPIO_OUTPUT)
        regs->DIR |=  mask;
    else
        regs->DIR &= ~mask;

    /* pull resistors are mutually exclusive */
    regs->PULL_UP &= ~mask;
    regs->PULL_DN &= ~mask;

    if (cfg->pull == GPIO_PULL_UP)
        regs->PULL_UP |= mask;
    else if (cfg->pull == GPIO_PULL_DOWN)
        regs->PULL_DN |= mask;

    if (cfg->drive == GPIO_DRIVE_OPENDRAIN)
        regs->OPENDRAIN |= mask;
    else
        regs->OPENDRAIN &= ~mask;

    return GPIO_OK;
}


/* --------------------------------------------------------------------
 * Driver API implementation
 * -------------------------------------------------------------------- */

static gpio_status_t driver_init(gpio_dev_t *dev)
{
    gpio_mock_regs_t *regs = get_regs(dev);
    gpio_mock_dev_t *driver = dev_adapter(dev);
    uint8_t i;

    regs->DATA      = 0x00000000;
    regs->DIR       = 0x00000000;
    regs->PULL_UP   = 0x00000000;
    regs->PULL_DN   = 0x00000000;
    regs->OPENDRAIN = 0x00000000;
    regs->IRQ_RISE  = 0x00000000;
    regs->IRQ_FALL  = 0x00000000;
    regs->IRQ_STAT  = 0xFFFFFFFF;   /* clear any pending flags */

    for (i = 0; i < GPIO_MOCK_PIN_COUNT; i++) {
        driver->callbacks[i].fn = NULL;
        driver->callbacks[i].user = NULL;
    }

    return GPIO_OK;
}

static gpio_status_t driver_config(gpio_dev_t *dev, uint8_t pin,
    const gpio_config_t *cfg)
{
    gpio_status_t err = check_pin(dev, pin);

    if (err != GPIO_OK)
        return err;

    return apply_config(get_regs(dev), 1u << pin, cfg);
}

static gpio_status_t driver_config_port(gpio_dev_t *dev, uint32_t mask,
    const gpio_config_t *cfg)
{
    return apply_config(get_regs(dev), mask, cfg);
}

static gpio_status_t driver_write(gpio_dev_t *dev, uint8_t pin, gpio_val_t val)
{
    gpio_mock_regs_t *regs = get_regs(dev);
    gpio_status_t err = check_pin(dev, pin);

    if (err != GPIO_OK)
        return err;

    if (val == GPIO_HIGH)
        regs->DATA |= (1u << pin);
    else
        regs->DATA &= ~(1u << pin);

    return GPIO_OK;
}

static gpio_status_t driver_read(gpio_dev_t *dev, uint8_t pin, gpio_val_t *val)
{
    gpio_status_t err = check_pin(dev, pin);

    if (err != GPIO_OK)
        return err;

    *val = (get_regs(dev)->DATA >> pin) & 1u ? GPIO_HIGH : GPIO_LOW;

    return GPIO_OK;
}

static gpio_status_t driver_toggle(gpio_dev_t *dev, uint8_t pin)
{
    gpio_status_t err = check_pin(dev, pin);

    if (err != GPIO_OK)
        return err;

    get_regs(dev)->DATA ^= (1u << pin);

    return GPIO_OK;
}

static gpio_status_t driver_write_port(gpio_dev_t *dev, uint32_t mask, uint32_t vals)
{
    gpio_mock_regs_t *regs = get_regs(dev);

    regs->DATA = (regs->DATA & ~mask) | (vals & mask);

    return GPIO_OK;
}

static gpio_status_t driver_read_port(gpio_dev_t *dev, uint32_t *vals)
{
    *vals = get_regs(dev)->DATA;

    return GPIO_OK;
}

static gpio_status_t driver_toggle_port(gpio_dev_t *dev, uint32_t mask)
{
    get_regs(dev)->DATA ^= mask;

    return GPIO_OK;
}

static gpio_status_t driver_irq_attach(gpio_dev_t *dev, uint8_t pin,
    gpio_irq_trigger_t trig, gpio_cb_t callback, void *user)
{
    gpio_mock_regs_t *regs = get_regs(dev);
    gpio_mock_dev_t *driver = dev_adapter(dev);
    gpio_status_t err;
    uint32_t bit = 1u << pin;

    err = check_pin(dev, pin);
    if (err != GPIO_OK)
        return err;

    if (trig == GPIO_IRQ_HIGH_LEVEL || trig == GPIO_IRQ_LOW_LEVEL)
        return GPIO_ERR_INVALID_CFG;

    /* store (or clear) the callback */
    driver->callbacks[pin].fn = callback;
    driver->callbacks[pin].user = user;

    /* configure the trigger edges */
    regs->IRQ_RISE &= ~bit;
    regs->IRQ_FALL &= ~bit;

    if (trig == GPIO_IRQ_RISING_EDGE  || trig == GPIO_IRQ_BOTH_EDGES)
        regs->IRQ_RISE |= bit;
    if (trig == GPIO_IRQ_FALLING_EDGE || trig == GPIO_IRQ_BOTH_EDGES)
        regs->IRQ_FALL |= bit;

    return GPIO_OK;
}

static gpio_status_t driver_irq_handler(gpio_dev_t *dev)
{
    gpio_mock_regs_t *regs = get_regs(dev);
    gpio_mock_dev_t *driver = dev_adapter(dev);
    uint32_t pending = regs->IRQ_STAT;
    uint8_t pin;

    pin = 1u;
    while (pending) {
        regs->IRQ_STAT = pin;       /* w1c: acknowledge this pin */

        if (driver->callbacks[pin].fn != NULL)
            driver->callbacks[pin].fn(pin, driver->callbacks[pin].user);
        pin <<= 1u;
        pending >>= 1u;
    }

    return GPIO_OK;
}


/* --------------------------------------------------------------------
 * Driver ops table
 * -------------------------------------------------------------------- */

const gpio_ops_t mock_gpio_ops = {
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
