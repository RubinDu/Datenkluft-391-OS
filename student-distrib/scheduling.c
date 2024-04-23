#include "scheduling.h"
#include "system_call.h"

/**
 * void initiate_shells()
 * DESCRIPTION: initiates three shells at the entry
 * INPUT: none
 * OUTPUT: none
 * RETURN: none
 */
void initiate_shells() {
    uint32_t magic;             /* checks the magic number */
    dentry_t shell_dentry;      /* the dentry of the shell */
    if (read_dentry_by_name((const uint8_t *)"shell", &shell_dentry) == -1
        || read_data(shell_dentry.inode_num, 0, (uint8_t *)&magic, 4) == -1
        || magic != MAGIC_NUM) {
            return;
    }

    uint8_t entry[4];           /* entry point of shell */
    volatile uint32_t eip;               /* reverse bytes*/
    if (read_data(shell_dentry.inode_num, 24, (uint8_t *)entry, 4) == -1) {
        return;
    }
    eip = (((uint32_t)entry[3] << 24) | ((uint32_t)entry[2] << 16) | ((uint32_t)entry[1] << 8) | ((uint32_t)entry[0]));

    page_directory[USER_ENTRY].MB.present = 1;
    page_directory[USER_ENTRY].MB.user_supervisor = 1;
    page_directory[USER_ENTRY].MB.read_write = 1;

    int pid, i;                 /* the corresponding eip*/
    struct pcb* pcb;            /* the pcb of process*/
    for (pid = 2; pid >= 0; --pid) {
        page_directory[USER_ENTRY].MB.page_base_address = 2 + pid;

        asm volatile (
            "movl %%cr3, %%eax\n"
            "movl %%eax, %%cr3\n"
            :
            :
            : "%eax"
        );

        read_data(shell_dentry.inode_num, 0, (uint8_t*)PROGRAM_IMAGE_ADDR, PROGRAM_IMAGE_LIMIT);

        pcb = GET_PCB(pid);
        pcb->present = 1;
        pcb->parent = NULL;
        pcb->pid = pid;

        pcb->fd[0].file_ops = (file_operations_t *)&stdin_op;
        pcb->fd[0].flags = 1;
        pcb->fd[1].file_ops = (file_operations_t *)&stdout_op;
        pcb->fd[1].flags = 1;
        for (i = 2; i < MAX_FILES; ++i) {
            pcb->fd[i].file_ops = (file_operations_t *)&null_op;
            pcb->fd[i].flags = 0;
        }

        pcb->esp0 = KSTACK_START - KSTACK_SIZE * pid;
        pcb->ss0 = KERNEL_DS;

        asm volatile (
            "movl %%esp, %%esi\n"
            "movl %%ebp, %%edi\n"
            "movl %5, %%esp\n"  /* write to stack belonging to pcb */
            "movl %5, %%ebp\n"
            "pushl %1\n"        /* USER_DS */
            "pushl %2\n"        /* USER_STACK */
            "pushfl\n"          /* %eflags */
            "pushl %3\n"        /* USER_CS */
            "pushl %4\n"        /* EIP */
            "pushl $iret_exec\n"
            "pushl $iret_exec\n"/* simulate RET */
            "movl %%esp, %0\n"  /* changed esp0 */
            "movl %%edi, %%ebp\n"
            "movl %%esi, %%esp\n"
            : "=r"(pcb->ebp)
            : "g"((uint32_t)USER_DS), "r"((uint32_t)USER_STACK), "g"((uint32_t)USER_CS), "r"(eip), "r"(pcb->esp0)
            : "%esi", "%edi", "%esp", "%ebp"
        );
    }

    tss.esp0 = pcb->esp0;
    tss.ss0 = pcb->ss0;

    asm volatile ("movl %0, %%ebp"::"r"(pcb->ebp));             /* ebp = &iret_exec #1*/
    asm volatile ("leave"); /* ESP = EBP + 4, EBP = M[EBP] */   /* esp = &iret_exec #2; EBP = M[ebp] = iret_exec */                  
    asm volatile ("ret");   /* EIP = M[ESP],  ESP = ESP + 4*/   /* eip = iret_exec    ; esp = esp + 4 = switch for iret */
    asm volatile ("iret_exec: iret");
}

void context_switch(uint32_t next_pid) {
    cli();
    
    volatile struct pcb *current = current_pcb();
    volatile struct pcb *next = GET_PCB(next_pid);
    if (current->pid == next->pid) {
        return;
    }

    // page_table[VIDEO_MEMORY_PTE].
    page_directory[USER_ENTRY].MB.page_base_address = 2 + next_pid;
    
    current->esp0 = tss.esp0;
    current->ss0 = tss.ss0;
    tss.esp0 = next->esp0;
    tss.ss0 = next->ss0;

    asm volatile (
        "movl %%cr3, %%ecx\n"
        "movl %%ecx, %%cr3\n"
        "movl %%ebp, %%eax \n"
        "movl %%edx, %%ebp \n"
        "sti\n"
        "leave\n"
        "ret\n"
        : "=a"(current->ebp)
        : "d"(next->ebp)
        : "%ecx"
    );
}

// void context_switch() {
//     cli();

//     struct pcb* current = current_pcb();

//     uint32_t terminal_id, next_terminal_id, next_pid;
//     if (current->pid > 3) {
//         terminal_id = current->parent->pid;
//     } else {
//         terminal_id = current->pid;
//     }

//     while (next_terminal_id != terminal_id) {
//         if (terminals[next_terminal_id].pid >= 3) {
//             next_pid = terminals[next_terminal_id].pid;
//             break;
//         } else if (next_terminal_id == active_terminal) {
//             next_pid = active_terminal;
//             break;
//         }
//     }

//     if (next_terminal_id == terminal_id) {
//         return;
//     }

//     struct pcb* next = GET_PCB(next_pid);
    
//     page_directory[USER_ENTRY].MB.page_base_address = 2 + next_pid;
//     asm volatile (
//         "movl %%cr3, %%eax\n"
//         "movl %%eax, %%cr3\n"
//         :
//         :
//         : "%eax"
//     );

//     asm volatile (
//         "movl %%esp, %0 \n"
//         "movl %%ebp, %1 \n"
//         "movl %2, %%esp \n"
//         "movl %3, %%ebp \n"
//         : "=r"(current->esp), "=r"(current->ebp)
//         : "r"(next->esp), "r"(next->ebp)
//         : "%esp", "%ebp"
//     );

//     tss.esp0 = next->esp0;
//     tss.ss0 = next->ss0;

//     sti();
// }