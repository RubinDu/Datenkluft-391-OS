#include "malloc.h"

#define ALIGNED_SIZE(init_size) ((init_size & 0xFFFFFF80) + 128)
#define RESERVED_SIZE(size) ((size &0xFF))

#define MIN_SIZE 128

typedef struct block block_t;

struct block {
    uint32_t size;      /* the size of the block */
    block_t *prev;      /* previous block */
    block_t *next;      /* next block */
    unsigned char padding[sizeof(uint32_t)];    /* 16 bit is good ^_^ */
};

block_t *blocks = NULL;

void *malloc(uint32_t size) {
    if (size == 0 || size > KERNEL_DYNAMIC_CAPACITY) { /* the size is too large */
        return NULL;
    }

    size = ALIGNED_SIZE(size);                  /* align to multiple of 4 */
    uint32_t real_size = size + sizeof(block_t);/* for each block, we have a block_t */

    if (blocks) {                               /* we did the initialization*/
        block_t *free;
        for (free = blocks; free; free = free->next) {
            if (free->size >= real_size) {
                if (free->size - real_size < sizeof(block_t)) {
                    if (free->prev) {           /* remove this block from free blocks */
                        free->prev->next = free->next;
                    }
                    if (free->next) {
                        free->next->prev = free->prev;
                    }
                    return (void *)(free + 1);  /* all spaces is now used up */
                } else {
                    block_t *next = (block_t *)((unsigned char *)free + real_size);
                    next->size = free->size - real_size;
                    free->size = real_size;
                    next->prev = free->prev;
                    next->next = free->next;
                    if (free->prev) {
                        free->prev->next = next;
                    }
                    if (free->next) {
                        free->next->prev = next;
                    }
                    if (free == blocks) {
                        blocks = next;
                    }
                    return (void *)(free + 1);
                }
            }
        }

        return NULL;
    }
    
    blocks = (block_t *)(KERNEL_DYNAMIC_BASE);  /* no alloc happens */
    blocks->size = real_size;                   /* the remaining size */
    blocks->prev = NULL;                        /* the first one */
    blocks->next = NULL;
    blocks = (block_t *)((unsigned char*)blocks + real_size);
    blocks->size = (KERNEL_DYNAMIC_CAPACITY - real_size - sizeof(block_t));
    blocks->prev = NULL;
    blocks->next = NULL;
    return (void *)(KERNEL_DYNAMIC_BASE + sizeof(block_t));
}

void *calloc(uint32_t each, uint32_t count) {
    return malloc(each * count);
}

void free(void *ptr) {
    if (!ptr) {
        return;
    }

    block_t *cor = (block_t *)(ptr) - 1;
    cor->prev = NULL;
    cor->next = NULL;

    if (cor < blocks) {                         /* the block is the first free block */
        blocks->prev = cor;                     /* the block should be the head */
        cor->prev = NULL;
        cor->next = blocks;
        blocks = cor;                           /* reassign the head */
    } else {
        block_t *curr;
        for (curr = blocks; curr->next; curr = curr->next) {
            if ((uint32_t)curr < (uint32_t)cor && (uint32_t)cor < (uint32_t)curr->next) {
                cor->prev = curr;
                cor->next = curr->next;
                curr->next->prev = cor;
                curr->next = cor;
            }
        }
    }
    
    /* TODO: merge blocks */
}
