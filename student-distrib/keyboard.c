#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "system_call.h"

static char sc=0;                               //Scan char buffer
static char lshift = 0;        
static char rshift = 0;        
static char cap = 0;          
static char cap_released = 1; 
static char ctrl = 0;         
static char az = 0;
static int num_echoed=0;
static char read_buf[READBUF_SIZE];                      //Buffer for read
static char keys[KEYS_SIZE]={                          //ps/2 keyboard scan char map for numbers, alphabet, and some symbols
    0,   0,   '1', '2', '3', '4', '5', '6', 
    '7', '8', '9', '0', '-', '=', 0,  '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u','i', 
    'o', 'p', '[', ']', 0, 0 , 'a', 's', 
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
    '\'', '`', 0,  '\\','z', 'x', 'c', 'v', 
    'b', 'n', 'm', ',', '.', '/',  0,   0, 
    0,' '};
static char keys_shifted[KEYS_SIZE]={                    //ps/2 keyboard scan char map for numbers, alphabet, and some symbols when shift is pressed
    0,   0,   '!', '@', '#', '$', '%', '^', 
    '&', '*', '(', ')', '_', '+', 0,  '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 
    'O', 'P', '{', '}', 0   , 0 , 'A', 'S', 
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', 
    '\"','~', 0,  '|', 'Z', 'X', 'C', 'V', 
    'B', 'N', 'M', '<', '>', '?', 0,    0, 
    0,   ' '};

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
    memset(read_buf, '\n', READBUF_SIZE);
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
    sc = inb(KEYBOARD_PORT);        //Scans from keyboard port 0x60
    switch (sc&0xFF)                //Only lower 8 bits are valid
    {
    case 0x2A:      //LShift pressed
        lshift =1;
        break;
    case 0xAA:      //LShift released
        lshift =0;
        rshift =0;
        break;
    case 0x36:      //RShift pressed
        rshift =1;
        break;
    case 0xB6:      //RShift released
        lshift =0;
        rshift =0;
        break;
    case 0x3A:      //Caps lock pressed
        if (cap_released){
            cap = !cap;
            cap_released = 0;
        }
        break;
    case 0xBA:      //Caps lock released
        cap_released = 1;
        break;
    case 0x1D:      //Ctrl pressed
        ctrl = 1;
        break;
    case 0x9D:      //Ctrl released
        ctrl = 0;
        break;
    case 0x0E:      //Backspace pressed
        if (num_echoed){
            putc('\b');
            num_echoed --;
            if (num_echoed<READBUF_SIZE-1)
                read_buf[num_echoed]='\n';
        }
        break;
    case 0x1C:      //Enter pressed
        putc('\n');
        end_of_line(read_buf);    // Return contents in the read buffer to terminal and resume terminal_read
        reset_buf();
        break;
    default:
        if (sc < KEYS_SIZE && keys[(uint32_t)sc]){
            if (ctrl){
                switch (sc){
                case 0x26:      //Ctrl + L
                    clear();
                    break;
                case 0x2E:      //Ctrl + C
                    reset_buf();
                    send_eoi(KEYBOARD_IRQ);      //Send end of interrupt
                    sti();
                    halt(128);
                    break;
                default:
                    break;
                }
            }
            else{
                az=keys[(uint32_t)sc] >= 'a' && keys[(uint32_t)sc] <= 'z';      //Check if the key pressed is an alphabet
                if ((az&&(!(lshift||rshift)!=!cap))||(!az&&(lshift||rshift))){    //If alphabet, check if only one of shift and cap is active. If not, check if shift is active
                    if (num_echoed<READBUF_SIZE-1)                              //If buffer isn't full, put character into buffer
                        read_buf[num_echoed]=keys_shifted[(uint32_t)sc];
                    putc(keys_shifted[(uint32_t)sc]);                           //Print shifted character
                }
                else{                           
                    if (num_echoed<READBUF_SIZE-1)                              //If buffer isn't full, put character into buffer        
                        read_buf[num_echoed]=keys[(uint32_t)sc];
                    putc(keys[(uint32_t)sc]);                                   //Print character         
                }
                num_echoed++;
            }
        }
        break;
    }
    send_eoi(KEYBOARD_IRQ);      //Send end of interrupt
}

/* 
 * reset_buf
 *   DESCRIPTION: Reset read buffer and counter upon a new terminal_read
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Clears read buffer and counter
 */
void reset_buf(){
    num_echoed = 0;
    memset(read_buf, '\n', READBUF_SIZE);    //Resets the read buffer to all newlines
}
