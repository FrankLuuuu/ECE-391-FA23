#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "types.h"
#include "paging.h"


#define BUFFER_SIZE 128 //buffer size for keyboard
#define BACKSPACE 0x08 //backspace ascii code
#define MAX_TERM 3     //max number of terminals
#define FOURKB 0x1000
#define fourmb              0x400000
#define eightmb             0x800000
#define fourkb              0x1000
#define six          6
typedef struct 
{ 
    int32_t prog_count;
    int32_t prog_table[six];

    uint8_t buffer[BUFFER_SIZE];

    int32_t screen_x;
    int32_t screen_y;

    uint32_t esp;
    uint32_t ebp;

    volatile int32_t characterCounter;
    volatile int32_t start_read;
    
    int32_t numBackspaces;

    int pid;
    int ppid;

    int32_t vid_mem_addr;
} terminal_t;

extern void term_init ();
//terminal open
extern int32_t term_open (const uint8_t* filename);
//terminal write
extern int32_t term_close (int32_t filename);
//terminal read
extern int32_t term_read (int32_t fd, void* buf, int32_t nbytes);
//terminal write
extern int32_t term_write (int32_t fd, const void* buf, int32_t nbytes);
//register typed character and put it in buffer
extern void register_typedChar(char typed);

extern void terminal_switch(int new_term);

extern void update_vid_mem_paging(int term);

terminal_t terminals[MAX_TERM];

int curr_term;
int sc_term;

#endif

