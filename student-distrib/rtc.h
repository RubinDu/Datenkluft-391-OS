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

#define MIN_RATE 7      /* as the instructor said, there would be problem a*/
#define MAX_RATE 15     /* the maximum rate, which corresponds to 2 Hz, or 0.5 s/int*/

#define MIN_FREQUENCY (0x8000 >> (MAX_RATE - 1)) /* 2 Hz */
#define MAX_FREQUENCY (0x8000 >> (MIN_RATE - 1)) /* 512 Hz*/

typedef struct {
    volatile int32_t current;   /* the current progress to a milestone */
    volatile int32_t rate;      /* how many interrupts a logical rtc interrupt should occur */
} rtc_info_t;

/* initializes the rtc driver */
void rtc_init();

/* rtc interrupt handler */
void rtc_handler();

/* opens an rtc file */
int32_t rtc_open(const uint8_t* filename);

/* closes an rtc file */
int32_t rtc_close(int32_t fd);

/* updates the frequency of the rtc */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* returns only when rtc interrupt occurs */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* set the rate of the rtc driver */
int32_t rtc_set_rate(int32_t rate);


#endif
