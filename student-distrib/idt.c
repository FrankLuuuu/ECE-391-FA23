#include "idt.h"

/* init_idt
 * 
 * Initializes the idt
 * Inputs: None
 * Outputs: None
 * Side Effects: fills out the idt appropriately and loads the idt
 */
void init_idt() {
    // initialize idt entries
    int i;
    for(i = 0; i < NUM_VEC; i++) {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;
        idt[i].reserved0 = 0; 
        idt[i].dpl = 0;
        idt[i].present = 0;

        if(i == SYSCALL_IDT)  
            idt[i].dpl = 3;
        else if(i == RTC_IDT || i == KEYBOARD_IDT) 
            idt[i].reserved3 = 1;
        if(i <= 19 || i == SYSCALL_IDT || i == RTC_IDT || i == KEYBOARD_IDT) 
            idt[i].present = 1;
    }

    // store exceptions, interrupts, and system call 
    SET_IDT_ENTRY(idt[0], exc_div_error);
    SET_IDT_ENTRY(idt[1], exc_debug);
    SET_IDT_ENTRY(idt[2], exc_nonmask_int);
    SET_IDT_ENTRY(idt[3], exc_breakpoint);
    SET_IDT_ENTRY(idt[4], exc_overflow);
    SET_IDT_ENTRY(idt[5], exc_bound_range_exc);
    SET_IDT_ENTRY(idt[6], exc_inv_op);
    SET_IDT_ENTRY(idt[7], exc_dev_not_avai);
    SET_IDT_ENTRY(idt[8], exc_dbl_fault);
    SET_IDT_ENTRY(idt[9], exc_cop_seg_over);
    SET_IDT_ENTRY(idt[10], exc_inv_tss);
    SET_IDT_ENTRY(idt[11], exc_seg_not_pres);
    SET_IDT_ENTRY(idt[12], exc_stackseg_fault);
    SET_IDT_ENTRY(idt[13], exc_gen_prot_fault);
    SET_IDT_ENTRY(idt[14], exc_pg_fault);
    SET_IDT_ENTRY(idt[15], exc_res);
    SET_IDT_ENTRY(idt[16], exc_x87_float_exc);
    SET_IDT_ENTRY(idt[17], exc_align_check);
    SET_IDT_ENTRY(idt[18], exc_mach_check);
    SET_IDT_ENTRY(idt[19], exc_simd_float_exc);
    SET_IDT_ENTRY(idt[RTC_IDT], rtc_linkage);
    SET_IDT_ENTRY(idt[KEYBOARD_IDT], keyboard_linkage);
    SET_IDT_ENTRY(idt[SYSCALL_IDT], sys_call_linkage);

    lidt(idt_desc_ptr);
}

// void syscall_temp() {
//     cli();

//     printf("System Call");
//     halt(HALT);

//     sti();
// }

void exc_div_error() {
    cli();

    printf("Division Error!");
    halt(HALT);

    sti();
}

void exc_debug() {
    cli();

    printf("Debug!");
    halt(HALT);

    sti();
}

void exc_nonmask_int() {
    cli();

    printf("Non-maskable Interrupt!");
    halt(HALT);

    sti();
}

void exc_breakpoint() {
    cli();

    printf("Breakpoint!");
    halt(HALT);

    sti();
}

void exc_overflow() {
    cli();

    printf("Overflow!");
    halt(HALT);

    sti();
}

void exc_bound_range_exc() {
    cli();

    printf("Bound Range Exceeded!");
    halt(HALT);

    sti();
}

void exc_inv_op() {
    cli();

    printf("Invalid Opcode!");
    halt(HALT);

    sti();
}

void exc_dev_not_avai() {
    cli();

    printf("Device Not Available!");
    halt(HALT);

    sti();
}

void exc_dbl_fault() {
    cli();

    printf("Double Fault!");
    halt(HALT);

    sti();
}

void exc_cop_seg_over() {
    cli();

    printf("Coprocessor Segment Overrun!");
    halt(HALT);

    sti();
}

void exc_inv_tss() {
    cli();

    printf("Invalid TSS!");
    halt(HALT);

    sti();
}

void exc_seg_not_pres() {
    cli();

    printf("Segment Not Present!");
    halt(HALT);

    sti();
}

void exc_stackseg_fault() {
    cli();

    printf("Stack-Segment Fault!");
    halt(HALT);

    sti();
}

void exc_gen_prot_fault() {
    cli();

    printf("General Protection Fault!");
    halt(HALT);

    sti();
}

void exc_pg_fault() {
    cli();

    printf("Page Fault!");
    halt(HALT);

    sti();
}

void exc_res() {
    cli();

    printf("Reserved Execption");
    halt(HALT);

    sti();
}

void exc_x87_float_exc() {
    cli();

    printf("x87 Floating-Point Exception!");
    halt(HALT);

    sti();
}

void exc_align_check() {
    cli();

    printf("Alignment Check!");
    halt(HALT);

    sti();
}

void exc_mach_check() {
    cli();

    printf("Machine Check!");
    halt(HALT);

    sti();
}

void exc_simd_float_exc() {
    cli();

    printf("SIMD Floating-Point_exception!");
    halt(HALT);

    sti();
}
