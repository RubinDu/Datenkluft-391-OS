#include "rtc.h"

#define MIN_RATE 6      /* as the instructor said, there would be problem a*/
#define MAX_RATE 15     /* the maximum rate, which corresponds to 2 Hz, or 0.5 s/int*/

#define MIN_FREQUENCY (0x8000 >> (MAX_RATE - 1)) /* 2 Hz */
#define MAX_FREQUENCY (0x8000 >> (MIN_RATE - 1)) /* 1024 Hz*/

uint32_t rtc_occurred;

volatile struct rtc_info_t {
    int32_t enabled; /* 1 if rtc is enabled, 0 otherwise */
    int32_t current; /* the current progress to a milestone */
    int32_t frequency; /* how many interrupts a logical rtc interrupt should occur */
} rtc_info;

/*
* rtc_init
*   DESCRIPTION: Initialize the RTC
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: unmarks the rtc iqr
*/
void rtc_init(){
    cli();
    outb(REG_B, RTC_COMMAND); // Select Register B, and disable NMI (by setting the 0x80 bit)
    char prev = inb(RTC_DATA); // Read the current value of Register B
    outb(REG_B, RTC_COMMAND);
    outb(prev | 0x40, RTC_DATA); // Turn on bit 6 of register B to enable periodic interrupts
    enable_irq(RTC_IRQ);
    sti();
    rtc_set_rate(MIN_RATE); /* set the maximum acceptable rate, corresponding to 1024 Hz */
    rtc_info.enabled = 0;   /* rtc is logically disabled at first */
}

/*
* rtc_handler
*   DESCRIPTION: Handles the rtc interrupt
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: reads the register C and sets the flag to indicate interrupt occurred
*/
void rtc_handler(){
    cli();
    outb(REG_C, RTC_COMMAND); // Select and flush Register C
    inb(RTC_DATA);
    if (rtc_info.enabled) {
        if (rtc_info.current <= 0) {
            rtc_info.current = rtc_info.frequency;
        } else {
            --rtc_info.current;
        }
    }
    send_eoi(RTC_IRQ);
    sti();
}

/*
* rtc_set_rate
*   DESCRIPTION: Set the rate of the RTC
*   INPUTS: rate - the rate to set the RTC to
*   OUTPUTS: none
*   RETURN VALUE: 0 if successful, -1 if not
*   SIDE EFFECTS: changes the frequency of the RTC
*/
int32_t rtc_set_rate(int32_t rate){
    if (rate < MIN_RATE || rate > MAX_RATE){
        return -1; // can only be within 0-15
    }
    cli();
    outb(REG_A, RTC_COMMAND);  // Select Register A, but do not disable NMI (0x80 bit is not set)
    char prev = inb(RTC_DATA); // Read the current value of Register A
    char new_value = (prev & 0xF0) | (rate); // Prepare to write the new rate by preserving the top 4 bits and updating the bottom 4 bits
    outb(REG_A, RTC_COMMAND); // Write the new rate value back to Register A
    outb(new_value, RTC_DATA); // Re-select Register A in case the index was changed
    sti();
    return 0;
}

int32_t rtc_open(const uint8_t* filename){
    rtc_info.enabled = 1;
    rtc_info.frequency = MAX_FREQUENCY / MIN_FREQUENCY;
    return 0;
}

int32_t rtc_close(int32_t fd){
    rtc_info.enabled = 0;
    return 0;
}

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    if (nbytes != sizeof(int32_t) || buf == NULL) { /* the size or the buffer is incorrect */
        return -1;
    }
    int32_t frequency = *((int32_t*)buf);
    if (frequency < MIN_FREQUENCY
        || frequency > MAX_FREQUENCY
        || (frequency & (frequency - 1))) { /* zero if power of 2 */
        return -1;
    }
    rtc_info.current = rtc_info.frequency = (MAX_FREQUENCY / frequency);
    return 0;
}

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    while (1) {
        if (rtc_info.enabled && rtc_info.current == 0) {
            // printf("Interrupt Reached\n");
            --rtc_info.current;
            return 0;
        }
    }
}
