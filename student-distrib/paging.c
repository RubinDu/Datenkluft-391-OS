#include "paging.h"
#include "lib.h"

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
    memset((void*)page_table_user_vidmem, 0, PAGE_TABLE_COUNT * sizeof(pte_t));
    
    /* PDE #0: the first 4MB should be further split into 4KB subpages */
    page_directory[0].KB.present = 1;
    page_directory[0].KB.page_size = 0;
    page_directory[0].val |= (uint32_t)page_table; /* plug the page table address into the page_dir[0]*/

    /* PDE #1: the second 4MB should be kernel page */
    page_directory[1].MB.present = 1;
    page_directory[1].MB.page_size = 1;
    page_directory[1].val |= (uint32_t)KERNEL_ADDR; /* plug the kernel address into the page_dir[1]*/

    /* PDE/PTE 2-1024: set necessary things, not presented*/
    page_table[1].page_base_address = 1;
    page_table_user_vidmem[1].page_base_address = 1;
    for (i = 2; i < PAGE_DIRECTORY_COUNT; ++i) {
        page_directory[i].MB.page_size = 1; /* assign to 4MB pages for all remaining pages*/
        page_directory[i].MB.page_base_address = i; /* assign each address space to a specific page */
        page_table[i].page_base_address = i;
        page_table_user_vidmem[i].page_base_address = i;
    }
    
    page_table[VIDEO_MEMORY_PTE].present = 1; /* the page storing the video memory should be presented */
    
    page_table[VIDEO_MEMORY_PTE+1].present = 1; /* Backup video memory page */
    page_table[VIDEO_MEMORY_PTE+2].present = 1; /* Terminal 0 video memory page */
    page_table[VIDEO_MEMORY_PTE+3].present = 1; /* Terminal 1 video memory page */
    page_table[VIDEO_MEMORY_PTE+4].present = 1; /* Terminal 2 video memory page */

    page_table[VIDEO_MEMORY_PTE+1].page_base_address--; /* Set to map the video memory */
    
    page_table_user_vidmem[VIDEO_MEMORY_PTE].user_supervisor = 1;
    page_table_user_vidmem[VIDEO_MEMORY_PTE].read_write = 1;
    
    page_directory[VIDEO_MEMORY_PTE].KB.present = 1;
    page_directory[VIDEO_MEMORY_PTE].KB.user_supervisor = 1;
    page_directory[VIDEO_MEMORY_PTE].KB.read_write = 1;
    page_directory[VIDEO_MEMORY_PTE].KB.page_size = 0;          /* we need one subpage */
    page_directory[VIDEO_MEMORY_PTE].KB.page_table_base_address = ((uint32_t)page_table_user_vidmem >> 12);

    /* **************************************************
     * *          Set Page Directory to CR3             *
     * **************************************************/
    asm volatile (
        "movl %0, %%cr3" :                      /* no outputs */
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
