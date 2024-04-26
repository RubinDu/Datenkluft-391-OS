#include "rtc.h"
#include "system_call.h"

rtc_info_t rtc_info[3] = {
    {.enabled = 0, .current = 0, .rate = MAX_FREQUENCY / MIN_FREQUENCY},
    {.enabled = 0, .current = 0, .rate = MAX_FREQUENCY / MIN_FREQUENCY},
    {.enabled = 0, .current = 0, .rate = MAX_FREQUENCY / MIN_FREQUENCY},
};

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
    outb(REG_C, RTC_COMMAND); /* throw away the value, or it would continue waiting */
    inb(RTC_DATA);
    
    int i;
    // rtc_info_t *info;
    // for (i = 0, info = rtc_info; i <= 2; ++i, ++info) {
    //     if (!info->enabled) {
    //         continue;
    //     }
    //     if (info->current <= 1) {
    //         info->current = info->rate;
    //     } else {
    //         --info->current;
    //     }
    // }

    pcb_t *pcb;
    for (i = 3, pcb = GET_PCB(i); i < 6; ++i, pcb = GET_PCB(i)) {
        if (pcb->present && pcb->rtc && !pcb->rtc_det) {
            if (pcb->rtc_curr <= 1) {
                pcb->rtc_det = 1;
            } else {
                --pcb->rtc_curr;
            }
        }
    }

    send_eoi(RTC_IRQ);
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

/**
 * rtc_open
 * DESCRIPTION: Creates an rtc file
 * INPUTS: filename - ignored
 * OUTPUTS: none
 * RETURN: 0 after creation
 * SIDE EFFECTS: If another rtc is already open, this would reset the frequency
 */
int32_t rtc_open(const uint8_t* filename){
    pcb_t *pcb = current_pcb();
    pcb->rtc = 1;
    pcb->rtc_curr = pcb->rtc_rate = MAX_FREQUENCY / MIN_FREQUENCY;
    return 0;
}

/**
 * rtc_open
 * DESCRIPTION: Closes an rtc file
 * INPUTS: fd - ignored
 * OUTPUTS: none
 * RETURN: 0 after closing
 * SIDE EFFECTS: none
 */
int32_t rtc_close(int32_t fd){
    pcb_t *pcb = current_pcb();
    pcb->rtc = 0;
    return 0;
}

/**
 * rtc_write
 * DESCRIPTION: Resets the frequency of an rtc clock
 * INPUTS: fd - ignored
 *         buf - a pointer of an int32_t frequency, must be power of 2
 *         nbytes - the size of the buffer, must be sizeof(int32_t)
 * OUTPUTS: none
 * RETURN: 0 if scuccessfully, -1 otherwise
 * SIDE EFFECTS: none
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    if (nbytes != sizeof(int32_t) || buf == NULL) { /* the size or the buffer is invalid */
        return -1;
    }
    int32_t frequency = *((int32_t*)buf);
    if (frequency < MIN_FREQUENCY           /* frequency < 2Hz */
        || frequency > MAX_FREQUENCY        /* frequency > 512 Hz */
        || (frequency & (frequency - 1))) { /* not power of 2 */
        return -1;
    }
    
    pcb_t *pcb = current_pcb();
    if (!pcb->rtc) {
        return -1;
    }
    pcb->rtc_curr = pcb->rtc_rate = MAX_FREQUENCY / frequency;

    // rtc_info_t *info = rtc_info + *get_current_terminal();
    // if (info->enabled == 0) {
    //     return -1;
    // }
    // info->current = info->rate = MAX_FREQUENCY / frequency;

    return 0;
}

/**
 * rtc_read
 * DESCRIPTION: Checks if a logical rtc interrupt has occurred
 * INPUTS: fd - ignored
 *         buf - ignored
 *         nbytes - ignored
 * OUTPUTS: none
 * RETURN: 0 if rtc interrupt has occurred, wait otherwise
 * SIDE EFFECTS: none
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    pcb_t *pcb = current_pcb();
    while (0xECE391) {
        // if (info->enabled == 1 && info->current == 1) {
        if (pcb->rtc == 1 && pcb->rtc_det == 1) {
            pcb->rtc_det = 0;
            pcb->rtc_curr = pcb->rtc_rate;
            return 0;
        }
    }
}
