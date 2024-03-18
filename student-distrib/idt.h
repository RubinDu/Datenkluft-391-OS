#ifndef IDT_H
#define IDT_H

#define EXCEPTION_SIZE 0x14
#define SYSTEMCALL 0x80
#define KEYBOARD 0x21
#define RTC 0x28

void EXP0();
void EXP1();
void EXP2();
void EXP3();
void EXP4();
void EXP5();
void EXP6();
void EXP7();
void EXP8();
void EXP9();
void EXPA();
void EXPB();
void EXPC();
void EXPD();
void EXPE();
void EXPF();
void EXP10();
void EXP11();
void EXP12();
void EXP13();
void systemcall_blank();

extern void idt_init();

#endif