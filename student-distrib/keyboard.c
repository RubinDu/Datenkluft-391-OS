#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

static char sc=0;                               //Scan char buffer
static char keys[54]={                          //ps/2 keyboard scan char map for numbers, alphabet, and some symbols
    0,   0,   '1', '2', '3', '4', '5', '6', 
    '7', '8', '9', '0', '-', '=', 0,   0,
    '\t','q', 'w', 'e', 'r', 't', 'y', 'i', 
    'o', 'p', '[', ']', '\n', 0 , 'a', 's', 
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
    '\'', '`', 0,  '\\','z', 'x', 'c', 'v', 
    'b', 'n', 'm', ',', '.', '/'};

/* 
 * keyboard_init
 *   DESCRIPTION: Initializes keyboard interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Unmasks keyboard IRQ
 */
void keyboard_init(){                    
    enable_irq(KEYBOARD_IRQ);    //Keyboard is connected to IR1 of the master PIC, which is irq 1
}

/* 
 * keyboard_handler
 *   DESCRIPTION: Handler for keyboard interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Reads from keyboard port 0x60 and prints the char (if valid) to screen
 */
void keyboard_handler(){
    sc = inb(KEYBOARD_PORT);     //Scans from keyboard port 0x60
    if (sc<54){         //if valid, print to screen
        clear();
        printf("%c", keys[sc]);
    }
    send_eoi(0x1);      //Send end of interrupt
}
