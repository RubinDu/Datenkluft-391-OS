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
    pushl %esi
    pushl %edi
    pushl %esp
    pushl %ebp
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushw %fs
    pushw %es
    pushw %ds
    pushw $0

    subl $1, %eax               # eax := interrupt number
    cmpl $9, %eax
    ja bad_sc

    movw $0x0018, %si
    movw %si, %ds

    pushl %edx
    pushl %ecx
    pushl %ebx
    call *sc_table(, %eax, 4)   # call specific functions
    addl $12, %esp
    # movl %eax, 32(%esp)         # the return value
    jmp sc_done

bad_sc:
    movl $-1, %eax

sc_done:
    addl $2, %esp
    popw %ds
    popw %es
    popw %fs

    popl %edx
    popl %ecx
    popl %ebx
    popl %ebp
    popl %esp
    popl %edi
    popl %esi

    iret
