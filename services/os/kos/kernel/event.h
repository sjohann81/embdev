#ifndef _EVENT_H
#define _EVENT_H

/* event flag wait options */
#define EVENT_WAIT_AND  0x01  /* wait for ALL specified flags */
#define EVENT_WAIT_OR   0x02  /* wait for ANY specified flags */
#define EVENT_CLEAR     0x04  /* clear flags after wait succeeds */

/* event flag group structure */
typedef struct {
    unsigned int flags;           /* current flag values (32 bits) */
    unsigned int id;              /* event group ID */
    unsigned char in_use;
} event_group_t;

/* eask event wait information */
typedef struct {
    unsigned int event_group_id;  /* event group to wait on */
    unsigned int wait_flags;      /* flags to wait for */
    unsigned char wait_option;    /* EVENT_WAIT_AND or EVENT_WAIT_OR */
    unsigned char clear_on_exit;  /* clear flags after wait? */
} event_wait_t;

/* event control API */
int event_create(void);
int event_delete(unsigned int event_id);
int event_set(unsigned int event_id, unsigned int flags);
int event_clear(unsigned int event_id, unsigned int flags);
unsigned int event_get(unsigned int event_id);
int event_wait(unsigned int event_id, unsigned int flags, unsigned char options);


#endif
