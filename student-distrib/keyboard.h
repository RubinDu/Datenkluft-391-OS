#ifndef KEY_H
#define KEY_H

#define KEYBOARD_IRQ 0x1
#define KEYBOARD_PORT 0x60
#define READBUF_SIZE 128
#define KEYS_SIZE 58

/* 
 * keyboard_init
 *   DESCRIPTION: Initializes keyboard interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Unmasks keyboard IRQ
 */
void keyboard_init();

/* 
 * keyboard_handler
 *   DESCRIPTION: Handler for keyboard interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Reads from keyboard port 0x60 and prints the char (if valid) to screen
 */
void keyboard_handler();

/* 
 * reset_buf
 *   DESCRIPTION: Reset read buffer and counter upon a new terminal_read
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Clears read buffer and counter
 */
void reset_buf();

#endif
