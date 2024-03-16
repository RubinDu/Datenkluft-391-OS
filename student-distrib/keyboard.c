#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

static sc=0;
static char keys[54]={
    0,   0,   '1', '2', '3', '4', '5', '6', 
    '7', '8', '9', '0', '-', '=', 0,   0,
    '\t','q', 'w', 'e', 'r', 't', 'y', 'i', 
    'o', 'p', '[', ']', '\n', 0 , 'a', 's', 
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 
    '\'', '`', 0,  '\\','z', 'x', 'c', 'v', 
    'b', 'n', 'm', ',', '.', '/'};

void keyboard_init(){
    enable_irq(0x1);
}

void keyboard_handler(){
    sc = inb(0x60);
    if (sc<54){
        clear();
        printf("%c", keys[sc]);
    }
    send_eoi(0x1);
}