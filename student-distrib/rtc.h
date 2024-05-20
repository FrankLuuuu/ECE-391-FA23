#include "lib.h"
#include "i8259.h"

//initialize
//set the registers
void rtc_init();

//send interupts
//periodically (every second) send an interupt to increment time
void rtc_irq_handler();

//waits until interupt occurs/finishes
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

//changes the frequecy of interupts
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

//sets to default freq
int32_t rtc_open(const uint8_t* filename);

//closes rtc device driver
int32_t rtc_close(int32_t fd);

//change clock frequency
//calculate new frequency
void rtc_chg_clk_f(int rate);

//calculate log of value
int log2(int buf);
