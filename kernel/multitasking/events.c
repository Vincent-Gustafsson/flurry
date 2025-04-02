#include "flurry/multitasking/events.h"

#include <flurry/common.h>
#include <flurry/memory/kmalloc.h>


static Event* event_root = NULL;

Event* event_create() {
    Event* new_event = kmalloc(sizeof(Event));
    *new_event = (Event) {
        .owner = NULL,
        .time = 0,
        .callback = NULL,
        .callback_arg = NULL,
        .kind = "NONE",
        .left = NULL,
        .right = NULL,
        .parent = NULL
    };
    return new_event;
}

void event_destroy(Event* to_destroy) {
    kfree(to_destroy);
}

// Helper: Insert node into BST, ordering by time.
static void event_bst_insert(Event* node) {
    if (event_root == NULL) {
        event_root = node;
        return;
    }

    Event* current = event_root;
    Event* parent = NULL;
    while (current != NULL) {
        parent = current;
        if (node->time < current->time)
            current = current->left;
        else
            current = current->right;
    }
    node->parent = parent;
    if (node->time < parent->time)
        parent->left = node;
    else
        parent->right = node;
}

// Helper: BST transplant for deletion.
static void bst_transplant(Event* u, Event* v) {
    if (u->parent == NULL) {
        event_root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    if (v != NULL) {
        v->parent = u->parent;
    }
}

// Helper: Returns the minimum (leftmost) node in the BST.
static Event* bst_minimum(Event* node) {
    if (node == NULL)
        return NULL;
    while (node->left != NULL)
        node = node->left;
    return node;
}

// Helper: Remove a node from the BST.
static void bst_delete(Event* node) {
    if (node->left == NULL) {
        bst_transplant(node, node->right);
    } else if (node->right == NULL) {
        bst_transplant(node, node->left);
    } else {
        Event* y = bst_minimum(node->right);
        if (y->parent != node) {
            bst_transplant(y, y->right);
            y->right = node->right;
            if (y->right)
                y->right->parent = y;
        }
        bst_transplant(node, y);
        y->left = node->left;
        if (y->left)
            y->left->parent = y;
    }
}

// Remove a specific event from the BST.
void event_remove(Event* to_remove) {
    if (!event_root || !to_remove)
        return;
    bst_delete(to_remove);
    // Clear pointers and the enqueued flag.
    to_remove->left = to_remove->right = to_remove->parent = NULL;
    to_remove->enqueued = false;
}

// Inserts an event into the BST.
void event_enqueue(Event* to_insert) {
    kassert(to_insert != NULL, "Event to be inserted is null");

    if (to_insert->enqueued)
        event_remove(to_insert);

    // Initialize BST pointers.
    to_insert->left = to_insert->right = to_insert->parent = NULL;
    event_bst_insert(to_insert);
    to_insert->enqueued = true;
}

// Returns the event with the smallest time.
Event* event_peek_next() {
    return bst_minimum(event_root);
}

// Remove and return the event with the smallest time.
Event* event_dequeue() {
    kassert(event_root != NULL, "event_dequeue(): event_queue is empty");
    Event* min_event = bst_minimum(event_root);
    bst_delete(min_event);
    return min_event;
}

static void print_event_bst_inorder(Event* node, int log_level) {
    if (!node)
        return;

    // Traverse the left subtree.
    print_event_bst_inorder(node->left, log_level);

    // Print current event.
    logln(log_level, "Event: owner=%s, kind=%s, time=%llu",
          (node->owner != NULL) ? node->owner->name : "None",
          node->kind,
          (unsigned long long) node->time);

    // Traverse the right subtree.
    print_event_bst_inorder(node->right, log_level);
}

void print_event_queue(LogLevel log_level) {
    if (event_root == NULL) {
        logln(log_level, "Event queue is empty.");
    } else {
        logln(log_level, "Event queue contents (in order):");
        print_event_bst_inorder(event_root, log_level);
    }
}

