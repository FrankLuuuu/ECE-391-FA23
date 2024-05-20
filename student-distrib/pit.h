#include "lib.h"
#include "i8259.h"
#include "sys_call.h"


//initialize
//set the registers
void pit_init();

//send interupts
//periodically (every second) send an interupt to increment time
void pit_irq_handler();
// void pit_sleep(uint32_t millis);

//waits until interupt occurs/finishes
// int32_t pit_read(int32_t fd, void* buf, int32_t nbytes);

// //changes the frequecy of interupts
// int32_t pit_write(int32_t fd, const void* buf, int32_t nbytes);

// //sets to default freq
// int32_t pit_open(const uint8_t* filename);

// //closes pit device driver
// int32_t pit_close(int32_t fd);


// we needf  for pit
// pit sleep check how wde need to calculate, currently is how osdev sets it up
