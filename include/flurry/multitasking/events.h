#pragma once

#include <stdint.h>

typedef void (*EventCallback)(void* arg);

/* One event per Thread. */
typedef struct Event {
    uint64_t deadline;

    EventCallback callback;
    void* callback_arg;

    struct Event* prev;
    struct Event* next;
} Event;

/* Creates an empty event. */
Event* event_create();

/* Destroys (frees) the given event.
 * Call it when the owning thread is reaped.
 */
void event_destroy(Event* to_destroy);

/* Inserts an event according to its deadline. */
void event_enqueue(Event* to_insert);

/* Remove an element. */
void event_remove(Event* to_remove);

/* Get the first element in the queue */
Event* event_dequeue();

/* Get the first element in the queue without dequeuing it */
Event* event_peek_next();

