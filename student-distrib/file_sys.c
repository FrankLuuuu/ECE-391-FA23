#include "file_sys.h"
#include "sys_call.h"

/* files_init
 * Inputs: the start of the filesys
 * Return Value: always 0
 * Function: set the structures and global variables
 */
int32_t files_init(uint32_t fs_start_ptr)
{
    //TODO: finish initializing eveyrthing -- figure out the filesys_img memory address stuff
    boot_block = (boot_block_t*)fs_start_ptr;
    inode_start = (inode_t*)(fs_start_ptr + DBLOCK_SIZE);
    data_block = (data_block_t*)(fs_start_ptr + (boot_block->inodes_count + 1) * DBLOCK_SIZE);

    dentry_num = boot_block->dir_count;
    inode_num = boot_block->inodes_count;
    data_block_num = boot_block->data_count;

    return 0;
}

/* read_dentry_by_name
 * Inputs: the file name, the dentry to return
 * Return Value: 0 if success, -1 if fail
 * Function: call by_index to find and return the dentry by file name
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
    int i = 0;
    int foundFile = -1; 
    uint32_t fname_len = strlen((int8_t*)fname);

    if(fname_len == 0 || fname_len > FILENAME_LEN)
        return foundFile; 

    // iterate through the dentries and find by name
    for(i = 0; i < dentry_num; i++) {
        int8_t* file_name = (int8_t*)boot_block->direntries[i].filename;
        uint32_t cur_length = strlen(file_name);
        
        if(FILENAME_LEN < cur_length)
            cur_length = FILENAME_LEN;

        // get the dentry with the index
        if(cur_length == fname_len) {
            if(!strncmp((int8_t*)fname, file_name, FILENAME_LEN)) {
                read_dentry_by_index(i, dentry);
                foundFile = 0;
                break;
            }
        }
    }

    return foundFile;
}

/* read_dentry_by_index
 * Inputs: the dentry index, the dentry to return
 * Return Value: 0 if success, -1 if fail
 * Function: find and return the dentry by the index
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
    if(index < 0 || index > dentry_num)
        return -1;

    // uint8_t file_dest = dentry->filename;
    // uint8_t file_source = (int8_t*)boot_block->direntries[index].filename;

    // copy the file name
    strncpy((int8_t*)dentry->filename, (int8_t*)boot_block->direntries[index].filename, FILENAME_LEN); 

    // store the members of the dentry
    dentry->filetype = boot_block->direntries[index].filetype; 
    dentry->inode_num = boot_block->direntries[index].inode_num;

    // int reservedSize = 24;

    //I guess we don't need

    // memcpy(dentry[index]->reserved, boot_block[0].dentry[index].reserved, reservedSize); //Frank: what is this for?

    return 0;
}

/* fs_read_data
 * Inputs: the inode index, the offset of block, the buffer to store data, the length in bytes
 * Return Value: the # of bytes read
 * Function: read the file by inode and store the content of the file
 */
int32_t fs_read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    if (inode >= inode_num) {                   
        return -1;
    }

    // initiate the data block and inode variables
    uint32_t block_index;                        
    uint32_t block_offset;                        
    inode_t* curr_inode; 
    data_block_t* curr_block;                      
    int i;

    curr_inode = inode_start + inode;              
    for(i = 0; i < length; i++) {
        // see if the end of the file is reached
        if ((i + offset) >= curr_inode->length) {  
            return i;
        }

        // calculate the index and offset of the data block
        // get the data block and store the data in the buffer
        block_index = (offset + i) / DBLOCK_SIZE;
        curr_block = data_block + curr_inode->data_block_num[block_index];
        block_offset = (offset + i) % DBLOCK_SIZE;
        buf[i] = curr_block->data[block_offset];  
    }

    return length;
}

/* fs_dir_read
 * Inputs: the file descriptor, the buffer to store files, the # of bytes
 * Return Value: the # of bytes read, -1 if fail
 * Function: find and get the file name by index
 */
int32_t fs_dir_read(int32_t fd, void* buf, int32_t nbytes)
{
    //TODO: review please
    //I don't know if this is right, but fd is an index in direntries that we have?
    //i'm implementing this function with this in mind

    //so we'll use read_dentry_by_index from this function as instructed in discussion video
    //int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
	// printf("%d", fd);

    dentry_t dentry;
    // int32_t bytes;                 
    // inode_t* curr_inode; 
    pcb_t* pcb = pcb_getactpcb();
    if (pcb->fd_table[fd].flag)
        return 0;

    // get the current file position
    int pos = pcb->fd_table[fd].file_pos;

    // find the file by index and store the dentry
    int retVal = read_dentry_by_index(pos, &dentry);
    if(retVal == -1)
        return 0;

    // copy the file name to the buffer and store file length
    strncpy((int8_t*)buf, (int8_t*)&(dentry.filename), FILENAME_LEN);
	// printf("File name: %s\n", buf);

    // update file position
    pos++;
    pcb->fd_table[fd].file_pos = pos;
    // curr_inode = inode_start + dentry.inode_num;
    // bytes = curr_inode->length;

    // return the bytes read
    if(strlen((int8_t*)&(dentry.filename)) < FILENAME_LEN)
        return strlen((int8_t*)&(dentry.filename));
    return FILENAME_LEN;
}

/* fs_file_read
 * Inputs: the file descriptor, the buffer to store files, the # of bytes
 * Return Value: the # of bytes read, -1 if fail
 * Function: call read data to get the content of file
 */
// int32_t fs_file_read(int32_t fd, const void* buf, int32_t nbytes)
int32_t fs_file_read(int32_t fd, void* buf, int32_t nbytes)
{
    //TODO: review please
   //I don't know if this is right, but fd is an index in direntries that we have?
    //i'm implementing this function with this in mind

    //so we'll call read_data from this function as instructed in discussion video

    // if(!buf)
    //     return -1;
    // uint32_t inode = pcb.fd_table[fd].inode;
    // uint32_t offset = pcb.fd_table[fd].file_pos;

    // check if the file exists and store the inode and position of file

    pcb_t* pcb = pcb_getactpcb();
    if (pcb->fd_table[fd].flag)
        return -1;
	
    // get the inode and offset
    uint32_t inode = pcb->fd_table[fd].inode;
    uint32_t offset = pcb->fd_table[fd].file_pos; 

    // call read data to store the content of the file
    uint32_t bytes = fs_read_data(inode, offset, (uint8_t*)buf, (uint32_t)nbytes);

    // update the file position
    if(bytes != -1)
        pcb->fd_table[fd].file_pos += bytes;

    return bytes;
}

/* fs_dir_open
 * Inputs: the file name
 * Return Value: 0
 * Function: do nothing
 */
int32_t fs_dir_open(const uint8_t* filename)
{
    return 0;
}

/* fs_dir_close
 * Inputs: the file name
 * Return Value: 0
 * Function: do nothing
 */
int32_t fs_dir_close(int32_t fd)
{
    return 0;
}

/* fs_dir_write
 * Inputs: the file descriptor, buffer to store, the bytes to write
 * Return Value: -1
 * Function: do nothing
 */
int32_t fs_dir_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/* fs_file_write
 * Inputs: the file descriptor, buffer to store, the bytes to write
 * Return Value: -1
 * Function: do nothing
 */
int32_t fs_file_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/* fs_file_open
 * Inputs: the file name
 * Return Value: 0
 * Function: do nothing
 */
int32_t fs_file_open(const uint8_t* filename)
{
    return 0;
}

/* fs_file_close
 * Inputs: the file name
 * Return Value: 0
 * Function: do nothing
 */
int32_t fs_file_close(int32_t fd)
{
    return 0;
}
