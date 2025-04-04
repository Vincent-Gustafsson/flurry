﻿#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "flurry/multitasking/thread.h"
#include "log.h"

typedef void (*EventCallback)(void* arg);

typedef struct Thread Thread;

/* One event per Thread. */
typedef struct Event {
    Thread* owner;

    uint64_t time;
    bool enqueued;

    EventCallback callback;
    char* kind;
    void* callback_arg;

    struct Event* left;
    struct Event* right;
    struct Event* parent;
} Event;

/* Creates an empty event. */
Event* event_create();

/* Destroys (frees) the given event.
 * Call it when the owning thread is reaped.
 */
void event_destroy(Event* to_destroy);

/* Inserts an event according to its time. */
void event_enqueue(Event* to_insert);

/* Remove an element. */
void event_remove(Event* to_remove);

/* Get the first element in the queue */
Event* event_dequeue();

/* Get the first element in the queue without dequeuing it */
Event* event_peek_next();

void print_event_queue(LogLevel level);

