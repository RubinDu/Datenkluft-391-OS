/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    // master_port/slave_port is command port
    // master_port+1/slave_port+1 is data port

    // mask all interrupts upfront
    outb(0xFF, MASTER_8259_PORT + 1);
    outb(0xFF, SLAVE_8259_PORT + 1);

    /* Start initialization sequence */
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    /* Set vector offset */
    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);

    /* Tell Master about Slave and vice versa */
    outb(ICW3_MASTER, MASTER_8259_PORT + 1);
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);

    /* 8086/88 mode */
    outb(ICW4, MASTER_8259_PORT + 1);
    outb(ICW4, SLAVE_8259_PORT + 1);

    /* Mask all interrupts */
    outb(0xFF, MASTER_8259_PORT + 1);
    outb(0xFF, SLAVE_8259_PORT + 1);

}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    cli();
    if(irq_num<0 || irq_num>15) return;
    if (irq_num < 8) {
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
    } else {
        slave_mask &= ~(1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
    sti();
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    cli();
    if(irq_num<0 || irq_num>15) return;
    if(irq_num < 8) {
        master_mask |= (1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
    } else {
        slave_mask |= (1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
    sti();
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num<0 || irq_num>15) return;
    if(irq_num < 8) {
        outb(EOI | irq_num, MASTER_8259_PORT);
    } else {
        outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
        outb(EOI | 2, MASTER_8259_PORT);
    }
}
