#include "rtc.h"


volatile uint32_t INTERRUP;

void rtc_init(){
    cli();
    disable_irq(RTC_IRQ);
    outb(REG_B, RTC_COMMAND); // Select Register B, and disable NMI (by setting the 0x80 bit)
    char prev = inb(RTC_DATA); // Read the current value of Register B
    outb(REG_B, RTC_COMMAND);
    outb(prev | 0x40, RTC_DATA); // Turn on bit 6 of register B to enable periodic interrupts

    // set initial rate to 2 Hz
    rtc_set_rate(RTC_RATE_2);

    // outb(REG_A,RTC_COMMAND);
    // outb(RTC_RATE_2,RTC_DATA); // Set the rate at which periodic interrupts occur
    enable_irq(RTC_IRQ);
    sti();
}

void rtc_handler(){
    cli();
    outb(REG_C, RTC_COMMAND); // Select Register C
    inb(RTC_DATA);
    INTERRUP = 1; // Set the flag to indicate that an interrupt has occurred
    printf("RTC interrupt\n");
    send_eoi(RTC_IRQ);
    sti();
}

int32_t rtc_set_rate(int32_t rate){
    if (rate < 2 || rate > 1024 || (rate & (rate-1))!= 0){
        return -1; // [2,1024] && power of 2
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
    return 0;
}

int32_t rtc_close(int32_t fd){
    return 0;
}

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    return 0;
}

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    return 0;
}

