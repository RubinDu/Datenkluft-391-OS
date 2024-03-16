#include "paging.h"
#include "lib.h"

pde_t page_directory[PAGE_DIRECTORY_COUNT] __attribute__((aligned(PAGE_DIRECTORY_COUNT)));
pte_t page_table[PAGE_TABLE_COUNT] __attribute__((aligned(PAGE_TABLE_COUNT)));

/**
 * void init_paging();
 *      DESCRIPTION: Initializes the paging configuration;
 *                   As the document mentioned, the first 4MB
 *                   page should be broken down into 4KB subpages,
 *                   while the remaining pages should be 4MB.
 * 
 *      INPUTS: None
 *      OUTPUTS: None
 *      RETURN: None
 * 
 *      SIDEEFFECTS: None
 */
void paging_init() {
    int i;
    int32_t ctrl_reg; /* used to g/s control registers */

    /* zero all of them */
    memset((void*)page_directory, 0, PAGE_DIRECTORY_COUNT * sizeof(pde_t));
    memset((void*)page_table, 0, PAGE_TABLE_COUNT * sizeof(pte_t));
    
    /* PDE #0: the first 4MB should be further split into 4KB subpages */
    page_directory[0].KB.present = 1;
    page_directory[0].KB.page_size = 0;
    page_directory[0].KB.page_table_base_address = ((uint32_t)page_table >> PTE_OFFSET);

    /* PDE #1: the second 4MB should be kernel page */
    page_directory[1].MB.present = 1;
    page_directory[1].MB.page_size = 1;
    page_directory[1].MB.page_base_address = (0x400000 >> PDE_OFFSET);

    page_table[1].page_base_address = 1;
    for (i = 2; i < PAGE_DIRECTORY_COUNT; ++i) {
        page_directory[i].MB.page_size = 1;
        page_directory[i].MB.page_base_address = i;
        page_table[i].page_base_address = i;
    }
    
    page_table[VIDEO_MEMORY_PTE].present = 1;

    /* **************************************************
     * *          Set Page Directory to CR3             *
     * **************************************************/
    asm volatile (
        "movl %0, %%cr3" :                /* no outputs */
                         : "r" (page_directory) /* input, gp register*/
    );

    /* **************************************************
     * *             Set Page Size to 4MB               *
     * **************************************************/
    asm volatile (
        "movl %%cr4, %0" : "=r" (ctrl_reg)  /* output, gp register */
                         :                  /* no inputs */
    );

    ctrl_reg |= PAGING_SIZE_EXTENTION_FLAG; /* set the PG flag */

    asm volatile (
        "movl %0, %%cr4" :                  /* no outputs */
                         : "r" (ctrl_reg)   /* input, gp register*/
    );

    /* **************************************************
     * *                 Enable Paging                  *
     * **************************************************/
    asm volatile (
        "movl %%cr0, %0" : "=r" (ctrl_reg)  /* output, gp register */
                         :                  /* no inputs */
    );

    ctrl_reg |= PAGING_FLAG; /* set the PG flag */

    asm volatile (
        "movl %0, %%cr0" :                  /* no outputs */
                         : "r" (ctrl_reg)   /* input, gp register*/
    );
}