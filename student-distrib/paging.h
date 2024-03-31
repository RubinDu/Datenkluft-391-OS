#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "x86_desc.h"

/**
 * To enable the paging, we need to set the bit 31 of CR0
 * to 1 to enable the paging, and set the protection mode,
 * which is the bit 0 of CR0.
 */
#define PAGING_FLAG 0x80000001

/* enables 4MB pages, bit #4 of CR4 */
#define PAGING_SIZE_EXTENTION_FLAG 0x00000010

#define PAGING_ALIGNMENT 0x1000     /* the alignment of PDE and PTE */

#define KERNEL_ADDR 0x400000        /* kernel lies in here */
#define KERNEL_PDE 1                /* the index of kernel page in PDE */
#define VIDEO_MEMORY_ADDR 0xB8000   /* video memory lies in here */
#define VIDEO_MEMORY_PTE 0xB8       /* the index of video memory PTE in 0th page*/

pde_t page_directory[PAGE_DIRECTORY_COUNT] __attribute__((aligned(PAGING_ALIGNMENT)));
pte_t page_table[PAGE_TABLE_COUNT] __attribute__((aligned(PAGING_ALIGNMENT)));

/* initialize the paging configuration of x86 */
void paging_init();

#endif
