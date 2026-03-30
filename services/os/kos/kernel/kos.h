#ifndef _KOS_H
#define _KOS_H

#include <stdint.h>
#include "port.h"
#include "bsp.h"
#include "event.h"
#include "task.h"
#include "config.h"

typedef enum {
    SYS_OK = 0,
    SYS_ERR = -1,
} sys_err_t;

/* kernel control API */
void sys_init(void);
void sys_start(void);
unsigned int sys_ticks(void);

/* internal scheduler function */
task_t *sys_schedule(void);

/* application defined hook functions (called from kernel) */
void init_hook(void);
void tick_hook(void);
void idle_hook(void);
void stackoverflow_hook(void);
void exception_hook(void);

#endif
