#define ASM     1
#include "common_asm_link.h"

.text
.globl keyboard_intr, rtc_intr, system_call

sc_table:
    .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

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

# System call linkage
# Saves all, checks if the system call is valid, calls the corresponding handler, restores all, return with return value in eax
system_call:
    pushfl
    pushal
    subl $1, %eax
    cmpl $9, %eax
    ja bad_sc
    pushl %edx
    pushl %ecx
    pushl %ebx
    call *sc_table(,%eax,4)
    addl $12, %esp
    movl %eax, 32(%esp)
    popal
    popfl
    iret

bad_sc:
    popal
    popfl
    movl $-1, %eax
    iret
