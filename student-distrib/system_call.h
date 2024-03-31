#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H

#include "types.h"
#include "filesys.h"
#include "paging.h"
#include "keyboard.h"

#define MAX_FILES 8
#define MAGIC_SIZE 4
#define MAGIC_NUM 0x7F454C46
#define USER_ENTRY 32
#define MAX_TASKS 2

#define GET_PCB(pid) ((pcb_t*)(0x7FE000-0x2000*pid))

typedef struct pcb{
    file_descriptor_t file_arr[MAX_FILES];
    uint8_t present;
    uint32_t pid;
    uint32_t uesp;
    uint32_t uebp;
    uint32_t kesp;
    uint32_t kss;
    char args[READBUF_SIZE];
}pcb_t;

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
