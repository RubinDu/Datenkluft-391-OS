#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "common_asm_link.h"

uint8_t exception_occurred = 0;

typedef void (*exceptions)();

/* 
 * EXPX/systemcall_blank
 *   DESCRIPTION: Handlers for exceptions and unimplemented system call
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Prints corresponding exception (or unimplemented system call) and freezes the kernel
 */
void EXP0(){
    printf(" Exception: Divide by zero\n");     //prints the exception
    exception_occurred = 1;
    halt(255);                                 
}
void EXP1(){
    printf(" Exception: Debug\n");
    exception_occurred = 1;
    halt(255);
}
void EXP2(){
    printf(" Exception: Non-maskable interrupt\n");
    exception_occurred = 1;
    halt(255);
}
void EXP3(){
    printf(" Exception: Breakpoint\n");
    exception_occurred = 1;
    halt(255);
}
void EXP4(){
    printf(" Exception: Overflow\n");
    exception_occurred = 1;
    halt(255);
}
void EXP5(){
    printf(" Exception: Bound Range Exceeded\n");
    exception_occurred = 1;
    halt(255);
}
void EXP6(){
    printf(" Exception: Invalid Opcode\n");
    exception_occurred = 1;
    halt(255);
}
void EXP7(){
    printf(" Exception: Device Not Available\n");
    exception_occurred = 1;
    halt(255);
}
void EXP8(){
    printf(" Exception: Double Fault\n");
    exception_occurred = 1;
    halt(255);
}
void EXP9(){
    printf(" Exception: Coprocessor Segment Overrun\n");
    exception_occurred = 1;
    halt(255);
}
void EXPA(){
    printf(" Exception: Invalid TSS\n");
    exception_occurred = 1;
    halt(255);
}
void EXPB(){
    printf(" Exception: Segment Not Present\n");
    exception_occurred = 1;
    halt(255);
}
void EXPC(){
    printf(" Exception: Stack-Segment Fault\n");
    exception_occurred = 1;
    halt(255);
}
void EXPD(){
    printf(" Exception: General Protection\n");
    exception_occurred = 1;
    halt(255);
}
void EXPE(){
    uint32_t page_fault_linear_addr;
    asm volatile (
        "movl %%cr2, %0" : "=r" (page_fault_linear_addr)
                         :
    );
    printf(" Exception: Page Fault, at %d\n", page_fault_linear_addr);
    exception_occurred = 1;
    halt(255);
}
void EXPF(){
    printf(" Exception: Reserved\n");
    exception_occurred = 1;
    halt(255);
}
void EXP10(){
    printf(" Exception: x87 FPU Floating-Point Error\n");
    exception_occurred = 1;
    halt(255);
}
void EXP11(){
    printf(" Exception: Alignment Check\n");
    exception_occurred = 1;
    halt(255);
}
void EXP12(){
    printf(" Exception: Machine Check\n");
    exception_occurred = 1;
    halt(255);
}
void EXP13(){
    printf(" Exception: SIMD Floating-Point Exception\n");
    exception_occurred = 1;
    halt(255);
}

/* 
 * idt_init
 *   DESCRIPTION: Initializes interrupt descripter table
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Fills the entries and loads them into IDT
 */
void idt_init(){
    int i;
    exceptions EXP[20] = {EXP0, EXP1, EXP2, EXP3, EXP4, EXP5, EXP6, EXP7, EXP8, EXP9, EXPA, EXPB, EXPC, EXPD, EXPE, EXPF, EXP10, EXP11, EXP12, EXP13};  //Function pointers to the handlers
    for (i = 0; i < NUM_VEC; i++){
        idt[i].present = 0;                 //traverse and set all entries
        idt[i].size = 1;
        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;
        idt[i].reserved4 = 0;
        idt[i].seg_selector = KERNEL_CS;   
        if (i<EXCEPTION_SIZE){              //if the given entry is for exceptions, set present, dpl to kernel, and link to corresponding handler
            idt[i].present = 1;
            idt[i].dpl = 0;
            SET_IDT_ENTRY(idt[i], EXP[i]);
        }
        if (i==SYSTEMCALL){                 //if the given entry is for system call, set present, dpl to applications, and link to corresponding handler
            idt[i].present = 1;
            idt[i].dpl = 3;
            idt[i].reserved3 = 1;
            SET_IDT_ENTRY(idt[i], system_call);
        }
        if (i==KEYBOARD){                   //if the given entry is for keyboard, set present, dpl to kernel, and link to corresponding handler             
            idt[i].present = 1;
            idt[i].dpl = 0;
            SET_IDT_ENTRY(idt[i], keyboard_intr);
        }
        if (i==RTC){                        //if the given entry is for RTC, set present, dpl to kernel, and link to corresponding handler     
            idt[i].present = 1;
            idt[i].dpl = 0;
            SET_IDT_ENTRY(idt[i], rtc_intr);
        }
        if (i == PIT) {
            idt[i].present = 1;
            idt[i].dpl = 0;
            SET_IDT_ENTRY(idt[i], pit_intr);
        }
    }
    lidt(idt_desc_ptr);                     //load the IDT
}
