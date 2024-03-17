#define ASM     1
#include "common_asm_link.h"

.text
.globl keyboard_intr, rtc_intr

keyboard_intr:
    cli
    pushfl
    pushal
    call keyboard_handler
    popal
    popfl
    sti
    iret

rtc_intr:
    cli
    pushfl
    pushal
    call rtc_handler
    popal
    popfl
    sti
    iret
