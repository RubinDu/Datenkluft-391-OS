.text
.globl keyboard_intr

keyboard_intr:
    cli
    pushfl
    pushal
    call keyboard_handler
    popal
    popfl
    sti
    iret
