#ifndef TERMINAL
#define TERMINAL
#include "types.h"

/*
* terminal_read
*   DESCRIPTION: Read user keyboard inputs
*   INPUTS: file - file descriptor (not used)
*           nbytes - number of bytes to read
*   OUTPUTS: buf - buffer to store the input
*   RETURN VALUE: bytes read
*   SIDE EFFECTS: While user haven't pressed enter, spins and waits for input
*/
int32_t terminal_read(int32_t file, void* buf, int32_t nbytes);

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
int32_t terminal_write(int32_t file, const void* buf, int32_t nbytes);

/*
* terminal_open
*   DESCRIPTION: Open specified terminal
*   INPUTS: file - not used
*   OUTPUTS: none
*   RETURN VALUE: 0 for now
*   SIDE EFFECTS: none
*/
int32_t terminal_open(const uint8_t* file);

/*
* terminal_close
*   DESCRIPTION: Close specified terminal
*   INPUTS: file - not used
*   OUTPUTS: none
*   RETURN VALUE: 0 for now
*   SIDE EFFECTS: none
*/
int32_t terminal_close(int32_t file);

/*
* end_of_line
*   DESCRIPTION: Helper function to handle end of line
*   INPUTS: buf - char read from keyboard
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: Resumes terminal_read, return to idle state at completion
*/
void end_of_line();

#endif /* _TERMINAL_H */
