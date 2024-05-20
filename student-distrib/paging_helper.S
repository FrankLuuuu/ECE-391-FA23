#define ASM     1

.globl TLB_flushing, enable_paging
TLB_flushing:
    movl %cr3, %eax    
    movl %eax, %cr3
    ret

enable_paging:
  pushl %ebp
  movl  %esp, %ebp

  #load the page directory argument into eax
  movl  8(%ebp), %eax   
  #move the argument to cr3
  movl %eax, %cr3

  movl %cr4, %eax         # context switch
  orl  $0x00000010, %eax  
  movl %eax, %cr4

  movl %cr0, %eax
  orl  $0x80000001, %eax  
  movl %eax, %cr0

  movl %cr3, %eax         # reload cr3
  movl %eax, %cr3

  leave
  ret
