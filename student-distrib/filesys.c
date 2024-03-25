#include "filesys.h"

file_descriptor_t global[8];
/*
* file_system_init
*   DESCRIPTION: Initialize the file system
*   INPUTS: boot_addr - the address of the boot block
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: Initialize the file system
*/
void file_system_init(uint32_t boot_addr){
    boot_block = (boot_block_t*)boot_addr;
    inode_block = (inode_t*)(boot_addr + 1);
    dentry_block = boot_block->dir_entries_arr;
    data_block = (data_block_t*)(boot_addr + 1 + boot_block->num_inodes);

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
    if(fname == NULL){
        return -1;
    }
    int i;
    for(i = 0; i < boot_block->num_dir_entries; i++){
        if(strncmp((int8_t*)fname, (int8_t*)dentry_block[i].file_name, MAX_FILE_NAME) == 0){
            *dentry = dentry_block[i];
            f_size = inode_block->file_size;
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
	int i;

	if (inode>=boot_block->num_inodes || inode<0){
	    return -1;
	}
	 
	inode_t* inode_block;
	inode_block = (inode_t*)boot_block+inode+1;
	 
	uint32_t block_num = offset/BLOCK_SIZE;
	uint32_t block_offset = offset%BLOCK_SIZE;
	uint8_t* data_ptr = (uint8_t*)(boot_block + boot_block->num_inodes + 1 + inode_block->data_block_num[block_num])+block_offset;	 
	for (i=0;i<length;i++){
		if (i+offset >= inode_block->file_size){
			return i;	
		}
		buf[i] = *data_ptr;
		block_offset++;

		if (block_offset%BLOCK_SIZE==0){
			block_num++;
			if (inode_block->data_block_num[block_num]>=boot_block->num_data_blocks){
				return -1;
			}
			data_ptr = (uint8_t*)(boot_block + boot_block->num_inodes + 1 + inode_block->data_block_num[block_num]);
		}
		else{
		    data_ptr++;
		} 
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
    // int32_t bytes_read;
    // int32_t inode_index;
    // int32_t file_position;
    // file_descriptor_t file_desc = global[fd];
    // inode_index = file_desc.inode;
    // file_position = file_desc.file_position;

    // bytes_read = read_data(inode_index, file_position, (uint8_t*)buf, nbytes);
    // file_desc.file_position += bytes_read;
    // global[fd] = file_desc;
    // return bytes_read;
    uint32_t bytes_read;
    if(dentry_block->file_type ==2){
        bytes_read = read_data(boot_block->num_inodes, 0, buf, inode_block->file_size);
        return 0;
    }
    return -1;
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
*   dir_read
*   DESCRIPTION: Read the directory
*   INPUTS: fd - the file descriptor
*           buf - the buffer to store the data
*           nbytes - the number of bytes to read
*   OUTPUTS: buf - the buffer to store the data
*   RETURN VALUE: 1 if success, 0 if fail
*   SIDE EFFECTS: none
*/
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes){
    int32_t bytes_read = 0;
    int32_t dir_position;
    file_descriptor_t file_desc = global[fd];
    dir_position = file_desc.file_position;
    if(dir_position >= boot_block->num_dir_entries){
        return 0;
    } 
    memcpy(buf, dentry_block[dir_position].file_name, MAX_FILE_NAME);
    file_desc.file_position++;
    global[fd] = file_desc;
    return 1;
}

/*
*   dir_read2
*   DESCRIPTION: Read the directory
*   INPUTS: offset - the offset of the directory
*   OUTPUTS: none
*   RETURN VALUE: the size of the file
*   SIDE EFFECTS: none
*/
int32_t dir_read2 (uint32_t offset){
    uint32_t num_inode;
    memcpy(&num_inode, (int8_t*)(start)+100+64*offset, 4);
    int8_t* addr = (int8_t*)start+4096+(num_inode*4096);
    uint32_t file_size =0;
    memcpy(&file_size, addr, 4);
    return file_size;
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

