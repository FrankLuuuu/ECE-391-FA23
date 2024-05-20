#ifndef _IDT_H
#define _IDT_H

#include "lib.h"
#include "x86_desc.h"
#include "handler_linkage.h"
#include "sys_call_linkage.h"
#include "sys_call.h"

#define KEYBOARD_IDT    0x21
#define RTC_IDT         0x28
#define SYSCALL_IDT     0x80
#define EXC_TOT         20
#define HALT            1

extern void init_idt();

// extern void syscall_temp(void);

extern void exc_div_error(void);
extern void exc_debug(void);
extern void exc_nonmask_int(void);
extern void exc_breakpoint(void);
extern void exc_overflow(void);
extern void exc_bound_range_exc(void);
extern void exc_inv_op(void);
extern void exc_dev_not_avai(void);
extern void exc_dbl_fault(void);
extern void exc_cop_seg_over(void);
extern void exc_inv_tss(void);
extern void exc_seg_not_pres(void);
extern void exc_stackseg_fault(void);
extern void exc_gen_prot_fault(void);
extern void exc_pg_fault(void);
extern void exc_res(void);
extern void exc_x87_float_exc(void);
extern void exc_align_check(void);
extern void exc_mach_check(void);
extern void exc_simd_float_exc(void);

#endif /* _IDT_H */
