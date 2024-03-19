#define ASM     1
#include "common_asm_link.h"

.text
.globl keyboard_intr, rtc_intr

# Keyboard interrupt linkage
# Masks interrupt flags, saves all, call the corresponding handler, restores all, return from the interrupt, function headers in .h file
keyboard_intr:
    cli
    pushfl
    pushal
    call keyboard_handler
    popal
    popfl
    sti
    iret

# RTC interrupt linkage
# Masks interrupt flags, saves all, call the corresponding handler, restores all, return from the interrupt, function headers in .h file
rtc_intr:
    cli
    pushfl
    pushal
    call rtc_handler
    popal
    popfl
    sti
    iret
