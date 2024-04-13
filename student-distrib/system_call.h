#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H

#include "types.h"
#include "paging.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "filesys.h"

#define MAX_FILES 8
#define MAGIC_SIZE 4
#define MAGIC_NUM 0x464C457F
#define USER_ENTRY 32
#define MAX_TASKS 3
#define PROGRAM_IMAGE_ADDR 0x8048000        /* virtual address of the program image */
#define PROGRAM_IMAGE_LIMIT 0x3B8000        /* limit of size of program image */
#define USER_STACK 0x8400000
#define KSTACK_START 0x800000
#define KSTACK_SIZE 0x2000

#define GET_PCB(pid) ((pcb_t*)(KSTACK_START - KSTACK_SIZE - KSTACK_SIZE * pid))

typedef struct pcb {
    file_descriptor_t fd[MAX_FILES];
    uint8_t present;
    uint32_t pid;
    struct pcb* parent;
    uint32_t esp;
    uint32_t ebp;
    char args[READBUF_SIZE];
}pcb_t;


int32_t null_read(int32_t fd, void* buf, int32_t nbytes);
int32_t null_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t null_open(const uint8_t* filename);
int32_t null_close(int32_t fd);

static const struct file_operations stdin_op = {
    .open = terminal_open,
    .read = terminal_read,
    .write = null_write,
    .close = terminal_close
};

static const struct file_operations stdout_op = {
    .open = terminal_open,
    .read = null_read,
    .write = terminal_write,
    .close = terminal_close
};

static const struct file_operations rtc_op = {
    .open = rtc_open,
    .read = rtc_read,
    .write = rtc_write,
    .close = rtc_close
};

static const struct file_operations file_op = {
    .open = file_open,
    .read = file_read,
    .write = file_write,
    .close = file_close
};

static const struct file_operations dir_op = {
    .open = dir_open,
    .read = dir_read,
    .write = dir_write,
    .close = dir_close
};

static const struct file_operations null_op = {
    .open = null_open,
    .read = null_read,
    .write = null_write,
    .close = null_close
};

pcb_t* current_pcb();

int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);


#endif
