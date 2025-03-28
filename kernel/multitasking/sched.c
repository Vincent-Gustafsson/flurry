#include "flurry/multitasking/sched.h"

#include <stddef.h>
#include <flurry/log/tty.h>
#include <sys/types.h>

#include "log.h"
#include "flurry/string.h"
#include "flurry/hardware/lapic.h"
#include "flurry/memory/kmalloc.h"
#include "flurry/memory/vmm.h"
#include "flurry/multitasking/process.h"
#include "flurry/multitasking/thread.h"
#include "flurry/time/timer.h"



int IRQ_disable_counter = 0;

void lock_scheduler(void) {
    asm volatile("cli");
    IRQ_disable_counter++;
}

void unlock_scheduler(void) {
    IRQ_disable_counter--;
    if(IRQ_disable_counter == 0)
        asm volatile("sti");
}

typedef struct ThreadListNode {
    Thread* thread;
    struct ThreadListNode* next;
} ThreadListNode;

typedef struct {
    ThreadListNode* head;
    ThreadListNode* tail;
} ThreadList;

ThreadList* thread_list_new() {
    ThreadList* list = kmalloc(sizeof(ThreadList));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void thread_list_add(ThreadList* list, Thread* t) {
    // TODO LOCKING? DISABLING INTERRUPTS?
    ThreadListNode* node = kmalloc(sizeof(ThreadListNode));
    *node = (ThreadListNode) { .thread = t, .next = NULL };

    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
        return;
    }

    list->tail->next = node;
    list->tail = node;
}

Thread* thread_list_get_next(ThreadList* list) {
    ThreadListNode* head = list->head;

    if (head == NULL)
        return NULL;

    list->head = head->next;
    if (list->head == NULL)  // List is empty, update tail.
        list->tail = NULL;

    Thread* t = head->thread;
    kfree(head);

    return t;
}




Process* kernel_process;

ThreadList* ready_threads;
ThreadList* finished_threads;


Thread* idle_thread;
Thread* reaper_thread;
Thread* current_thread;

Thread* switch_context(Thread* old, Thread* new);

Thread* get_next_thread() {
    // Try to get one from the ready queue.
    Thread* ready_t = thread_list_get_next(ready_threads);

    if (ready_t != NULL)
        return ready_t;

    // Otherwise, run the idle thread.
    return idle_thread;
}

void switch_thread(void* arg) {
    Thread* this = current_thread;
    Thread* next = get_next_thread();
    current_thread = next;

    next->status = THREAD_RUNNING;

    logln(LOG_DEBUG, "this: %s ; next: %s", this->name, next->name);

    switch(this->status) {
        case THREAD_RUNNING: thread_list_add(ready_threads, this); break;
        case THREAD_READY: break;
        case THREAD_DONE: break;
        default: break;
    }

    current_thread->event->deadline = ns_to_lapic_ticks(3000000);

    event_enqueue(current_thread->event);

    Thread* old = switch_context(this, next);
}

void rearm_timer(uint64_t next_time) {
    lapic_timer_one_shot(next_time, 32);
}

void handle_next_event(InterruptCtx* _) {
    lapic_eoi();

    Event* event = event_dequeue();
    event->callback(event->callback_arg);

    rearm_timer(event->deadline);
}



void sched() {

    /*
    Thread* this = current_thread;
    Thread* next = get_next_thread();
    current_thread = next;

    this->status = THREAD_READY;
    next->status = THREAD_RUNNING;

    logln(LOG_DEBUG, "this: %s ; next: %s", this->name, next->name);

    Thread* old = switch_context(this, next);

    thread_list_add(ready_threads, old);
    */
}

void handle_timer_int(InterruptCtx* ctx) {
    lock_scheduler();
    lapic_eoi();
    sched();
    unlock_scheduler();
}

void one() {
    while (true) {
        logln(LOG_NORMAL, "hello from 1");
    }
}

void two() {
    while (true) {
        logln(LOG_NORMAL, "hello from 2");
    }
}

void idle() {
    while (true) {
        logln(LOG_NORMAL, "IDLE");
        asm volatile("pause");
    }
}

void reap() {
    while (true) {
        Thread* to_be_reaped = thread_list_get_next(finished_threads);
        while (to_be_reaped != NULL) {
            // just leak it lol
            logln(LOG_NORMAL, "reaping \"%s\"", to_be_reaped->name);
            to_be_reaped = thread_list_get_next(finished_threads);
        }
    }
}

void sched_init() {
    ready_threads = thread_list_new();
    finished_threads = thread_list_new();

    kernel_process = process_create("kernel", vmm_get_kernel_pml4());

    Thread* init_thread = kmalloc(sizeof(Thread));
    strcpy(init_thread->name, "init");
    init_thread->event = event_create();
    init_thread->status = THREAD_DONE;

    //reaper_thread = thread_kcreate("reaper", reap, kernel_process);
    //hread_list_add(ready_threads, reaper_thread);

    idle_thread = thread_kcreate("idle", idle, kernel_process);
    Thread* t_one = thread_kcreate("one", one, kernel_process);
    Thread* t_two = thread_kcreate("two", two, kernel_process);

    t_one->event->callback = switch_thread;
    t_two->event->callback = switch_thread;

    thread_list_add(ready_threads, t_one);
    thread_list_add(ready_threads, t_two);

    current_thread = init_thread;
    interrupts_set_handler(32, handle_next_event);

    init_thread->event->callback = switch_thread;
    event_enqueue(init_thread->event);

    interrupts_set_handler(32, handle_next_event);
    lapic_timer_one_shot(1000000, 32);
}
