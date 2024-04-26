#include "system_call.h"
#include "lib.h"

/* nonzero if exception occurs. */
extern uint8_t exception_occurred;

extern pte_t page_table_user_vidmem[PAGE_TABLE_COUNT];

/**
 * int32_t halt(uint8_t status):
 * DESCRIPTION: a system call handler that when some process
 *              wants to terminate, or generates some exception
 * INPUTS: status -- the return value execute should get
 * OUTPUTS: none
 * RETURNS: never used
 */
int32_t halt(uint8_t status){
    cli();
    int i;
    pcb_t* pcb = current_pcb();
    
    /* **************************************************
     * *            Reclaims Owned Resources            *
     * **************************************************/
    pcb->present = 0;
    pcb->vidmap = 0;
    pcb->rtc = 0;
    get_terminal(*get_active_terminal())->halt = 0;

    for (i = 0; i < MAX_FILES; ++i) {
        pcb->fd[i].flags = 0;
    }

    if (pcb->pid < 3) { /* if exit the shell, recreate it ^-^ */
        pcb->present = 0;
        execute((uint8_t*)"shell");
        return;
    }
    
    get_terminal(*get_current_terminal())->pid = current_pcb()->parent->pid;

    /* **************************************************
     * *          Restore Parent Paging & TSS           *
     * **************************************************/
    page_directory[USER_ENTRY].MB.page_base_address = 2 + pcb->parent->pid; /* shell */
    tss.esp0 = pcb->parent->esp0;
    tss.ss0 = KERNEL_DS;

    asm volatile (
        "movl %%cr3, %%eax\n"   /* flushes the tlb*/
        "movl %%eax, %%cr3\n"
        :
        :
        : "%eax"
    );

    if (exception_occurred) {
        asm volatile(           /* exception occurred, %eax = 0x100 */
            "movl $0x100, %%eax\n"
            "movl %0, %%ebp\n"      /* restores the ebp */
            :
            : "r"(pcb->eebp)
            : "%eax", "%ebp"
        );
        exception_occurred = 0;
    } else {
        asm volatile(
            "movl $0, %%eax\n"
            "movb %b0, %%al\n"
            "movl %1, %%ebp\n"      /* restores the ebp */
            :
            : "r"(status), "r"(pcb->eebp)
            : "%eax", "%ebp"
        );
    }

    asm volatile(               /* we are logically in remaining part of */
        "sti\n"
        "leave\n"               /* execute handler, because we reset ebp */
        "ret\n"                 /* we return to execute actually.        */
    );

    return -1;                  /* never reaches here */
}

/**
 * int32_t execute(const uint8_t* command):
 * DESCRIPTION: executes a program specified by \p command
 *              followed by arguments (not required) after
 *              the program name in \p command
 * INPUTS: command -- a null-terminated string stored the
 *                    program name and the arguments
 * OUTPUTS: none
 * RETURN: 0 if created successfully, 1 otherwise
 */
int32_t execute(const uint8_t* command){
    cli();
    int i, pid;
    uint8_t filename[READBUF_SIZE] = {0};   /* file name */
    uint8_t args[READBUF_SIZE] = {0};       /* arguments */
    dentry_t exec_dentry;           /* location of file name */
    uint32_t magic_check;           /* exec format check */
    uint8_t entry[4];               /* instruction ptr */
    uint32_t eip;
    pcb_t* pcb;

    /* **************************************************
     * *                 Parse Argument                 *
     * **************************************************/
    /* filters spaces before file name */
    for (i = 0; i < READBUF_SIZE; i++)
        if (!command[i])
            return -1;
        else if (command[i] != ' ')
            break;

    command += i;

    /* parses the file name */
    for (i = 0; i < READBUF_SIZE; i++)
        if (!command[i] || command[i] == ' ')
            break;
        else
            filename[i] = command[i];

    command += i;
    
    /* filters the space before arguments */
    for (i = 0; i < READBUF_SIZE; i++)
        if (!command[i] ||command[i] != ' ')
            break;

    command += i;

    /* parses the arguments */
    for (i = 0; i < READBUF_SIZE; i++)
        if (!command[i])
            break;
        else
            args[i] = command[i];
    
    /* **************************************************
     * *              Check File Validity               *
     * **************************************************/
    /*
     * checks the
     *   - existence of the file
     *   - size of the file
     *   - magic header of executable
     */
    if (read_dentry_by_name(filename, &exec_dentry) == -1
        || read_data(exec_dentry.inode_num, 0, (uint8_t*)&magic_check, MAGIC_SIZE) == -1
        || magic_check != MAGIC_NUM)
        return -1;

    /* gets the index of the new process */
    for (pid = 0; pid < MAX_TASKS; pid++)
        if (!(GET_PCB(pid)->present))
            break;

    if (pid == MAX_TASKS) /* cannot handle it */
        return -1;


    /* **************************************************
     * *                  Setup Paging                  *
     * **************************************************/
    page_directory[USER_ENTRY].MB.present = 1;              /* 128MB -> user program */
    page_directory[USER_ENTRY].MB.user_supervisor = 1;      /* user can access */
    page_directory[USER_ENTRY].MB.read_write = 1;           /* allows user to write (-_-|||) */
    page_directory[USER_ENTRY].MB.page_base_address = 2 + pid; /* physical address: 0x400000 & 0x800000 */

    asm volatile (                  /* rewrite cr3 (pde[]) to empty tlb */
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        :
        :
        : "%eax"
    );

    /* **************************************************
     * *             Load File into Memory              *
     * **************************************************/
    /* loads the program image */
    read_data(exec_dentry.inode_num, 0, (uint8_t*)PROGRAM_IMAGE_ADDR, PROGRAM_IMAGE_LIMIT);

    /* **************************************************
     * *              Create PCB & File OP              *
     * **************************************************/
    /* creates the pcb */
    pcb = GET_PCB(pid);
    pcb->parent = (pid > 2) ? current_pcb() : NULL;
    pcb->present = 1;
    pcb->pid = pid;
    
    /* setup stdin and stdout*/
    pcb->fd[0].file_ops = (file_operations_t*)&stdin_op;
    pcb->fd[0].flags = 1;
    pcb->fd[1].file_ops = (file_operations_t*)&stdout_op;
    pcb->fd[1].flags = 1;

    /* setup remaining fileop */
    for (i = 2; i < MAX_FILES; i++)
        pcb->fd[i].file_ops = (file_operations_t*)&null_op;

    memcpy(pcb->args, args, READBUF_SIZE); /* assign pcb->args */
    
    read_data(exec_dentry.inode_num, 24, (uint8_t*)entry, 4);
    eip = (((uint32_t)entry[3] << 24) | ((uint32_t)entry[2] << 16) | ((uint32_t)entry[1] << 8) | ((uint32_t)entry[0]));

    tss.esp0 = KSTACK_START - KSTACK_SIZE * pid;
    tss.ss0 = KERNEL_DS;

    //for (i = 0; i < NUM_TERMINAL; i++)
    //if (get_terminal(*get_current_terminal())->pid == -1){
    //    get_terminal(*get_current_terminal())->pid = pid;
    //    goto found;
    //}
//
    //for (i = 0; i < NUM_TERMINAL; i++)
    //    if (get_terminal(i)->pid == current_pcb()->pid){
    //        get_terminal(i)->pid = pid;
    //        break;
    //    }
    //found:

    get_terminal(*get_current_terminal())->pid = pid;

    pcb->esp0 = tss.esp0;
    /* **************************************************
     * *           Prepare for Context Switch           *
     * **************************************************/
    /* save the current ebp */
    asm volatile(
        "movl %%ebp, %0\n"
        : "=g"(pcb->ebp)
    );

    pcb->eebp = pcb->ebp;

    /* Create its own context switch stack */
    asm volatile (
        "movw %w0, %%ds\n"
        "pushl %0\n"      /* USER_DS */
        "pushl %1\n"      /* USER_STACK */
        "sti\n"
        "pushfl\n"        /* eflags */
        "pushl %2\n"      /* USER_CS */
        "pushl %3\n"      /* eip */
        "iret\n"
        :
        :"r"((uint32_t)USER_DS), "r"((uint32_t)USER_STACK), "r"((uint32_t)USER_CS), "r"(eip)
        :"memory"
    );

    return 0;
}

/**
 * int32_t read(int32_t fd, void* buf, int32_t nbytes):
 * DESCRIPTION: reads the content of the first \p nbytes bytes
 *              file in the \p fd of the process, and stores the
 *              read content into \p buf.
 * INPUTS: fd - the file descriptor of the read file
 *         nbytes - the capacity of the buffer
 * OUTPUTS: buf - the buffer storing the content
 * RETURNS: the number of bytes read, or -1 if failed
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    pcb_t* curr_pcb = current_pcb();    /* current pcb for fd array */
    uint32_t result;
    if(fd < 0 || fd >= MAX_FILES || curr_pcb->fd[fd].flags==0) {
        return -1;                      /* the argument is illegal*/
    }
    result = curr_pcb->fd[fd].file_ops->read(fd, buf, nbytes);  /* call the interfance */
    curr_pcb->fd[fd].file_position += result; 
    return result;
}

/**
 * int32_t write(int32_t fd, const void* buf, int32_t nbytes):
 * DESCRIPTION: wrties the first \p nbytes buffer \p buf into the file
 *              represented by \p fd.
 * INPUTS: fd - the file descriptor of the file to write
 *         buf - the buffer to be wrote into the file
 *         nbytes - the number of bytes to be wrote
 * OUTPUTS: none
 * RETURNS: the number of byte wrote, or -1 if failed to write
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    pcb_t* curr_pcb = current_pcb();
    if(fd < 0 || fd >= MAX_FILES || curr_pcb->fd[fd].flags==0){
        return -1; /* illegal argument */
    }
    return curr_pcb->fd[fd].file_ops->write(fd, buf, nbytes);
}

/**
 * int32_t open(const uint8_t* filename):
 * DESCRIPTION: opens a files at the given \p filename
 *              open should be called BEFORE any other manipulations
 * INPUTS: filename - the file name
 * OUTPUTS: none
 * RETURN: the file descriptor of the file, or -1 otherwise
 */
int32_t open(const uint8_t* filename){
    int i;
    dentry_t dentry;
    pcb_t* curr_pcb = current_pcb();                    /* the process to open the file */

    if(filename == NULL || read_dentry_by_name(filename, &dentry) == -1) {
        return -1;                                      /* the file name is invalid */
    }

    for(i=2; i<MAX_FILES; i++){                         /* seeks for a idle fd */
        if( curr_pcb->fd[i].flags == 0){
            curr_pcb->fd[i].flags = 1;                  /* marks it open */
            curr_pcb->fd[i].inode = dentry.inode_num;
            curr_pcb->fd[i].file_position = 0;          /* marks the position to the beginning */
            switch(dentry.file_type) {                  /* assigns the corresponding interface */
                case 0:
                    curr_pcb->fd[i].file_ops = (file_operations_t*)&rtc_op;
                    break;
                case 1:
                    curr_pcb->fd[i].file_ops = (file_operations_t*)&dir_op;
                    break;
                case 2:
                    curr_pcb->fd[i].file_ops = (file_operations_t*)&file_op;
                    break;
                default:
                    break;
            }
            if (curr_pcb->fd[i].file_ops->open(filename)) {
                curr_pcb->fd[i].flags = 0;
                return 0;
            }
            return i;                                   /* returns the fd */
        }
    }
    return -1;                                          /* reaches the capacity */
}

/**
 * int32_t close(int32_t fd):
 * DESCRIPTION: closes the file represented by \p fd
 * INPUTS: fd - the file needing to be closed
 * OUTPUTS: none
 * RETURN: 0 if closed, or -1 otherwise
 */
int32_t close(int32_t fd){
    pcb_t* curr_pcb = current_pcb();
    if(fd < 2 || fd >= MAX_FILES || curr_pcb->fd[fd].flags==0) {
        return -1; /* illegal arguments */
    }
    curr_pcb->fd[fd].flags = 0;             /* marks closed */
    return curr_pcb->fd[fd].file_ops->close(fd);    /* call the interface */
}

/**
 * int32_t getargs(uint8_t* buf, int32_t nbytes):
 * DESCRIPTION: gets the arguments of the process when it is created
 * INPUTS: nbytes - the size of the buffer
 * OUTPUTS: buf - the buffer to store the argument 
 * RETURN: 0 if succeed, -1 otherwise
 */
int32_t getargs(uint8_t* buf, int32_t nbytes){
    if (buf == NULL || nbytes == 0) {   /* check if the buffer is valid */
        return -1;
    }
    
    pcb_t* pcb = current_pcb();
    if (!pcb->args[0]) {                /* check if the process has an argument*/
        *buf = 0;
        return -1;
    }

    uint8_t* src, *dest;
    for (src = (uint8_t*)pcb->args, dest = buf;
         nbytes && *src;
         ++src, ++dest, --nbytes) {     /* copy until buffer reached   */
        *dest = *src;                   /* or the string is terminated */
    }
    if (nbytes == 0) {                  /* cannot put the termination */
        return -1;
    } else {
        *dest = 0;
        return 0;
    }
}

/**
 * int32_t vidmap(uint8_t** screen_start)
 * DESCRIPTION: maps the text-mode video memory into user space
 * INPUTS: screen_start - the memory entry of the screen
 * OUTPUTS: none
 * RETURN: video memory address
 */
int32_t vidmap(uint8_t** screen_start) {
    if (screen_start == NULL                /* invalid address */
        || ((uint32_t)(screen_start) >> 22) != USER_ENTRY) {
        return -1;
    }

    int i;

    current_pcb()->vidmap = 1;
    page_table_user_vidmem[VIDEO_MEMORY_PTE].present = 1;
    if (*get_active_terminal() == *get_current_terminal()) {
        page_table_user_vidmem[VIDEO_MEMORY_PTE].page_base_address = VIDEO_MEMORY_PTE;
    } else {
        page_table_user_vidmem[VIDEO_MEMORY_PTE].page_base_address = VIDEO_MEMORY_PTE + current_pcb()->pid + 2;
    }

    /* assigns page address */
    *screen_start = (uint8_t*)((VIDEO_MEMORY_PTE << 22) | (VIDEO_MEMORY_PTE << 12));

    asm volatile (
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        :
        :
        : "%eax"
    );

    return 0; /* succeed */
}

/**
 * int32_t set_handler(int32_t signum, void* handler_address):
 * DESCRIPTION: for cp4
 * INPUTS: signum - the index of the signal
 *         handler_address - the function pointer of the handler
 * OUTPUTS: none
 * RETURN: (pending for cp4)
 */
int32_t set_handler(int32_t signum, void* handler_address){
    return -1;
}

/**
 * int32_t sigreturn(void):
 * DESCRIPTION: for cp4
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: (pending for cp4)
 */
int32_t sigreturn(void){
    return -1;
}

/**
 * int32_t null_read(int32_t fd, void* buf, int32_t nbytes):
 * DESCRIPTION: read handler for closed meaningless fd
 * INPUTS: IGNORED
 * OUTPUTS: IGNORED
 * RETURN: always -1
 */
int32_t null_read(int32_t fd, void* buf, int32_t nbytes) {
    return -1;
}

/**
 * int32_t null_write(int32_t fd, const void* buf, int32_t nbytes):
 * DESCRIPTION: write handler for closed meaningless fd
 * INPUTS: IGNORED
 * OUTPUTS: IGNORED
 * RETURN: always -1
 */
int32_t null_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/**
 * int32_t null_open(const uint8_t* filename):
 * DESCRIPTION: open handler for closed meaningless fd
 * INPUTS: IGNORED
 * OUTPUTS: IGNORED
 * RETURN: always -1
 */
int32_t null_open(const uint8_t* filename){
    return -1;
}

/**
 * int32_t null_close(int32_t fd):
 * DESCRIPTION: close handler for closed meaningless fd
 * INPUTS: IGNORED
 * OUTPUTS: IGNORED
 * RETURN: always -1
 */
int32_t null_close(int32_t fd){
    return -1;
}

/**
 * pcb_t* current_pcb():
 * DESCRIPTION: gets the pcb of the running process
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: the pcb of the running process
*/
pcb_t* current_pcb(){
    uint32_t esp;
    asm volatile ("                 \n\
            movl %%esp, %0          \n\
            "
            : "=r"(esp)
    );
    return (pcb_t*)(esp & (KSTACK_START - KSTACK_SIZE));
}
