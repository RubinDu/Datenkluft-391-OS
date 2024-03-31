#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "lib.h"
#include "types.h"
#include "system_call.h"


#define MAX_FILE_NAME 32
#define MAX_DATA_BLOCKS 1023
#define MAX_DIR_ENTRIES 63
#define MAX_BOOT_RESERVED 52

#define BLOCK_SIZE 4096


// Directory entry struct
typedef struct {
    char file_name[MAX_FILE_NAME];                  // 32 bytes for the file name
    uint32_t file_type;                             // 4 bytes for the file type
    uint32_t inode_num;                             // 4 bytes for the inode number
    uint8_t reserved[24];                           // 24 bytes reserved
} dentry_t;

// Boot block struct
typedef struct {
    uint32_t num_dir_entries;                       // 4 bytes for number of directory entries
    uint32_t num_inodes;                            // 4 bytes for number of inodes
    uint32_t num_data_blocks;                       // 4 bytes for number of data blocks
    uint8_t reserved[MAX_BOOT_RESERVED];            // 52 bytes reserved
    dentry_t dir_entries_arr[MAX_DIR_ENTRIES];   // Flexible array member for directory entries
} boot_block_t;

// Index node struct
typedef struct {
    uint32_t file_size;                             // 4 bytes for the size of the file
    uint32_t data_block_num[MAX_DATA_BLOCKS];       // 1023 * 4 bytes for the data block numbers of the file
} inode_t;

typedef struct {
    uint32_t data_block[BLOCK_SIZE];
} data_block_t;


typedef struct file_operations {
    int (*open)(const uint8_t* filename);
    int (*read)(int32_t fd, void* buf, int32_t nbytes);
    int (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int (*close)(int32_t fd);
} file_operations_t;

typedef struct file_descriptor {
    file_operations_t* file_ops;                    // Pointer to the file operations table
    uint32_t inode;                                 // Inode number for the file
    uint32_t file_position;                         // Current position in the file
    uint32_t flags;                                 // Flags indicating the status of the file descriptor
} file_descriptor_t;


boot_block_t* boot_block;
inode_t* inode_block;
dentry_t* dentry_block;
data_block_t* data_block;

uint32_t f_size;

extern uint32_t* start;

void file_system_init(uint32_t boot_addr);

int32_t file_open (const uint8_t* filename);
int32_t file_read (int32_t fd, void* buf, int32_t nbytes);
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t file_close (int32_t fd);

int32_t dir_open (const uint8_t* filename);
int32_t dir_read2 (uint32_t offset);
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t dir_close (int32_t fd);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif
