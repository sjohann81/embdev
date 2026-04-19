#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include <stdint.h>

/* the abstract GPIO device type */
typedef void gpio_dev_t;

/* pin direction */
typedef enum {
    GPIO_INPUT     = 0,
    GPIO_OUTPUT    = 1,
} gpio_dir_t;

/* pin logic level */
typedef enum {
    GPIO_LOW  = 0,
    GPIO_HIGH = 1,
} gpio_val_t;

/* pull resistor configuration */
typedef enum {
    GPIO_PULL_NONE = 0,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN,
} gpio_pull_t;

/* output drive configuration */
typedef enum {
    GPIO_DRIVE_PUSHPULL  = 0,
    GPIO_DRIVE_OPENDRAIN = 1,
} gpio_drive_t;

/* pin configuration */
typedef struct {
    gpio_dir_t dir;
    gpio_pull_t pull;
    gpio_drive_t drive;
} gpio_config_t;

/* interrupt trigger mode */
typedef enum {
    GPIO_IRQ_DISABLE        = 0,
    GPIO_IRQ_RISING_EDGE,
    GPIO_IRQ_FALLING_EDGE,
    GPIO_IRQ_BOTH_EDGES,
    GPIO_IRQ_HIGH_LEVEL,
    GPIO_IRQ_LOW_LEVEL,
} gpio_irq_trigger_t;

/* user callback type: called from the IRQ handler for each fired pin */
typedef void (*gpio_cb_t)(uint8_t pin, void *user);

/* status / error codes */
typedef enum {
    GPIO_OK = 0,
    GPIO_ERR_INVALID_PORT,
    GPIO_ERR_INVALID_PIN,
    GPIO_ERR_IO,
} gpio_status_t;

/* low level driver callbacks */
typedef struct {
    gpio_status_t (*init)       (gpio_dev_t *dev);
    /* single pin operations */
    gpio_status_t (*config)     (gpio_dev_t *dev, uint8_t pin, const gpio_config_t *cfg);
    gpio_status_t (*write)      (gpio_dev_t *dev, uint8_t pin, gpio_val_t val);
    gpio_status_t (*read)       (gpio_dev_t *dev, uint8_t pin, gpio_val_t *val);
    gpio_status_t (*toggle)     (gpio_dev_t *dev, uint8_t pin);
    /* multiple pin operations */
    gpio_status_t (*config_port)(gpio_dev_t *dev, uint32_t mask, const gpio_config_t *cfg);
    gpio_status_t (*write_port) (gpio_dev_t *dev, uint32_t mask, uint32_t vals);
    gpio_status_t (*read_port)  (gpio_dev_t *dev, uint32_t *vals);
    gpio_status_t (*toggle_port)(gpio_dev_t *dev, uint32_t mask);
    /* interrupt operations */
    gpio_status_t (*irq_attach) (gpio_dev_t *dev, uint8_t pin,
        gpio_irq_trigger_t trig, gpio_cb_t *callback, void *user);
    gpio_status_t (*irq_handler)(gpio_dev_t *dev);
} gpio_ops_t;

/* abstract GPIO port handle */
typedef struct {
    const gpio_ops_t *ops;
    gpio_dev_t *dev;
} gpio_t;

/* HAL functions */
static inline gpio_status_t gpio_config(const gpio_t *g, uint8_t pin, const gpio_config_t *cfg)
{
    return g->ops->config(g->dev, pin, cfg);
}

static inline gpio_status_t gpio_write(const gpio_t *g, uint8_t pin, gpio_val_t val)
{
    return g->ops->write(g->dev, pin, val);
}

static inline gpio_status_t gpio_read(const gpio_t *g, uint8_t pin, gpio_val_t *val)
{
    return g->ops->read(g->dev, pin, val);
}

static inline gpio_status_t gpio_toggle(const gpio_t *g, uint8_t pin)
{
    return g->ops->toggle(g->dev, pin);
}

static inline gpio_status_t gpio_write_port(const gpio_t *g, uint32_t mask, uint32_t vals)
{
    return g->ops->write_port(g->dev, mask, vals);
}

static inline gpio_status_t gpio_read_port(const gpio_t *g, uint32_t *vals)
{
    return g->ops->read_port(g->dev, vals);
}

static inline gpio_status_t gpio_toggle_port(const gpio_t *g, uint32_t mask)
{
    return g->ops->write_port(g->dev, mask, vals);
}

static inline gpio_status_t gpio_irq_attach(const gpio_t *g, uint8_t pin,
    gpio_irq_trigger_t trig, gpio_cb_t callback, void *user)
{
    return g->ops->irq_attach(g->dev, pin, trig, callback, user);
}

static inline gpio_status_t gpio_irq_handle(const gpio_t *g)
{
    return g->ops->irq_handler(g->dev);
}

#endif /* GPIO_HAL_H */
