#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "lib.h"
#include "i8259.h"

// RTC ports from https://wiki.osdev.org/RTC
#define RTC_COMMAND     0x70
#define RTC_DATA        0x71

#define REG_A           0x8A
#define REG_B           0x8B
#define REG_C           0x0C

#define RTC_IRQ         8

void rtc_init();
void rtc_handler();
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_set_rate(int32_t rate);


#endif
