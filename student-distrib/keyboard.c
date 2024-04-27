#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "system_call.h"
#include "x86_desc.h"
#include "pit.h"

static char sc=0;                               //Scan char buffer
static char lshift = 0;        
static char rshift = 0;        
static char cap = 0;          
static char cap_released = 1; 
static char ctrl = 0;         
static char az = 0;
static char alt = 0;
static terminal_t *active_terminal;                      //Buffer for read
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
    memset(get_terminal(0)->terminal_buf, '\n', READBUF_SIZE);
    memset(get_terminal(1)->terminal_buf, '\n', READBUF_SIZE);
    memset(get_terminal(2)->terminal_buf, '\n', READBUF_SIZE);
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
    active_terminal = get_terminal(*get_active_terminal());
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
        if (active_terminal->num_echoed>0){
            echo('\b');
            active_terminal->num_echoed --;
            if (active_terminal->num_echoed<READBUF_SIZE-1)
                active_terminal->terminal_buf[active_terminal->num_echoed]='\n';
        }
        break;
    case 0x1C:      //Enter pressed
        echo('\n');
        end_of_line();    // Return contents in the read buffer to terminal and resume terminal_read
        active_terminal->num_echoed = 0;
        memset(active_terminal->terminal_buf, '\n', READBUF_SIZE);    //Resets the read buffer to all newlines
        break;
    case 0x38:      //Alt pressed
        alt = 1;
        break;
    case 0xB8:      //Alt released
        alt = 0;
        break;
    default:
            if (ctrl){
                switch (sc){
                case 0x26:      //Ctrl + L
                    clear();
                    break;
                case 0x2E:      //Ctrl + C
                    active_terminal->halt = 1;      //Tell scheduler to halt the process running on active terminal
                    break;
                default:
                    break;
                }
            }
            else if (alt)
            {
                switch (sc){
                case 0x3B:          //Alt + F1
                    switch_terminal(0);
                    pit_handler();  //Force a context switch to ensure paging isn't going to be messed up by a second switch, and also runs smoother
                    break;
                case 0x3C:          //Alt + F2
                    switch_terminal(1);
                    pit_handler();
                    break;
                case 0x3D:          //Alt + F3
                    switch_terminal(2);
                    pit_handler();
                    break;
                default:
                    break;
                }
            }
            else if (sc < KEYS_SIZE && keys[(uint32_t)sc]){
                az=keys[(uint32_t)sc] >= 'a' && keys[(uint32_t)sc] <= 'z';      //Check if the key pressed is an alphabet
                if ((az&&(!(lshift||rshift)!=!cap))||(!az&&(lshift||rshift))){    //If alphabet, check if only one of shift and cap is active. If not, check if shift is active
                    if (active_terminal->num_echoed<READBUF_SIZE-1)                              //If buffer isn't full, put character into buffer
                        active_terminal->terminal_buf[active_terminal->num_echoed]=keys_shifted[(uint32_t)sc];
                    echo(keys_shifted[(uint32_t)sc]);                           //Print shifted character
                }
                else{                           
                    if (active_terminal->num_echoed<READBUF_SIZE-1)                              //If buffer isn't full, put character into buffer        
                        active_terminal->terminal_buf[active_terminal->num_echoed]=keys[(uint32_t)sc];
                    echo(keys[(uint32_t)sc]);                                   //Print character         
                }
                active_terminal->num_echoed++;
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
    get_terminal(*get_current_terminal())->num_echoed = 0;
    memset(get_terminal(*get_current_terminal())->terminal_buf, '\n', READBUF_SIZE);    //Resets the read buffer to all newlines
}
