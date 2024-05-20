#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "types.h"
#include "lib.h"
#include "file_sys.h"
#include "paging.h"
#include "terminal.h"
#include "rtc.h"
#include "x86_desc.h"

#define mem_offset          2
#define max_shells          6
#define four                4
#define eight               8

#define fourkb              0x1000
#define fourmb              0x400000
#define eightmb             0x800000
#define eightkb             0x2000

#define prog_img_virt_addr  0x08048000
#define prog_img_off        0x48000
#define pg_dir_virt_addr    0x08000000

/*file operation table*/
typedef struct {
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* fname);
    int32_t (*close)(int32_t fd);
} file_ops_t;
  
file_ops_t rtc;   
file_ops_t dir;   
file_ops_t file;  
file_ops_t stdin; 
file_ops_t stdout; 
file_ops_t null; 

/* file descriptor*/
typedef struct {
    file_ops_t* file_op; 
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flag;
} fd_t;

typedef struct {
    uint32_t pid;
    fd_t fd_table[8]; // process number
    uint32_t ppid;
    uint32_t eip; // user
    uint32_t esp;
    uint32_t esp0; // kernel
    uint32_t ebp0;
    uint32_t esp0_tss;

    uint8_t cmd_arg[8 * 4]; // stores the commandline arguments
} pcb_t;

extern void syscall_handler();

int32_t halt(uint8_t status);

int32_t execute(const uint8_t* command);

int32_t open(const uint8_t* filename);

int32_t close(int32_t fd);

int32_t read(int32_t fd, void* buf, int32_t nbytes);

int32_t write(int32_t fd, void* buf, int32_t nbytes);

int32_t getargs(uint8_t* buf, int32_t nbytes);

int32_t vidmap(uint32_t** screen_start);

int32_t set_handler(int32_t signum, void* handler_address);

int32_t sigreturn(void);

//a function to get the currently active pcb
pcb_t* pcb_getactpcb();

//a function to get the pcb given a pid
pcb_t* pcb_getpcb(uint32_t pid);

void init_file_ops();

int32_t null_read(int32_t fd, void* buf, int32_t nbytes);
int32_t null_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t null_open(const uint8_t* fname);
int32_t null_close(int32_t fd);


int cur_pid;
int parentPid;

#endif
