#define ASM     1
#include "x86_desc.h"

.globl sys_call_linkage

sys_call_linkage:
    pushfl

    pushl %ecx
    pushl %edx
    pushl %ebx

    pushl %esp
    pushl %ebp
    pushl %esi
    pushl %edi

    pushl %edx
    pushl %ecx
    pushl %ebx

    # checks for valid sys call #
    cmp $0, %eax      
    jz error
    cmp $10, %eax      
    ja error

    # jump to the system call #
    call *sys_call_jumptable(, %eax, 4)   
    jmp done

# return -1 if there are errors
error:
    movl $-1, %eax       

# reset the stack
done:
    popl %edx
    popl %ecx
    popl %ebx     

    popl %edi
    popl %esi
    popl %ebp
    popl %esp

    popl %ebx
    popl %edx
    popl %ecx

    popfl

    iret

# store each sys call function
sys_call_jumptable:
    .long 0x00, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
    
