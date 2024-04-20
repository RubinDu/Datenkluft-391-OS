#include "pit.h"
#include "common_asm_link.h"
#include "i8259.h"

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

int pit_count = 1;
void pit_handler() {
    send_eoi(0);

    // if (++pit_count == 100) {
    //     printf("PIT!");
    //     pit_count = 1;
    // }

    // TODO: Task Switch
}