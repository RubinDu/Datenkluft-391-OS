#include "malloc.h"

#define ALIGNED_SIZE(init_size) ((init_size + 3) & ~0x03)
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

/**
 * void *malloc(uint32_t size):
 * DESCRIPTION: allocates a data in \p size bytes
 * INPUT: size - the size of the buffer
 * OUTPUT: none
 * RETURN: the allocated buffer, or NULL if failed
 */
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
    blocks->size = (KERNEL_DYNAMIC_CAPACITY - real_size);
    blocks->prev = NULL;
    blocks->next = NULL;
    return (void *)(KERNEL_DYNAMIC_BASE + sizeof(block_t));
}

/**
 * void *calloc(uint32_t each, uint32_t count):
 * DESCRIPTION: allocate \p count of \p each bytes
 * INPUT: each - sizeof(T)
 *        count - the amount of T
 * OUTPUT: none
 * RETURN: allocated buffer, or NULL if failed
 */
void *calloc(uint32_t each, uint32_t count) {
    return malloc(each * count);
}

/**
 * void *realloc(void *ptr, uint32_t new_size)
 * DESCRIPTION: enlarges the buffer from the original size to \p new_size
 * INPUT: ptr - the old buffer
 * OUTPUT: none
 * RETURN: allocated buffer, might be different
 */
void *realloc(void *ptr, uint32_t new_size) {
    if (!ptr) {                                 /* create new */
        return malloc(new_size);
    } else if (new_size == 0) {                 /* no longer need */
        free(ptr);
        return NULL;
    }

    block_t *cor = (block_t *)(ptr) - 1, *curr;
    if (cor->size - sizeof(block_t) > new_size) {
        return ptr;                             /* shrink it? NO!!! */
    }

    uint32_t real_size = new_size + sizeof(block_t);
    for (curr = blocks; curr; curr = curr->next) {
        if (curr == (block_t *)((unsigned char *)cor + cor->size)) {    /* free blocks just after the block */
            uint32_t extra_size = real_size - cor->size;                /* new size */
            if (curr->size > extra_size) {
                if (curr->size - extra_size <= sizeof(block_t)) {        /* delete this block */
                    if (curr->prev) {
                        curr->prev->next = curr->next;
                    }
                    if (curr->next) {
                        curr->next->prev = curr->prev;
                    }

                    cor->size += (curr->size - extra_size);             /* enlarge this block */
                } else {                                                /* shrink the block */
                    block_t *next = (block_t *)((unsigned char*)curr + extra_size);
                    cor->size += extra_size;
                    next->size = (curr->size - extra_size);             /* assign the next size */
                    next->prev = curr->prev;
                    next->next = curr->next;

                    if (curr->prev) {                                   /* create node and assign new node */
                        curr->prev->next = next;
                    }
                    if (curr->next) {
                        curr->next->prev = next;
                    }

                    if (curr == blocks) {                               /* refresh head */
                        blocks = next;
                    }
                }
            } else break;                                               /* adjacent blocks is large enough */
            
            return ptr;

            if (curr->size > extra_size) {          /* free block is large enough */
                if (curr->prev) {
                    curr->prev->next = curr->next;
                }
                if (curr->next) {
                    curr->next->prev = curr->prev;
                }
                cor->size = new_size + sizeof(block_t);

                return ptr;
            } else
                break;                              /* we can't enlarge it through two non-adjacent buffer */
        }
    }

    /* cannot find it, free and reallocate it is the only solution */
    void *new_ptr = malloc(new_size);
    memcpy(new_ptr, ptr, cor->size - sizeof(block_t));
    free(ptr);
    return new_ptr;
}

/**
 * void free(void *ptr):
 * DESCRIPTION: release the buffer
 * INPUT: ptr - allocated buffer
 * OUTPUT: none
 * RETURN: none
 */
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
    
    block_t *begin, *end;
    for (begin = cor;
         begin->prev                                                        /* check reached the head */
            && (uint32_t)begin->prev + begin->prev->size == (uint32_t)begin;/* check if adjacent */
         begin = begin->prev);
    for (end = cor;                                                         /* check reached the tail */
         end->next && (uint32_t)end + end->size == (uint32_t)end->next;     /* check if adjacent */
         end = end->next);

    if (begin != end) {
        begin->size = ((uint32_t)end + end->size - (uint32_t)begin);        /* merges the size */
        if (begin == blocks) {                                              /* reached the head -> assign */
            blocks->prev = NULL;
            blocks->next = end->next;
        } else {
            begin->next = end->next;                                        /* keep the first block */
            if (end->next) {
                end->next->prev = begin;                                    /* keep linked list alive */
            }
        }
    }
}
