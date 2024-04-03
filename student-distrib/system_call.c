#include "system_call.h"
#include "lib.h"
#include "paging.h"
#include "filesys.h"
#include "terminal.h"

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
    int i, pid;
    uint8_t* filename[READBUF_SIZE];
    uint8_t* args[READBUF_SIZE];
    dentry_t exec_dentry;
    uint32_t magic_check, eip;
    pcb_t* pcb;

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
        ||read_data(exec_dentry.inode_num, 0, (uint8_t*)&magic_check, MAGIC_SIZE) == -1
        ||magic_check != MAGIC_NUM)
        return -1;

    //Set up 4MB program paging
    for (i = 0; i < MAX_TASKS; i++)
        if (!(GET_PCB(i)->present))
            break;

    if (i==MAX_TASKS)
        return -1;

    pid=i;
    page_directory[USER_ENTRY].MB.present=1;
    page_directory[USER_ENTRY].MB.user_supervisor=1;
    page_directory[USER_ENTRY].val|=0x400000+pid*0x400000;

    asm volatile (              //Flush TLB
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "%eax"
    );

    //User-level Program Loader 
    read_data(exec_dentry.inode_num, 0, (uint8_t*)USER_CODE, USER_CODE_LIMIT);

    //Create Process Control Block
    pcb=GET_PCB(pid);
    if (pid)
        pcb->parent_pcb= current_pcb();
    pcb->present=1;
    pcb->pid=i;

    memset(pcb->fd, 0, sizeof(file_descriptor_t)*MAX_FILES);
    
    pcb->fd[0].file_ops=&stdin_op;
    pcb->fd[0].flags=1;
    pcb->fd[1].file_ops=&stdout_op;
    pcb->fd[1].flags=1;

    for (i = 2; i < MAX_FILES; i++)
        pcb->fd[i].file_ops=&null_op;

    memcpy(pcb->args, args, READBUF_SIZE);
    //Context Switch:
    tss.esp0=KSTACK_START-KSTACK_SIZE*pid-4;
    read_data(exec_dentry.inode_num, 24, (uint8_t*)&eip, 4);

    //Create its own context switch stack
    asm volatile (
        "movw %0, %ds;"
        "pushl %0;"
        "pushl %1;"
        "pushfl;"
        "pushl %2;"
        "pushl %3;"
        "iret;"
        :
        :"r"((uint32_t)USER_DS), "r"((uint32_t)USER_STACK), "r"((uint32_t)USER_CS), "r"(eip)
        :"memory"
    );
    return 0;
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

int32_t null_read(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

int32_t null_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

int32_t null_open(const uint8_t* filename){
    return -1;
}

int32_t null_close(int32_t fd){
    return -1;
}

pcb_t* current_pcb(){
    uint32_t esp;
    asm volatile ("                 \n\
            movl %%esp, %0          \n\
            "
            : "=r"(esp)
    );
    return (pcb_t*)(esp&(KSTACK_START-KSTACK_SIZE));
}

