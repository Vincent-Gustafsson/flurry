#include "flurry/multitasking/sched.h"

#include <flurry/common.h>

#include "log.h"
#include "flurry/string.h"
#include "flurry/hardware/lapic.h"
#include "flurry/hardware/tsc.h"
#include "flurry/memory/kmalloc.h"
#include "flurry/memory/vmm.h"
#include "flurry/multitasking/process.h"
#include "flurry/multitasking/thread.h"


const uint64_t SCHED_THREAD_QUANTUM = 3000000; // 3 ms

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
void switch_thread(void* arg);

void maybe_reschedule_thread(Thread* thread) {
    lock_scheduler();
    // Don't reschedule the idle thread, get_next_thread takes care of that.
    if (thread != idle_thread) {
        switch(thread->status) {
            case THREAD_READY:
            case THREAD_RUNNING:
                thread->status = THREAD_READY;
                thread_list_add(ready_threads, thread);
            break;

            default: break;
        }
    }

    //logln(LOG_DEBUG, "queuing thread event from: %s", current_thread->name);

    current_thread->event->time = tsc_read_ns() + SCHED_THREAD_QUANTUM;
    current_thread->event->callback = switch_thread;
    current_thread->event->callback_arg = NULL;
    current_thread->event->kind = "switch";

    event_enqueue(current_thread->event);
    unlock_scheduler();
}

void handle_next_event(InterruptCtx* _);
void init_thread(Thread* prev) {
    maybe_reschedule_thread(prev);
    handle_next_event(NULL);
}

Thread* get_next_thread() {
    // Try to get one from the ready queue.
    Thread* ready_t = thread_list_get_next(ready_threads);

    if (ready_t)
        return ready_t;

    // Otherwise, run the idle thread.
    return idle_thread;
}

void switch_thread(void* arg) {
    Thread* this = current_thread;
    Thread* next = get_next_thread();

    if (next == current_thread) {
        //logln(LOG_DEBUG, "only %s is ready, rearming timer.", current_thread->name);


        current_thread->event->time = tsc_read_ns() + SCHED_THREAD_QUANTUM;
        current_thread->event->callback = switch_thread;
        current_thread->event->kind = "switch same";
        current_thread->event->callback_arg = NULL;

        event_enqueue(current_thread->event);
        return;
    }

    current_thread = next;
    current_thread->status = THREAD_RUNNING;

    //logln(LOG_DEBUG, "this: %s ; next: %s", this->name, next->name);

    Thread* old = switch_context(this, next);

    maybe_reschedule_thread(old);
}

void timer_fire_at(uint64_t time) {
    lapic_timer_one_shot(time - tsc_read_ns(), 32);
}

// ISR => CLI
void handle_next_event(__attribute__ ((unused)) InterruptCtx* _) {
    lapic_eoi();

    while (true) {
        Event* event = event_peek_next();

        if (!event)
            break;

        if (event->time > tsc_read_ns()) // TODO: rename tsc_read_ns into tsc_now (tsc_now_ns ?)
            break;

        event_remove(event);
        event->callback(event->callback_arg);
    }

    Event* next = event_peek_next();

    if (next)
        timer_fire_at(next->time);
}

/*
void handle_next_event(InterruptCtx* _) {
    print_event_queue(LOG_INFO);

    Event* event = event_peek_next();
    if (!event) {
        lapic_eoi();
        logln(LOG_DEBUG, "no events, switching to idle");
        switch_thread(NULL);
        return;
    }

    while (event && event->time <= tsc_read_ns()) {
        event = event_dequeue();
        event->callback(event->callback_arg);
        event = event_peek_next();
    }

    lapic_eoi();
    if (event)
        next_event_in(event->time - tsc_read_ns());
    else
        switch_thread(NULL);
}
 */

void thread_yield() {
    switch_thread(NULL);
}

void thread_wake(Thread* t) {
    kassert(t->status == THREAD_SLEEPING, "trying to wake an already awake thread");
    //logln(LOG_DEBUG, "Waking up thread: %s", t->name);
    t->status = THREAD_READY;
    thread_list_add(ready_threads, t);
}

void thread_sleep(uint64_t time) {
    lock_scheduler();
    uint64_t wake_time = tsc_read_ns() + time;
    //logln(LOG_DEBUG, "Thread %s sleeping until %lu ns", current_thread->name, wake_time);

    current_thread->status = THREAD_SLEEPING;

    current_thread->event->time = wake_time;
    current_thread->event->callback = thread_wake;
    current_thread->event->callback_arg = current_thread;
    current_thread->event->kind = "wake";

    event_enqueue(current_thread->event);

    unlock_scheduler();
    thread_yield();
}

void one() {
    while (true) {
        logln(LOG_NORMAL, "hello from 1");
        thread_sleep(from_ms(1000));
    }
}


void two() {
    while (true) {
        logln(LOG_NORMAL, "hello from 2");
        thread_sleep(from_ms(3000));
    }
}

void idle() {
    while (true) {
        //logln(LOG_DEBUG, "IDLE");
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

    idle_thread = thread_kcreate("idle", idle, kernel_process);
    Thread* t_one = thread_kcreate("one", one, kernel_process);
    Thread* t_two = thread_kcreate("two", two, kernel_process);

    thread_list_add(ready_threads, t_one);
    thread_list_add(ready_threads, t_two);

    current_thread = init_thread;
    interrupts_set_handler(32, handle_next_event);

    init_thread->event->callback = switch_thread;
    event_enqueue(init_thread->event);

    handle_next_event(NULL);
}
