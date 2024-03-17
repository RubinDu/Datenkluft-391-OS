#ifndef COMMON_ASM
#define COMMON_ASM

#ifndef ASM

#include "types.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "idt.h"
#include "x86_desc.h"

extern void keyboard_intr();
extern void rtc_intr();

#endif
#endif
