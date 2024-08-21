#include "uthread.h"
#include "user/user.h"
#include "kernel/types.h"

struct uthread threads[MAX_UTHREADS];
struct uthread *currThread;
int started = 0;

int uthread_create(void (*start_func)(), enum sched_priority priority){
    struct uthread *ut = threads;
    while(ut < &threads[MAX_UTHREADS]){
        if(ut->state == FREE){
            ut->state = RUNNABLE;
            ut->priority = priority;
            ut->context.ra = (uint64) start_func;
            ut->context.sp = (uint64) &(ut->ustack) + STACK_SIZE;
            return 0;
        }
        ut++;
    }
    return -1;
}

void uthread_yield() {
    struct uthread *next = currThread;
    enum sched_priority maybeNext = LOW;
    int i;
    for (i = 0; i < MAX_UTHREADS; i++) {  //find the next thread
        if (threads[i].state == RUNNABLE && threads[i].priority > maybeNext) {
            maybeNext = threads[i].priority;
            next = &threads[i];
        }
    }
    if (next != currThread) {  //switch to the next thread
        currThread->state = RUNNABLE;
        next->state = RUNNING;
        currThread = next;
        uswtch(&currThread->context, &next->context);
    }
}

void uthread_exit(){
    struct uthread *ut = uthread_self();
    struct uthread *curr = threads;
    struct uthread *next = 0;
    ut->state = FREE;
    int found = 0;
    int currPrio = -1;
    while(curr < &threads[MAX_UTHREADS]){
        if(curr->state == RUNNABLE){
            if(currPrio == -1){
                currPrio = curr -> priority;
                next = curr;
            }
            else if (curr -> priority > currPrio){
            currPrio = curr -> priority;
            next = curr;
            found = 1;
            }
        }
        curr++;
    }   
    if (!found){
        exit(0);
    }
    currThread=next;
    uswtch(&ut->context, &next->context);
     
}

enum sched_priority uthread_set_priority(enum sched_priority priority){
    int before = currThread -> priority;
    currThread -> priority = priority;
    return before;
}

enum sched_priority uthread_get_priority(){
    return currThread -> priority;
}

int uthread_start_all() { 
    if (started == 0) {
        started = 1;
        struct uthread *next = 0;
        struct context curr;
        enum sched_priority maybeNext = LOW;       
        int i;

        for (i = 0; i < MAX_UTHREADS; i++) {  //find the next thread
            if (threads[i].state == RUNNABLE && threads[i].priority >= maybeNext) { 
                maybeNext = threads[i].priority;
                next = &threads[i];
            }
        }
        if (next == 0) {
            return -1;
        }
        currThread = next;
        currThread->state = RUNNING;
        uswtch(&curr, &currThread->context); // switch to the selected thread
        return 1;
    }
    return -1;
}

struct uthread* uthread_self(){
    return currThread;
}
