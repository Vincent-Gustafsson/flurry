#include "flurry/multitasking/events.h"

#include <flurry/common.h>
#include <flurry/memory/kmalloc.h>


Event* head = NULL;
Event* tail = NULL;

Event* event_create() {
    Event* new_event = kmalloc(sizeof(Event));

    *new_event = (Event) {
        .deadline = 0,
        .callback = NULL,
        .callback_arg = NULL,
        .prev = NULL,
        .next = NULL
    };

    return new_event;
}

void event_destroy(Event* to_destroy) {
    kfree(to_destroy);
}

void event_enqueue(Event* to_insert) {
    kassert(to_insert != NULL, "Event to be inserted is null");

    if (head == NULL) {
        head = to_insert;
        tail = to_insert;

        to_insert->prev = NULL;
        to_insert->next = NULL;

        return;
    }

    Event* current = tail;

    while (current) {
        if (current == head) {
            current->prev = to_insert;
            to_insert->next = current;
            to_insert->prev = NULL;

            head = to_insert;

            return;
        }

        if (to_insert->deadline <= current->deadline)
            continue;

        if (current == tail) {
            current->next = to_insert;

            to_insert->prev = current;
            to_insert->next = NULL;

            tail = to_insert;

            return;
        }

        to_insert->next = current->next;
        to_insert->next->prev = to_insert;

        current->next = to_insert;
        to_insert->prev = current;

        return;
    }
}

void event_remove(Event* to_remove) {
    kassert(head != NULL, "Trying to remove event from an empty list");
    kassert(to_remove != NULL, "Event to be inserted is null");

    if (to_remove == head && to_remove == tail) {
        head = NULL;
        tail = NULL;

    } else if (to_remove == head) {
        head = to_remove->next;
        head->prev = NULL;

    } else if (to_remove == tail) {
        tail = to_remove->prev;
        tail->next = NULL;
    } else {
        to_remove->prev->next = to_remove->next;
        to_remove->next->prev = to_remove->prev;
    }

    *to_remove = (Event) {
        .deadline = 0,
        .callback = NULL,
        .callback_arg = NULL,
        .prev = NULL,
        .next = NULL
    };
}

Event* event_dequeue() {
    kassert(head != NULL, "event_get_next(): event_queue is empty");

    Event* to_return = head;

    if (head == tail) {
        head = NULL;
        tail = NULL;
    } else {
        head = head->next;
    }

    return to_return;
}

Event* event_peek_next() {
    kassert(head != NULL, "event_peek_next(): event_queue is empty");
    return head;
}
