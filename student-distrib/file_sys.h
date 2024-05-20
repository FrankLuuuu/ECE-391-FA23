#ifndef _FILES_H
#define _FILES_H

#include "types.h"
#include "lib.h"

#define FILENAME_LEN 32
#define DENTRY_SIZE 63 
#define DBLOCK_SIZE 4096

/* directory entries in boot block */
typedef struct dentry {
    uint8_t filename[FILENAME_LEN]; // file names
    uint32_t filetype; // file types
    uint32_t inode_num; // inode #
    uint8_t reserved[24]; // reserved
} dentry_t;

/* boot block of the file system structure */
typedef struct boot_block {
    uint32_t dir_count; // # of dir entries
    uint32_t inodes_count; // # inodes
    uint32_t data_count; // # data blocks
    uint8_t reserved[52]; // reserved
    dentry_t direntries[63]; // 1 for directory, 62 for files
} boot_block_t;

/* inode of each file */
typedef struct inode {
    uint32_t length; // length in bytes
    uint32_t data_block_num[1023]; // data block #
} inode_t;

/* data block of a file */
typedef struct data_block {
    uint8_t data[4096]; // 4kb
} data_block_t;

/* ini file sys */
extern int32_t files_init(uint32_t fs_start_ptr);
/* search the given bit and find dentry structrue whose name is fname */
extern int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
/* search dentry by index */
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
/* read data from specific place and copy to the given pointer */
extern int32_t fs_read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* directory read/write */
extern int32_t fs_dir_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t fs_dir_read(int32_t fd, void* buf, int32_t nbytes);

/* directory open/close */
extern int32_t fs_dir_open(const uint8_t* filename);
extern int32_t fs_dir_close(int32_t fd);

/* file open/close */
extern int32_t fs_file_open(const uint8_t* filename);
extern int32_t fs_file_close(int32_t fd);

/* file read/write */
extern int32_t fs_file_write(int32_t fd, const void* buf, int32_t nbytes);
// extern int32_t fs_file_read(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t fs_file_read(int32_t fd, void* buf, int32_t nbytes);



//TODO: DON'T KNOW IF THESE ARE STRUCTS OR POINTERS TO STRUCTS
boot_block_t* boot_block;
inode_t* inode_start;
data_block_t* data_block;
// extern pcb_t pcb[];

uint32_t dentry_num;                     
uint32_t inode_num;                     
uint32_t data_block_num;                      


#endif
