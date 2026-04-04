#ifndef _PORT_PORT_H
#define _PORT_PORT_H

#include <stdint.h>
#include <interrupt.h>

void port_di(void);
void port_ei(void);
void port_init_context(void *tcb, void *(*task_ptr)(void *),
	void *arg, unsigned long sp);
void port_yield(void);
void port_halt(void);

#endif
