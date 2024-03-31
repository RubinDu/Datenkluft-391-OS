#include "system_call.h"
#include "lib.h"
#include "paging.h"
#include "filesys.h"

//TODO CP2

int32_t halt(uint8_t status){
    //Restore parent data
    //Restore parent paging
    //Clear file descriptor array
    //Write Parent process’ info back to TSS(esp0)
    //Jump to execute return 
    return -1;
}

int32_t execute(const uint8_t* command){
    int i;
    uint8_t* filename[READBUF_SIZE];
    uint8_t* args[READBUF_SIZE];
    dentry_t exec_dentry;
    uint32_t magic_check;

    memset(filename, 0, READBUF_SIZE);
    memset(args, 0, READBUF_SIZE);

    //Parse args
    for (i = 0; i < READBUF_SIZE; i++)
        if (!command[i])
            return -1;
        else if (command[i] != ' ')
            break;

    command += i;

    for (i = 0; i < READBUF_SIZE; i++)
        if (!command[i] || command[i] == ' ')
            break;
        else
            filename[i] = command[i];

    command += i;
    
    for (i = 0; i < READBUF_SIZE; i++)
        if (command[i] != ' ')
            break;

    command += i;

    for (i = 0; i < READBUF_SIZE; i++)
        if (!command[i])
            break;
        else
            args[i] = command[i];
    
    //Executable check
    if (read_dentry_by_name(filename, &exec_dentry) == -1
        ||read_data(exec_dentry.inode_num, 0, &magic_check, MAGIC_SIZE) == -1
        ||magic_check != MAGIC_NUM)
        return -1;

    //Set up 4MB program paging
    for (i = 0; i < MAX_TASKS; i++)
        if (!(GET_PCB(i)->present))
            break;

    if (i==MAX_TASKS)
        return -1;
    
    GET_PCB(i)->pid = i;
    GET_PCB(i)->present = 1;
    
    page_directory[USER_ENTRY].MB.present=1;
    page_directory[USER_ENTRY].MB.user_supervisor=1;
    page_directory[USER_ENTRY].val|=0x400000+i*0x400000;

    //User-level Program Loader 
    //Create Process Control Block
    //Context Switch:
    //Create its own context switch stack
    //IRET
    return -1;
}

int32_t read(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

int32_t open(const uint8_t* filename){
    return -1;
}

int32_t close(int32_t fd){
    return -1;
}

int32_t getargs(uint8_t* buf, int32_t nbytes){
    return -1;
}

int32_t vidmap(uint8_t** screen_start){
    return -1;
}

int32_t set_handler(int32_t signum, void* handler_address){
    return -1;
}

int32_t sigreturn(void){
    return -1;
}

pcb_t* current_pcb(){
    uint32_t esp;
    asm volatile ("                 \n\
            movl %%esp, %0          \n\
            "
            : "=r"(esp)
    );
    return (pcb_t*)((0x800000-esp)/0x2000);
}

