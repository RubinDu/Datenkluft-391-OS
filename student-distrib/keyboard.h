#ifndef KEY_H
#define KEY_H

#define KEYBOARD_IRQ 0x1
#define KEYBOARD_PORT 0x60

void keyboard_init();
void keyboard_handler();

#endif
