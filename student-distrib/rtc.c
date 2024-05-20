#include "rtc.h"

//used https://wiki.osdev.org/RTC as a reference

// DEFINING MAGIC NUMBERS

int rtc_flag = 0;           //rtc_flag determines if an interupt has occurred or is occuring

int def_freq = 2;           //default (and min) frequency is 2 Hz
int max_freq = 1024;        //max freq is 1024

int regA = 0x8A;            //register A with NMI disabled
int regB = 0x8B;            //register B with NMI disabled
int regC = 0x0C;            //register C

int regport = 0x70;         //chooses which register to change
int CMOS = 0x71;            //allows reading/writing to register

int bit6 = 0x40;            //bitmask, brabs bit 6

int lmask = 0x0F;           //grab low for bits
int hmask = 0xF0;           //grab upper for bits

int irq8 = 8;               //irq number

int rcon = 16;              //rate conversion
        


/* rtc_init
 * 
 * Checks if RTC interupts are working
 * Inputs: None									
 * Outputs: none
 * Side Effects: sets time
 * Coverage: initializes clock and sets time
 * Files: rtc.c/rtc.h					
 */
void rtc_init(){
    cli();
    outb(regB, regport);	        // select Status Register B, and disable NMI (by setting the 0x80 bit)
    
    char prev=inb(CMOS);	        // read the current value of register B
    outb(regB, regport);		    // set the index again (a read will reset the index to register D)
    outb(prev | bit6, CMOS);	    // write the previous value ORed with 0x40. This turns on bit 6 of register B
    enable_irq(irq8);		        // (perform an STI) and reenable NMI if you wish]
    sti();
    return;             
}




//send interupts
//periodically (every second) send an interupt to increment time, clear register C to allow it to be recalled

/* rtc_irq_handler
 * 
 * periodically (every second) send an interupt to increment time, clear register C to allow it to be recalled
 * Inputs: None									
 * Outputs: none
 * Side Effects: increments time, wipes register c
 * Coverage: sets time
 * Files: rtc.c/rtc.h					
 */
void rtc_irq_handler(){             //allows interupts in the future VERY IMPORTANT!!
    cli();
    outb(regC, regport);	        // select register C
    inb(CMOS);		                // just throw away contents

    rtc_flag = 1;                   //set flag, interupt finished
   
    //fortesting ckpt 1
    // test_interrupts();           //test rtc goes here      ckpt 1 rtc test uncomment when testing!!
    send_eoi(irq8);
    sti();
  
    return;
}




// ///all functions should return 0 for success or -1 for fail

/* rtc_read
 * 
 * waits until rtc interupt has occuured
 * Inputs: file descriptor, buffer for data passed in and number of byts of data									
 * Outputs: 0 for success
 * Side Effects: None
 * Coverage: waits for interupt
 * Files: rtc.c/rtc.h					
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    //should always return 0, but only after the interupt has been occured 
        //(HINT! set flac and wait until interupt hancdler clears it and return 0)
    //should use a jump table referenced ub  open system call   ??
    // sti();

    rtc_flag = 0;                   //set interupt in process
    while (rtc_flag == 0){}         //interupt occurred

    // cli();
    return 0;                       //success
}




/* rtc_write
 * 
 * changes rate of interupts and checks if parameters are valid
 * Inputs: file descriptor, buffer for data passed in and number of byts of data									
 * Outputs: -1 for inbalid param, 0 for success
 * Side Effects: changes frequency
 * Coverage: validates parameters and sets rate
 * Files: rtc.c/rtc.h					
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){

    if (*(int*)buf == NULL || nbytes  != sizeof(int32_t))       //checks if parameters are valid
        return -1;

    if (*(int*)buf < def_freq || *(int*)buf > max_freq)                    //check if freq is in range
        return -1;

    //check id regular file (nonwritable file)      return -1;

    if ((*(int*)buf & (*(int*)buf >> 1)) != 0)                  // checks if the frequency is a power of 2
        return -1;


    rtc_chg_clk_f(*(int*)buf);                                  //set to param rate
   
    return 0;                                                   //return success

}




/* rtc_open
 * 
 * sets default frequency
 * Inputs: fileneam, name of file to open									
 * Outputs: 0 for success
 * Side Effects: sets default freq
 * Coverage: sets initial freq
 * Files: rtc.c/rtc.h					
 */
int32_t rtc_open(const uint8_t* filename){
    rtc_chg_clk_f(def_freq);                                    //set to default freq of 2      jmp table for virtualization?
    return 0;
}




/* rtc_close
 * 
 * Inputs: file descriptor									
 * Outputs: 0 for success
 * Side Effects: none
 * Coverage: closes
 * Files: rtc.c/rtc.h					
 */
int32_t rtc_close(int32_t fd){
    return 0;
}




// change clock frequency         mp pt2       
// calculate new frequency

/* rtc_chg_clk_f
 * 
 * Inputs: buf (frequency to change to)									
 * Outputs: none
 * Side Effects: changes register A
 * Coverage: converst freq to rate and saves in registerA
 * Files: rtc.c/rtc.h					
 */
void rtc_chg_clk_f(int buf){                    //change param order
    cli();

    int rate = rcon - log2(buf);                  ////converts to rate from freq      32768 >> (rate-1);
    
    // if (rate < 6 || rate > 15)                  //param verivcation
    //     return;                                 //dont change rate if its unvaild

    //change interupt rate in register A
    rate &= lmask;			                    // rate must be between 6 and 15 (2-1024 Hz)
    outb(regA, regport);		                // set index to register A, disable NMI
    char prev=inb(CMOS);	                    // get initial value of register A
    outb(regA, regport);		                // reset index to A
    outb((prev & hmask) | rate, CMOS);          //write only our rate to A. Note, rate is the bottom 4 bits.
    sti();
}




/* log2
 * 
 * Inputs: buf(frequency)									
 * Outputs: log of the value
 * Side Effects: finds the power of 2
 * Coverage: log of input value
 * Files: rtc.c/rtc.h					
 */
int log2(int buf){
    int count = 0;                              //log power of two counter

    while (buf != 1){                           //divide by 2 until buf = 1;
        buf /= def_freq;
        count++;                                //count is the power of 2
    }

    return count;
}
