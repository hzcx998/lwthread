#include "lwthread.h"
#include "lwthread_imp.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

static lwthread_struct_t __lwthread_list_head = {&__lwthread_list_head, &__lwthread_list_head, };
static lwthread_struct_t *__lwthread_current = NULL;
static lwthread_attr_t __lwthread_attr_default = {NULL, NULL, 0};
volatile static lwthread_t __lwthread_next_free_id = 0;

int lwthread_attr_init(lwthread_attr_t *attr)
{
    if (!attr)
        return -EINVAL;
    attr->init = NULL;
    attr->context_addr = NULL;
    attr->context_size = 0;
    return 0;
}

int lwthread_attr_destroy(lwthread_attr_t *attr)
{
    if (!attr)
        return -EINVAL;
    if (attr->context_addr)
        free(attr->context_addr);
    attr->init = NULL;
    attr->context_size = 0;
    return 0;
}

int lwthread_attr_set_context(
    lwthread_attr_t *attr,
    lwthread_rountine_t init_rountine,
    unsigned long size)
{
    if (!attr)
        return -EINVAL;
    attr->init = init_rountine;
    attr->context_addr = malloc(size);
    if (!attr->context_addr)
        return -ENOMEM;
    attr->context_size = size;
    return 0;
}

void lwthread_list_insert(lwthread_struct_t *thread)
{
    /* insert at head of the list */
    lwthread_struct_t *head = &__lwthread_list_head;
    lwthread_struct_t *next = head->next;
    next->prev = thread;
    thread->next = next;
    thread->prev = head;
    head->next = thread;
}

void lwthread_list_remove(lwthread_struct_t *thread)
{
    lwthread_struct_t *prev = thread->prev;
    lwthread_struct_t *next = thread->next;
    next->prev = prev;
    prev->next = next;
    thread->prev = NULL;
    thread->next = NULL;
}

int lwthread_create(lwthread_t *thread,
        lwthread_attr_t *attr,
        lwthread_rountine_t cycle_rountine,
        void *arg)
{
    if (!cycle_rountine)
        return -EINVAL;
    lwthread_struct_t *thread_ptr = malloc(sizeof(lwthread_struct_t));
    if (!thread_ptr)
        return -ENOMEM;
    if (!attr) {
        thread_ptr->attr = __lwthread_attr_default; /* use default attr */
    } else {
        thread_ptr->attr = *attr;
        if (!thread_ptr->attr.context_addr && thread_ptr->attr.context_size > 0) {
            thread_ptr->attr.context_addr = malloc(thread_ptr->attr.context_size);
            if (!thread_ptr->attr.context_addr) {
                free(thread_ptr);
                return -ENOMEM;
            }
        }
    }
    thread_ptr->thread_id = __lwthread_next_free_id++;
    thread_ptr->cycle = cycle_rountine;
    thread_ptr->arg = arg;
    thread_ptr->timeslice = LWTHREAD_TIMESLICE_DEFAULT;
    thread_ptr->state = LWTHREAD_READY;
    if (thread_ptr->attr.init) { /* execute init */
        void *retval = thread_ptr->attr.init(thread_ptr->attr.context_addr, thread_ptr->arg);
        if (retval != LWTHREAD_OK) {
            if (thread_ptr->attr.context_addr)
                free(thread_ptr->attr.context_addr);
            free(thread_ptr);
            return (int) retval;
        }
    }
    if (thread)
        *thread = thread_ptr->thread_id;
    /* add to thread list. */
    lwthread_list_insert(thread_ptr);
    return 0;
}

lwthread_t lwthread_self(void)
{
    return __lwthread_current->thread_id;
}

void lwthread_set_exitcode(void *retval)
{
    lwthread_struct_t *thread = __lwthread_current;
    thread->exit_code = retval;
}

static void __lwthread_exit(lwthread_struct_t *thread, void *retval)
{
    thread->state = LWTHREAD_EXIT;
    thread->exit_code = retval;
}

void lwthread_exit(void *retval)
{
    __lwthread_exit(__lwthread_current, retval);
}

static void __lwthread_destroy(lwthread_struct_t *thread)
{
    printf("thread %d exit with %x\n", thread->thread_id, thread->exit_code);
    lwthread_list_remove(thread);
    if (thread->attr.context_addr)
        free(thread->attr.context_addr);
    free(thread);
}

void lwthread_schedule()
{
    lwthread_struct_t *thread;
    lwthread_struct_t *next;
    
    /* get first sched thread */
    thread = __lwthread_list_head.next;
    thread->state = LWTHREAD_RUNNING;

    unsigned long timeslice;
    char exit_flags;
    while (thread != &__lwthread_list_head) {
        exit_flags = 0;
        __lwthread_current = thread;
        if (thread->cycle) {
            timeslice = thread->timeslice;
            while (timeslice > 0) {
                if (thread->cycle(thread->attr.context_addr, thread->arg) == LWTHREAD_ERR) {
                    __lwthread_exit(thread, thread->exit_code); /* exit thread */
                }
                if (thread->state == LWTHREAD_EXIT) {
                    exit_flags = 1;
                    break;
                }    
                --timeslice;
            }
        }
        /* exit check */
        if (exit_flags) {
            next = thread->next; /* backup next thread */        
            __lwthread_destroy(thread);
            thread = next; /* select next thread */
        } else {
            thread->state = LWTHREAD_READY;
            thread = thread->next;
        }
        /* skip list head */
        if (thread == &__lwthread_list_head) {
            thread = thread->next;
        }
        thread->state = LWTHREAD_RUNNING;
    }
}
