#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "common_asm_link.h"

typedef void (*exceptions)();

void EXP0(){
    cli();
    clear();
    printf(" Exception: Divide by zero\n");
    while(1);
}
void EXP1(){
    cli();
    clear();
    printf(" Exception: Debug\n");
    while(1);
}
void EXP2(){
    cli();
    clear();
    printf(" Exception: Non-maskable interrupt\n");
    while(1);
}
void EXP3(){
    cli();
    clear();
    printf(" Exception: Breakpoint\n");
    while(1);
}
void EXP4(){
    cli();
    clear();
    printf(" Exception: Overflow\n");
    while(1);
}
void EXP5(){
    cli();
    clear();
    printf(" Exception: Bound Range Exceeded\n");
    while(1);
}
void EXP6(){
    cli();
    clear();
    printf(" Exception: Invalid Opcode\n");
    while(1);
}
void EXP7(){
    cli();
    clear();
    printf(" Exception: Device Not Available\n");
    while(1);
}
void EXP8(){
    cli();
    clear();
    printf(" Exception: Double Fault\n");
    while(1);
}
void EXP9(){
    cli();
    clear();
    printf(" Exception: Coprocessor Segment Overrun\n");
    while(1);
}
void EXPA(){
    cli();
    clear();
    printf(" Exception: Invalid TSS\n");
    while(1);
}
void EXPB(){
    cli();
    clear();
    printf(" Exception: Segment Not Present\n");
    while(1);
}
void EXPC(){
    cli();
    clear();
    printf(" Exception: Stack-Segment Fault\n");
    while(1);
}
void EXPD(){
    cli();
    clear();
    printf(" Exception: General Protection\n");
    while(1);
}
void EXPE(){
    cli();
    clear();
    printf(" Exception: Page Fault\n");
    while(1);
}
void EXPF(){
    cli();
    clear();
    printf(" Exception: Reserved\n");
    while(1);
}
void EXP10(){
    cli();
    clear();
    printf(" Exception: x87 FPU Floating-Point Error\n");
    while(1);
}
void EXP11(){
    cli();
    clear();
    printf(" Exception: Alignment Check\n");
    while(1);
}
void EXP12(){
    cli();
    clear();
    printf(" Exception: Machine Check\n");
    while(1);
}
void EXP13(){
    cli();
    clear();
    printf(" Exception: SIMD Floating-Point Exception\n");
    while(1);
}

void systemcall_blank(){
    clear();
    printf(" Unimplemented System Call\n");
    while(1);
}

void idt_init(){
    int i;
    exceptions EXP[20] = {EXP0, EXP1, EXP2, EXP3, EXP4, EXP5, EXP6, EXP7, EXP8, EXP9, EXPA, EXPB, EXPC, EXPD, EXPE, EXPF, EXP10, EXP11, EXP12, EXP13};
    for (i = 0; i < IDT_SIZE; i++){
        idt[i].present = 0;
        idt[i].size = 1;
        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;
        idt[i].reserved4 = 0;
        idt[i].seg_selector = KERNEL_CS;
        if (i<EXCEPTION_SIZE){
            idt[i].present = 1;
            idt[i].dpl = 0;
            SET_IDT_ENTRY(idt[i], EXP[i]);
        }
        if (i==SYSTEMCALL){
            idt[i].present = 1;
            idt[i].dpl = 3;
            SET_IDT_ENTRY(idt[i], systemcall_blank);
        }
        if (i==0x21){
            idt[i].present = 1;
            idt[i].dpl = 0;
            SET_IDT_ENTRY(idt[i], keyboard_intr);
        }
        if (i==0x28){
            idt[i].present = 1;
            idt[i].dpl = 0;
            SET_IDT_ENTRY(idt[i], rtc_intr);
        }
    }
    lidt(idt_desc_ptr);
}
