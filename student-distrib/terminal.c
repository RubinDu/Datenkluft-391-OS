#include "terminal.h"
#include "keyboard.h"
#include "lib.h"
#include "system_call.h"
#include "x86_desc.h"

static char readbuf[READBUF_SIZE];            // pointer to the read buffer
static int32_t readcount;        // number of bytes to read


/*
* terminal_read
*   DESCRIPTION: Read user keyboard inputs
*   INPUTS: file - file descriptor (not used)
*           nbytes - number of bytes to read
*   OUTPUTS: buf - buffer to store the input
*   RETURN VALUE: bytes read
*   SIDE EFFECTS: While user haven't pressed enter, spins and waits for input
*/
int32_t terminal_read(int32_t file, void* buf, int32_t nbytes){
    terminal_t *reading_terminal=get_terminal(*get_current_terminal());
    readcount = nbytes;
    reset_buf();        // Reset the keyboard buffer
    reading_terminal->idle=0;           // Set status to reading
    while (!(reading_terminal->idle)||*get_current_terminal()!=*get_active_terminal());      // Wait reading to finish
    memcpy(buf, readbuf, readcount);    // Copy the read buffer to the output buffer
    return readcount;   // Return actual number of bytes read
}

/*
* terminal_write
*   DESCRIPTION: Writes nbytes of buf to the terminal
*   INPUTS: file - file descriptor (not used)
*           buf - buffer containing data to write
*           nbytes - number of bytes to write
*   OUTPUTS: none
*   RETURN VALUE: The number of bytes written
*   SIDE EFFECTS: Outputs the content of buf to the terminal
*/
int32_t terminal_write(int32_t file, const void* buf, int32_t nbytes){
    cli();
    int i;
    for (i = 0; i < nbytes; i++)        // Traverse through the buffer
        if (((char*)buf)[i])            // Print to terminal if not null character
            putc(((char*)buf)[i]);
    reset_buf();                        // In case for asychronous write
    sti();
    return nbytes;
}

/*
* terminal_open
*   DESCRIPTION: Open specified terminal
*   INPUTS: file - not used
*   OUTPUTS: none
*   RETURN VALUE: 0 for now
*   SIDE EFFECTS: none
*/
int32_t terminal_open(const uint8_t* file){
    return 0;
}

/*
* terminal_close
*   DESCRIPTION: Close specified terminal
*   INPUTS: file - not used
*   OUTPUTS: none
*   RETURN VALUE: 0 for now
*   SIDE EFFECTS: none
*/
int32_t terminal_close(int32_t file){
    return 0;
}

/*
* end_of_line
*   DESCRIPTION: Helper function to handle end of line
*   INPUTS: buf - char read from keyboard
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: Resumes terminal_read, return to idle state at completion
*/
void end_of_line(){
    terminal_t *reading_terminal=get_terminal(*get_active_terminal());
    char* buf = reading_terminal->terminal_buf;
    if (reading_terminal->idle)                           // If terminal is idle, return
        return;
    int i;
    memset(readbuf, '\n', READBUF_SIZE);  // Clear the read buffer
    for (i = 0; i < readcount; i++){    // Traverse through the keyboard buffer, move characters to read buffer until size of read buffer reached or newline
        if (buf[i] == '\n'){
            i++;
            break;
        }
        ((char*)readbuf)[i] = buf[i];
    }
    ((char*)readbuf)[i-1] ='\n';        // Add newline to the end of the read buffer
    readcount = i;                      // Set actual number of bytes read
    reading_terminal->idle = 1;                           // Set terminal to idle
}
