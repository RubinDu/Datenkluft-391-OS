#ifndef _MALLOC_H
#define _MALLOC_H

#include "lib.h"

#define KERNEL_DYNAMIC_BASE (0xCA000000)
#define KERNEL_DYNAMIC_PTE (KERNEL_DYNAMIC_BASE >> 22)
#define KERNEL_DYNAMIC_PHYSICAL_BASE (10)
#define KERNEL_DYNAMIC_CAPACITY (0xD0000000 - 0xCA000000) /* 0x6000000 */

void *malloc(uint32_t size);

void *calloc(uint32_t each, uint32_t count);

void *realloc(void *ptr, uint32_t new_size);

void free(void *ptr);

#endif
