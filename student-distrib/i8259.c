/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the PIC */
void i8259_init(void) {
    master_mask = ALL_MASK;
    slave_mask = ALL_MASK;

    // initialize primary PIC
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT + NEXT_PORT);
    outb(ICW3_MASTER, MASTER_8259_PORT + NEXT_PORT);
    outb(ICW4, MASTER_8259_PORT + NEXT_PORT);

    // initialization secondary PIC
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + NEXT_PORT);
    outb(ICW3_SLAVE, SLAVE_8259_PORT + NEXT_PORT);
    outb(ICW4, SLAVE_8259_PORT + NEXT_PORT);

    // restore masks
    outb(master_mask, MASTER_8259_PORT + NEXT_PORT);
    outb(slave_mask, SLAVE_8259_PORT + NEXT_PORT);

    // enable secondary PIC
    enable_irq(SECONDARY);
}

/* Enable the specified IRQ */
void enable_irq(uint32_t irq_num) {
    //check if the irq is valid
    if((irq_num < 0) | (irq_num > 15))
        return;

    cli();

    if(irq_num >= 8){
        slave_mask = slave_mask & ~(1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + NEXT_PORT);
    }
    else {
        master_mask = master_mask & ~(1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + NEXT_PORT);
    }

    sti();
}

/* Disable the specified IRQ */
void disable_irq(uint32_t irq_num) {
    //check if the irq is valid
    if((irq_num < 0) | (irq_num > 15))
        return;
        
    cli();

    if(irq_num >= 8){
        slave_mask = slave_mask | ~(1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_PORT + NEXT_PORT);
    }
    else {
        master_mask = master_mask | ~(1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + NEXT_PORT);
    }

    sti();
}

/* Send end-of-interrupt signal*/
void send_eoi(uint32_t irq_num) {
    //check if the irq is valid
    if((irq_num < 0) | (irq_num > 15))
        return;
        
    if(irq_num >= 8){
        outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
        outb(EOI | SECONDARY, MASTER_8259_PORT);
       
    }
    else { 
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
}
