#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"

static char sc=0;                               //Scan char buffer
static char lshift = 0;        
static char rshift = 0;        
static char cap = 0;          
static char cap_released = 1; 
static char ctrl = 0;         
static char az = 0;
static int num_echoed=0;
static char read_buf[128];                      //Buffer for read
static char keys[57]={                          //ps/2 keyboard scan char map for numbers, alphabet, and some symbols
    0,   0,   '1', '2', '3', '4', '5', '6', 
    '7', '8', '9', '0', '-', '=', 0,   0,
    '\t','q', 'w', 'e', 'r', 't', 'y', 'i', 
    'o', 'p', '[', ']', 0, 0 , 'a', 's', 
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
    '\'', '`', 0,  '\\','z', 'x', 'c', 'v', 
    'b', 'n', 'm', ',', '.', '/', 0, 0, 0,
    ' '};
static char keys_shifted[57]={                    //ps/2 keyboard scan char map for numbers, alphabet, and some symbols when shift is pressed
    0,   0,   '!', '@', '#', '$', '%', '^', 
    '&', '*', '(', ')', '_', '+', 0,   0,
    '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'I', 
    'O', 'P', '{', '}', 0   , 0 , 'A', 'S', 
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', 
    '\"','~', 0,  '|', 'Z', 'X', 'C', 'V', 
    'B', 'N', 'M', '<', '>', '?', 0, 0, 0,
    ' '};

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
    memset(read_buf, '\n', 128);
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
    cli();
    sc = inb(KEYBOARD_PORT);     //Scans from keyboard port 0x60
    switch (sc&0xFF)
    {
    case 0x2A:      //LShift pressed
        lshift =1;
        break;
    case 0xAA:      //LShift released
        lshift =0;
        break;
    case 0x36:      //RShift pressed
        rshift =1;
        break;
    case 0xB6:      //RShift released
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
    case 0x0E:
        if (num_echoed){
            putc('\b');
            num_echoed --;
            read_buf[num_echoed]='\n';
        }
        break;
    case 0x1C:      //Enter pressed
        num_echoed = 0;
        putc('\n');
        end_of_line(read_buf);
        memset(read_buf, '\n', 128);
        break;
    default:
        if (sc < 57 && keys[(uint32_t)sc]){
            if (ctrl){
                switch (sc){
                case 0x26: 
                    clear();
                    break;
                default:
                    break;
                }
            }
            else{
                az=keys[(uint32_t)sc] >= 'a' && keys[(uint32_t)sc] <= 'z';
                if ((az&&(!(lshift||rshift)!=!cap))||(lshift||rshift)&&!az){
                    if (num_echoed<127)
                        read_buf[num_echoed]=keys_shifted[(uint32_t)sc];
                    putc(keys_shifted[(uint32_t)sc]);
                }
                else{
                    putc(keys[(uint32_t)sc]);
                    if (num_echoed<127)
                        read_buf[num_echoed]=keys[(uint32_t)sc];
                }
                num_echoed++;
            }
        }
        break;
    }
    send_eoi(KEYBOARD_IRQ);      //Send end of interrupt
    sti();
}
