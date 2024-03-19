#ifndef COMMON_ASM
#define COMMON_ASM

#ifndef ASM

#include "types.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "idt.h"
#include "x86_desc.h"

/* 
 * keyboard_intr
 *   DESCRIPTION: Masks interrupt flags, saves all, call the keyboard handler, restores all, return from the interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Calls the keyboard handler
 */
extern void keyboard_intr();

/* 
 * rtc_intr
 *   DESCRIPTION: Masks interrupt flags, saves all, call the RTC handler, restores all, return from the interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Calls the RTC handler
 */
extern void rtc_intr();

#endif
#endif
