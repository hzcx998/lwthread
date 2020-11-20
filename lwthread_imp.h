#ifndef _LWTHREAD_IMP_H
#define _LWTHREAD_IMP_H

#include "lwthread.h"

/* config memory management with malloc & free */
#define LWTHREAD_CONFIG_MALLOC

/* config heap memory management */
#define LWTHREAD_CONFIG_HEAPMEM

#define LWTHREAD_TIMESLICE_DEFAULT  10

typedef enum {
    LWTHREAD_READY = 0,
    LWTHREAD_RUNNING,
    LWTHREAD_BLOCK,
    LWTHREAD_EXIT,
} lwthread_state_t;

/* thread type */
typedef struct lwthread_struct {
    struct lwthread_struct *prev;
    struct lwthread_struct *next;
    lwthread_t thread_id;
    lwthread_attr_t attr;
    lwthread_rountine_t cycle;
    unsigned long timeslice;
    void *arg;
    lwthread_state_t state;
    void *exit_code;
} lwthread_struct_t;

#endif /* _LWTHREAD_IMP_H */