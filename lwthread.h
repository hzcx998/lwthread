#ifndef _LWTHREAD_H
#define _LWTHREAD_H

/* Light weight thread */

#define LWTHREAD_ERR        ((void *) -1)
#define LWTHREAD_OK         ((void *) 0)

typedef unsigned long lwthread_t;
typedef void *(*lwthread_rountine_t)(void *, void *);

typedef struct {
    lwthread_rountine_t init;
    void *              context_addr;
    unsigned long       context_size;
} lwthread_attr_t;

int lwthread_attr_init(lwthread_attr_t *attr);
int lwthread_attr_set_context(
    lwthread_attr_t *attr,
    lwthread_rountine_t init_rountine,
    unsigned long size);
int lwthread_attr_destroy(lwthread_attr_t *attr);

int lwthread_create(lwthread_t *thread,
        lwthread_attr_t *attr,
        lwthread_rountine_t cycle_rountine,
        void *arg);
void lwthread_exit(void *retval);
lwthread_t lwthread_self(void);
void lwthread_set_exitcode(void *retval);

void lwthread_schedule();

#endif /* _LWTHREAD_H */