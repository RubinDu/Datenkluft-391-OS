#include "pit.h"
#include "common_asm_link.h"
#include "i8259.h"
#include "scheduling.h"

#define PIT_FREQUENCY 1193182
#define PIT_SQUARE_MODE 0x36

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42
#define PIT_COMMAND 0x43

void pit_init(uint16_t frequency) {
    uint16_t param = PIT_FREQUENCY / frequency;

    outb(PIT_SQUARE_MODE, PIT_COMMAND);
    outb((uint8_t)(param & 0xFF), PIT_CHANNEL_0);           /* send the frequency byte by byte */
    outb((uint8_t)((param >> 8) & 0xFF), PIT_CHANNEL_0);    /* shift right and reserve last 8 bytes*/

    enable_irq(0);                                          /* enable the interrupt 0x20 in PIC */
}

void pit_handler() {
    send_eoi(0);            /* send eoi before handling it */
    
    struct pcb* current = current_pcb();
    uint32_t current_pid = current->pid, current_terminal;
    if (current->pid >= NUM_TERMINAL) {             /* program is running on the terminal */
        current_terminal = current->parent->pid;    /* the terminal is the parent */
    } else {
        current_terminal = current_pid;             /* the terminal is idle */
    }

    uint32_t next_terminal = (current_terminal + 1) % 2;
    uint32_t next_pid;
    if (terminals[next_terminal].pid >= NUM_TERMINAL) { /* the terminal is running a program*/
        next_pid = terminals[next_terminal].pid;
    } else {
        next_pid = next_terminal;                       /* the terminal is idle */
    }

    if (next_terminal == active_terminal) {             /* show the content */
        page_table[VIDEO_MEMORY_PTE].page_base_address = VIDEO_MEMORY_PTE;
    } else {                                            /* don't need to show, but need to update */
        page_table[VIDEO_MEMORY_PTE].page_base_address = VIDEO_MEMORY_PTE + next_terminal + 2;
    }

    context_switch(next_pid);                           /* switch to that process */
}