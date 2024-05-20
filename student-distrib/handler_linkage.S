#define ASM 1

.global rtc_linkage, keyboard_linkage

# call rtc handler 
rtc_linkage:
    pushal
    pushfl
    call rtc_irq_handler
    popfl
    popal
    iret

# call keyboard handler 
keyboard_linkage:
    pushal
    pushfl
    call keyboard_irq_handler
    popfl
    popal
    iret
