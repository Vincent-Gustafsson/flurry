﻿#include "flurry/memory/kmalloc.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "log.h"
#include "flurry/common.h"
#include "flurry/log/tty.h"
#include "flurry/memory/pmm.h"
#include "flurry/memory/vmm.h"

/* CODE YOINKED FROM https://github.com/Aster-OS/aster/blob/main/kernel/src/memory/kmalloc/kmalloc.c */

static const uintptr_t HEAP_START = 0xffff810000000000;
static const uintptr_t HEAP_SIZE = 0x200000;
static const uintptr_t HEAP_END = HEAP_START + HEAP_SIZE;

static const uint64_t HEAP_ALIGNMENT = 8;
static const uint64_t CHUNK_SIZE_MASK = ~(HEAP_ALIGNMENT - 1);

static const uint64_t FLAG_IS_FREE = 0x1;
static const uint64_t FLAG_IS_PREV_FREE = 0x2;

// free chunks are kept on a doubly linked freelist
struct free_node_t {
    size_t chunk_metadata; // do not move
    struct free_node_t *prev;
    struct free_node_t *next;
};

// footer placed right before the end of every free chunk
struct free_footer_t {
    size_t sz;
};

// header placed before every allocated chunk, which is just chunk metadata
struct alloc_hdr_t {
    size_t chunk_metadata;
};

// chunk metadata:
// - is placed at the start of every chunk
// - is of type `size_t`
// - has the following structure:
//     bit 0: FLAG_IS_FREE          - is this chunk free?
//     bit 1: FLAG_IS_PREV_FREE     - is the previous chunk free?
//     bit 2: unused
//     bits 3 and above: chunk size - how large is this chunk? (always aligned on 8-byte boundary)
//   since chunk sizes are always aligned, the lower bits can be used for flagging

// before the end of every free chunk is placed a `free_footer_t` struct
// which holds the size of the chunk
// this footer is used for coalescing adjacent free chunks,
// specifically, for obtaining the size of the previous chunk, relative to the chunk to be freed

static inline size_t align_sz(size_t sz) {
    return (size_t) ALIGN_UP(sz, HEAP_ALIGNMENT);
}

static inline size_t get_sz(void *chunk) {
    // metadata is at the start of any chunk
    size_t *metadata = (size_t *) chunk;
    return *metadata & CHUNK_SIZE_MASK;
}

static inline void set_sz(void *chunk, size_t sz) {
    size_t *metadata = (size_t *) chunk;
    // should not overwrite metadata flags
    *metadata &= ~CHUNK_SIZE_MASK;
    *metadata |= sz & CHUNK_SIZE_MASK;
}

static inline bool get_flag(void *chunk, uint8_t flag) {
    size_t *metadata = (size_t *) chunk;
    return *metadata & flag;
}

static inline void set_flag(void *chunk, uint8_t flag) {
    size_t *metadata = (size_t *) chunk;
    *metadata |= flag;
}

static inline void unset_flag(void *chunk, uint8_t flag) {
    size_t *metadata = (size_t *) chunk;
    *metadata &= ~flag;
}

static inline bool is_in_heap_bounds(uintptr_t addr) {
    return addr >= HEAP_START && addr < HEAP_END;
}

static struct free_node_t *freelist_head;

static void freelist_add_node(struct free_node_t *node_to_add) {
    if (freelist_head == NULL) {
        freelist_head = node_to_add;
        freelist_head->prev = NULL;
        freelist_head->next = NULL;
        return;
    }

    node_to_add->prev = NULL;
    node_to_add->next = freelist_head;
    freelist_head->prev = node_to_add;
    freelist_head = node_to_add;
}

static void freelist_remove_node(struct free_node_t *node_to_remove) {
    kassert(node_to_remove != NULL, "???");

    if (node_to_remove->prev != NULL) {
        node_to_remove->prev->next = node_to_remove->next;
    }

    if (node_to_remove->next != NULL) {
        node_to_remove->next->prev = node_to_remove->prev;
    }

    if (node_to_remove == freelist_head) {
        freelist_head = freelist_head->next;
    }
}

void *kmalloc(size_t sz) {
    size_t alloc_sz = sz + sizeof(struct alloc_hdr_t);
    alloc_sz = align_sz(alloc_sz);

    // make sure that after the allocation, when this chunk is freed,
    // it will be large enough to hold the free_node_t and the free_footer_t structs
    size_t FREE_CHUNK_MIN_SZ =
        align_sz(sizeof(struct free_node_t) + sizeof(struct free_footer_t));
    if (alloc_sz < FREE_CHUNK_MIN_SZ) {
        alloc_sz = FREE_CHUNK_MIN_SZ;
    }

    // search for a free chunk which either has a free size equal to alloc_sz:
    //   allocate the whole chunk
    // OR a free size of at least alloc_sz + FREE_CHUNK_MIN_SZ:
    //   split the chunk in two; allocate the first slice; add the second slice on the freelist

    struct free_node_t *free = freelist_head;
    size_t free_sz;
    while (free != NULL) {
        free_sz = get_sz(free);
        if (free_sz == alloc_sz ||
            free_sz >= alloc_sz + FREE_CHUNK_MIN_SZ) {
            break;
        }

        free = free->next;
    }

    if (free == NULL) {
        kpanic("Kheap out of memory");
        return NULL;
    }

    // if a free chunk is found, remove it from the freelist
    freelist_remove_node(free);

    uintptr_t free_addr = (uintptr_t) free;

    if (free_sz == alloc_sz) {
        // this whole chunk gets allocated,
        // so the next chunk will not have any previous free chunk
        uintptr_t next_addr = free_addr + free_sz;
        if (is_in_heap_bounds(next_addr)) {
            size_t *next = (size_t *) next_addr;
            unset_flag(next, FLAG_IS_PREV_FREE);
        }

    } else /* free_sz >= alloc_sz + FREE_CHUNK_MIN_SZ */ {
        uintptr_t remain_addr = free_addr + alloc_sz;
        struct free_node_t *remain = (struct free_node_t *) remain_addr;
        set_sz(remain, free_sz - alloc_sz);
        set_flag(remain, FLAG_IS_FREE);
        unset_flag(remain, FLAG_IS_PREV_FREE);

        // add the remaining slice on the freelist
        freelist_add_node(remain);

        // place footer
        uintptr_t remain_end_addr = remain_addr + get_sz(remain);
        uintptr_t remain_footer_addr = remain_end_addr - sizeof(struct free_footer_t);
        struct free_footer_t *remain_footer = (struct free_footer_t *) remain_footer_addr;
        remain_footer->sz = get_sz(remain);
    }

    struct alloc_hdr_t *alloc_hdr = (struct alloc_hdr_t *) free;
    set_sz(alloc_hdr, alloc_sz);
    unset_flag(alloc_hdr, FLAG_IS_FREE);
    // FLAG_IS_PREV_FREE is left unchanged

    // return the address after the allocation header
    return (void *) ((uintptr_t) alloc_hdr + sizeof(struct alloc_hdr_t));
}

void kfree(void *ptr) {
    uintptr_t ptr_addr = (uintptr_t) ptr;

    kassert(is_in_heap_bounds(ptr_addr), "kmalloc: free address out of heap bounds");

    // the chunk to be freed and its address
    uintptr_t to_free_addr = (uintptr_t) (ptr_addr - sizeof(struct alloc_hdr_t));
    struct free_node_t *to_free = (struct free_node_t *) to_free_addr;

    // double frees are considered a bug
    kassert(!get_flag(to_free, FLAG_IS_FREE), "kmalloc: double free");

    uintptr_t next_addr = to_free_addr + get_sz(to_free);
    size_t *next = (size_t *) next_addr;

    bool coalesce_with_prev = get_flag(to_free, FLAG_IS_PREV_FREE);
    bool coalesce_with_next = is_in_heap_bounds(next_addr) && get_flag(next, FLAG_IS_FREE);

    // no coalescing
    if (!coalesce_with_prev && !coalesce_with_next) {
        // place footer
        uintptr_t new_footer_addr = to_free_addr + get_sz(to_free) - sizeof(struct free_footer_t);
        struct free_footer_t *new_footer = (struct free_footer_t *) new_footer_addr;
        new_footer->sz = get_sz(to_free);

        set_flag(to_free, FLAG_IS_FREE); // mark `to_free` as free
        freelist_add_node(to_free);

        // `to_free` was freed
        // now the next chunk has a previous free chunk
        if (is_in_heap_bounds(next_addr)) {
            set_flag(next, FLAG_IS_PREV_FREE);
        }

    // coalesce with the prev chunk only
    } else if (coalesce_with_prev && !coalesce_with_next) {
        // use the previous chunk footer to get previous chunk address
        uintptr_t prev_footer_addr = to_free_addr - sizeof(struct free_footer_t);
        struct free_footer_t *prev_footer = (struct free_footer_t *) prev_footer_addr;
        uintptr_t prev_addr = to_free_addr - prev_footer->sz;
        struct free_node_t *prev = (struct free_node_t *) prev_addr;

        size_t new_sz = get_sz(prev) + get_sz(to_free);
        set_sz(prev, new_sz);

        // place footer for the coalesced chunk
        uintptr_t new_footer_addr = prev_addr + new_sz - sizeof(struct free_footer_t);
        struct free_footer_t *new_footer = (struct free_footer_t *) new_footer_addr;
        new_footer->sz = new_sz;

        // `to_free` was freed
        // now the next chunk has a previous free chunk
        if (is_in_heap_bounds(next_addr)) {
            set_flag(next, FLAG_IS_PREV_FREE);
        }

    // coalesce with the next chunk only
    } else if (!coalesce_with_prev && coalesce_with_next) {
        size_t new_sz = get_sz(to_free) + get_sz(next);
        set_sz(to_free, new_sz);

        // place footer for the coalesced chunk
        uintptr_t new_footer_addr = to_free_addr + new_sz - sizeof(struct free_footer_t);
        struct free_footer_t *new_footer = (struct free_footer_t *) new_footer_addr;
        new_footer->sz = new_sz;

        freelist_remove_node((struct free_node_t *) next);

        set_flag(to_free, FLAG_IS_FREE); // mark `to_free` as free
        freelist_add_node(to_free);

    // coalesce with the prev & next chunk
    } else {
        // use the previous chunk footer to get previous chunk address
        uintptr_t prev_footer_addr = to_free_addr - sizeof(struct free_footer_t);
        struct free_footer_t *prev_footer = (struct free_footer_t *) prev_footer_addr;
        uintptr_t prev_addr = to_free_addr - prev_footer->sz;
        struct free_node_t *prev = (struct free_node_t *) prev_addr;

        size_t new_sz = get_sz(prev) + get_sz(to_free) + get_sz(next);
        set_sz(prev, new_sz);

        // place footer for the coalesced chunk
        uintptr_t new_footer_addr = prev_addr + new_sz - sizeof(struct free_footer_t);
        struct free_footer_t *new_footer = (struct free_footer_t *) new_footer_addr;
        new_footer->sz = new_sz;

        freelist_remove_node((struct free_node_t *) next);
    }
}

void kmalloc_init() {
    for (uintptr_t virt = HEAP_START; virt < HEAP_END; virt += PAGE_SIZE) {
        uint64_t phys = pmm_alloc();
        vmm_kmap(virt, phys, VM_FLAG_WRITE | VM_FLAG_NX);
    }

    struct free_node_t *head = (struct free_node_t *) HEAP_START;
    set_sz(head, align_sz(HEAP_SIZE));
    set_flag(head, FLAG_IS_FREE);
    unset_flag(head, FLAG_IS_PREV_FREE);
    freelist_add_node(head);

    logln(LOG_INFO, "[KMALLOC] Initialized with %dMiB of memory", HEAP_SIZE >> 20);
}
