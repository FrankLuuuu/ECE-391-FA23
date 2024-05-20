#include "pit.h"

//used https://wiki.osdev.org/Programmable_Interval_Timer and http://www.osdever.net/bkerndev/Docs/pit.htm as a reference

// DEFINING MAGIC NUMBERS


int irq0 = 0;               //irq number
int channel0 = 0x40;           //Channel 0 data port (read/write)
int modedcommane = 0x43;       //  Mode/Command register (write only, a read is ignored)
int mode = 0x36;            //canel 0 ,Access mode: lobyte/hibyte,Mode 3 (square wave generator, same as 011b) //rate gen
int spped = 64;             //power od 2 speed for the pit (this is the frequency)

uint8_t shift8 = 8;
// int rcon = 16;              //rate conversion
        
extern void TLB_flushing();

/* pit_init
 * 
 * Initialzes PIT interupts are working
 * Inputs: None									
 * Outputs: none
 * Side Effects: sets time
 * Coverage: initializes clock and sets time
 * Files: pit.c/pit.h					
 */
void pit_init() {                //need to define what the sepd is
    // cli();
    
    outb(mode, modedcommane);	        // set what mode
    
    // // char prev=inb(CMOS);	        // read the current value of register B
    // outb(spped, channel0);		    // setting to freq/speed of high bit
    // outb(spped >> shift8, channel0);	    // sett low byte of channel speed

    // enable_irq(irq0);		        // (perform an STI) and reenable NMI if you wish]

    // // sti();
    // return;   

    int reload = 11931;
      // PIT operates with 16-bit values, so split into lower and upper bytes
    //extract lower and and upper bute and send to IO ports
    uint8_t lower_byte = (uint8_t)(reload & 0xFF);
    uint8_t upper_byte = (uint8_t)((reload >> 8) & 0xFF);

    // Set the reload value for Channel 0
    outb(lower_byte, 0x40);  // Assuming 0x40 is the I/O port for Channel 0
    outb(upper_byte, 0x40);  

    enable_irq(irq0);    

    //sti();    
}




//send interupts

/* pit_irq_handler
 * 
 * periodically  send an interupt to queue scheduling, c
 * Inputs: None									
 * Outputs: none
 * Side Effects: increments time, switches process for scehduling
 * Coverage: sets time
 * Files: pit.c/pit.h					
 */
void pit_irq_handler() {
    cli();

    // printf("before asm");
    
    uint32_t esp;
    uint32_t ebp;
    // asm volatile("movl %%esp, %0;" 
    //             "movl %%ebp, %1;" 
    //             : 
    //             :
    //             "r" (esp),
    //             "r" (ebp)); 
    asm("\t movl %%esp, %0" : "=r"(esp));
    asm("\t movl %%ebp, %0" : "=r"(ebp));

    // save esp ebp pid into terminals struct
    terminals[sc_term].pid = cur_pid;
    terminals[sc_term].esp = esp;
    terminals[sc_term].ebp = ebp;

    // round robin terminals
    sc_term += 1;
    if(sc_term > 2)
        sc_term = 0;
  
    // get the esp ebp pid of new term
    int pid_next = terminals[sc_term].pid;
    esp = terminals[sc_term].esp;
    ebp = terminals[sc_term].ebp;

    // if other terms arent running, dont switch 
    if(pid_next == -1 || esp == 0 || ebp == 0)
    {
        //printf("HELLO");
        sc_term = 0;
        send_eoi(irq0);
        sti();
        return;
    }

    update_vid_mem_paging(sc_term);

    //remap paging with pid
    page_dir[USER_I].addr = ((mem_offset + pid_next) * fourmb) / fourkb;
    TLB_flushing();

    // store into tss
    tss.ss0 = KERNEL_DS;
    tss.esp0 = eightmb - (eightkb * cur_pid) - four;

    // context switch
    asm volatile("movl %0, %%esp;" 
                "movl %1, %%ebp;" 
                : 
                :
                "r" (esp),
                "r" (ebp)); 

    send_eoi(irq0);

    sti();
}




