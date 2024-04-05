#include "system_call.h"
#include "filesys.h"

file_descriptor_t global[8];
uint32_t dir_pos=0;
uint32_t file_size = 0;

/*
* file_system_init
*   DESCRIPTION: Initialize the file system
*   INPUTS: boot_addr - the address of the boot block
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: Initialize the file system
*/
void file_system_init(uint32_t boot_addr){
    void* ptr = (void*)boot_addr;
    boot_block = (boot_block_t*)ptr;
    inode_block = (inode_t*)(boot_block + 1);
    dentry_block = boot_block->dir_entries_arr;
    data_block = (data_block_t*)(boot_block + boot_block->num_inodes + 1);

}

/*
* read_dentry_by_name
*   DESCRIPTION: Read the directory entry by name
*   INPUTS: fname - the name of the file
*           dentry - the directory entry
*   OUTPUTS: dentry - the directory entry
*   RETURN VALUE: 0 if success, -1 if fail
*   SIDE EFFECTS: none
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    int i;
    if(fname == NULL || strlen((int8_t*)fname) > MAX_FILE_NAME+1){
        return -1;
    }

    for(i = 0; i < boot_block->num_dir_entries; i++){
        if(strncmp((int8_t*)fname, (int8_t*)dentry_block[i].file_name, MAX_FILE_NAME) == 0){
            *dentry = dentry_block[i];
            // f_size = inode_block->file_size;
            return 0;
        }
    }
    return -1;
}

/*
* read_dentry_by_index
*   DESCRIPTION: Read the directory entry by index
*   INPUTS: index - the index of the file
*           dentry - the directory entry
*   OUTPUTS: dentry - the directory entry
*   RETURN VALUE: 0 if success, -1 if fail
*   SIDE EFFECTS: none
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    if(index >= boot_block->num_dir_entries || index < 0){
        return -1;
    }
    *dentry = dentry_block[index];
    return 0;
}

/*
*   read_data
*   DESCRIPTION: Read the data of the file
*   INPUTS: inode - the inode number
*           offset - the offset of the file
*           buf - the buffer to store the data
*           length - the length of the data
*   OUTPUTS: buf - the buffer to store the data
*   RETURN VALUE: the number of bytes read
*   SIDE EFFECTS: none
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	int i=0;
    uint32_t data_block_num;
    uint32_t data_block_offset;
    uint32_t origin_num_data = boot_block->num_data_blocks;
    inode_t* inode_block = (inode_t*)&(boot_block[inode+1]); //get current inode block

	if (inode>=boot_block->num_inodes+1 || inode<0) return -1; // invalid inode number
    if(offset >= inode_block->file_size) return 0; // offset is larger than file size

	data_block_num = offset/BLOCK_SIZE;
	data_block_offset = offset%BLOCK_SIZE;
    
    // get the current data block
	uint8_t* curr_data = (uint8_t*)&boot_block[boot_block->num_inodes+1+inode_block->data_block_num[data_block_num]+data_block_offset];
    
	// for (i=0;i<length;i++){
    while(i<length){
        // check if the offset is larger than the file size, yes exit
		if (i+offset >= inode_block->file_size){
			return i;	
		}
        // copy the data from data block to the buffer
        memcpy(buf + i, curr_data, 1);
		data_block_offset++;

        // check if the data block offset is larger than the block size
		if (data_block_offset%BLOCK_SIZE==0){
            // get the next data block number for index
			data_block_num++;
            // check if the data block number is valid
			if (inode_block->data_block_num[data_block_num]>=origin_num_data) return -1;
            // get the next data block
			curr_data = (uint8_t*)&boot_block[boot_block->num_inodes+1+inode_block->data_block_num[data_block_num]];
		}
		else{
		    curr_data++;
		}
        i++;
	}
	return length;
}

/*
* file_open
*   DESCRIPTION: Open the file
*   INPUTS: filename - the name of the file
*   OUTPUTS: none
*   RETURN VALUE: 0 if success, -1 if fail
*   SIDE EFFECTS: none
*/
int32_t file_open (const uint8_t* filename){
    dentry_t file_dentry;
    if(read_dentry_by_name(filename, &file_dentry) == -1){
        return -1;
    }
    return 0;
}

/*
*   file_read
*   DESCRIPTION: Read the file
*   INPUTS: fd - the file descriptor
*           buf - the buffer to store the data
*           nbytes - the number of bytes to read
*   OUTPUTS: buf - the buffer to store the data
*   RETURN VALUE: the number of bytes read
*   SIDE EFFECTS: none
*/

int32_t file_read (int32_t fd, void* buf, int32_t nbytes){

    pcb_t* curr_pcb = current_pcb();

    uint32_t inode_num = curr_pcb->fd[fd].inode;
    uint32_t pos = curr_pcb->fd[fd].file_position;

    return read_data(inode_num, pos, buf, nbytes);
}

/*
*   file_write
*   DESCRIPTION: Write the file
*   INPUTS: fd - the file descriptor
*           buf - the buffer to store the data
*           nbytes - the number of bytes to write
*   OUTPUTS: none
*   RETURN VALUE: -1
*   SIDE EFFECTS: none
*/
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes){
    return -1; // read only
}

/*
*   file_close
*   DESCRIPTION: Close the file
*   INPUTS: fd - the file descriptor
*   OUTPUTS: none
*   RETURN VALUE: 0
*   SIDE EFFECTS: none
*/
int32_t file_close (int32_t fd){
    return 0;
}

/*
*   dir_open
*   DESCRIPTION: Open the directory
*   INPUTS: filename - the name of the directory
*   OUTPUTS: none
*   RETURN VALUE: 0 if success, -1 if fail
*   SIDE EFFECTS: none
*/
int32_t dir_open (const uint8_t* filename){
    dentry_t dir_dentry;
    if(read_dentry_by_name(filename, &dir_dentry) == -1){
        return -1;
    }
    return 0;

}

/*
*   dir_read2
*   DESCRIPTION: Read the directory
*   INPUTS: offset - the offset of the directory
*   OUTPUTS: none
*   RETURN VALUE: the size of the file
*   SIDE EFFECTS: none
*/
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
    int rev;
    // int i;
    dentry_t dir_entry;

    if (buf == NULL || dir_pos >= boot_block->num_dir_entries){
        dir_pos = 0;
        return 0;
    }

    dir_entry = boot_block->dir_entries_arr[dir_pos];
    dir_pos ++;

    memcpy(buf,&dir_entry.file_name,32);

    // printf("file_name: ");
    // for (i = 0; i < strlen((int8_t*)dir_entry.file_name); i++){
    //     printf("%c",dir_entry.file_name[i]);
    // }
    // file_size = read_data(dir_entry.inode_num, 0, buf, 10000);
    int8_t* addr = (int8_t*)boot_block+BLOCK_SIZE+(dir_entry.inode_num*BLOCK_SIZE);
    memcpy(&file_size, addr, 4); // 4 bytes for the size of the file

    // printf("  file_type: %d, file_size: %d", dir_entry.file_type, file_size);
    // printf("\n");

    rev = strlen((int8_t*)dir_entry.file_name);
    if (strlen((int8_t*)dir_entry.file_name) >= 32){
        rev = 32;
    }
    return rev;
}

/*
*   dir_write
*   DESCRIPTION: Write the directory
*   INPUTS: fd - the file descriptor
*           buf - the buffer to store the data
*           nbytes - the number of bytes to write
*   OUTPUTS: none
*   RETURN VALUE: -1
*   SIDE EFFECTS: none
*/
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes){
    return -1; // read only
}

/*
*   dir_close
*   DESCRIPTION: Close the directory
*   INPUTS: fd - the file descriptor
*   OUTPUTS: none
*   RETURN VALUE: 0
*   SIDE EFFECTS: none
*/
int32_t dir_close (int32_t fd){
    return 0;
}

