#include <stdio.h>
#include "lwthread.h"

typedef struct {
    int count;
} thread_context_t;

void *thread_init(void *ext, void *arg)
{
    thread_context_t *context = (thread_context_t *) ext;
    context->count = 0;
    return LWTHREAD_OK;
}

void *thread_cycle(void *ext, void *arg)
{
    thread_context_t *context = (thread_context_t *) ext;
    printf("context: %x\n", context);
    static int init_done = 0;
    if (!init_done) {
        if (arg == ((void*) 123)) {
            context->count = 100;
            init_done = 1;
        }
    }
    
    printf("%d %d\n", arg, context->count);
    context->count++;
    if (context->count > 200) {
        lwthread_set_exitcode((void *)123);
        return LWTHREAD_ERR;
    }
    return LWTHREAD_OK;
}

void *thread2_cycle(void *ext, void *arg)
{
    static int count = 0;
    count++;
    printf("%d\n", count);
    if (count > 1000) {
        lwthread_t thread;
        lwthread_create(&thread, NULL, thread2_cycle, (void *)NULL);
        lwthread_exit(NULL);
    }
    return LWTHREAD_OK;
}

int main(int argc, char *argv[])
{
    printf("hello, lwthread!\n");

    lwthread_t thread1;
    lwthread_attr_t attr1;
    lwthread_attr_init(&attr1);
    lwthread_attr_set_context(&attr1, NULL, sizeof(thread_context_t));
    int ret = lwthread_create(&thread1, &attr1, thread_cycle, (void *)123);
    printf("create thread: %d state: %d\n", thread1, ret);
    
    lwthread_t thread2;
    lwthread_attr_t attr2;
    lwthread_attr_init(&attr2);
    lwthread_attr_set_context(&attr2, thread_init, sizeof(thread_context_t));
    ret = lwthread_create(&thread2, &attr2, thread_cycle, (void *)456);
    printf("create thread: %d state: %d\n", thread2, ret);
    
    lwthread_t thread3;
    ret = lwthread_create(&thread3, NULL, thread2_cycle, (void *)NULL);
    printf("create thread: %d state: %d\n", thread3, ret);
    
    lwthread_schedule();
    return 0;
}
