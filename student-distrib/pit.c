#include "pit.h"
#include "common_asm_link.h"
#include "i8259.h"

#define PIT_FREQUENCY 1193182
#define PIT_SQUARE_MODE 0x36

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42
#define PIT_COMMAND 0x43

/* void pit_init(uint16_t frequency)
 * Inputs: uint16_t frequency: PIT frequency in HZ
 * Return Value: none
 * Side effect: Initialize the PIT with the given frequency
 * Function: Enable PIT for scheduling */
void pit_init(uint16_t frequency) {
    uint16_t param = PIT_FREQUENCY / frequency;

    outb(PIT_SQUARE_MODE, PIT_COMMAND);
    outb((uint8_t)(param & 0xFF), PIT_CHANNEL_0);           /* send the frequency byte by byte */
    outb((uint8_t)((param >> 8) & 0xFF), PIT_CHANNEL_0);    /* shift right and reserve last 8 bytes*/

    enable_irq(0);                                          /* enable the interrupt 0x20 in PIC */
}

/* void pit_handler()
 * Inputs: none
 * Return Value: none
 * Side effect: Switch the currently running process
 * Function: Handle the PIT interrupt and schedule processes in a round robin fashion */
void pit_handler() {
    send_eoi(0);            /* send eoi before handling it */
    int current_terminal = *get_current_terminal();
    int next_terminal = (*get_current_terminal()+1)%3;
    pcb_t *current = GET_PCB(get_terminal(current_terminal)->pid);
    pcb_t *next;

    if (get_terminal(*get_current_terminal())->pid != -1){                /* If the kernel has been set up, if not, let execute set them */
        asm volatile (
            "movl %%ebp, %0\n"
            : "=r"(current->ebp)
        );
        current->esp0 = tss.esp0;
    }

    if (next_terminal == *get_active_terminal()) {                              /* show the content */
        page_table[VIDEO_MEMORY_PTE].page_base_address = VIDEO_MEMORY_PTE;
        page_table_user_vidmem[VIDEO_MEMORY_PTE].page_base_address = VIDEO_MEMORY_PTE;
    } else {                                                                    /* don't need to show, but need to update */
        page_table[VIDEO_MEMORY_PTE].page_base_address = VIDEO_MEMORY_PTE + next_terminal + 2;
        page_table_user_vidmem[VIDEO_MEMORY_PTE].page_base_address = VIDEO_MEMORY_PTE + next_terminal + 2;
    }

    sync_terminal();    
    *get_current_terminal() = next_terminal;                                    /* update the current terminal */
    
    if (get_terminal(next_terminal)->pid == -1)                                 /* if the task going to switch to isn't running */
        execute((uint8_t*)"shell");
    
    next = GET_PCB(get_terminal(next_terminal)->pid);

    page_directory[USER_ENTRY].MB.page_base_address = 2 + next->pid;
    page_table_user_vidmem[VIDEO_MEMORY_PTE].present = next->vidmap;

    tss.esp0=next->esp0;

    if (get_terminal(next_terminal)->halt){    //If scheduled to be halted
        asm volatile (
        "movl %%cr3, %%ecx\n"       /* flush the TLB*/
        "movl %%ecx, %%cr3\n"

        "movl %0, %%ebp\n"       /* set new EBP, go to that kernel stack and halt*/
        // "sti\n"
        "leave\n"               
        :
        : "r"(next->ebp)
        : "%ecx"
        );
        halt(128);
    }
    

    asm volatile (
        "movl %%cr3, %%ecx\n"       /* flush the TLB*/
        "movl %%ecx, %%cr3\n"
        "movl %0, %%ebp\n"       /* set new EBP, used by return */
        // "sti\n"

        "leave\n" 
        "ret\n" 
        :
        : "r"(next->ebp)
        : "%ecx"
    );
}
