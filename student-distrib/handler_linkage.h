#ifndef _WRAPPER_HANDLERS_H
#define _WRAPPER_HANDLERS_H

#include "rtc.h"
#include "keyboard.h"

#ifndef ASM

/* call rtc handler */
void rtc_linkage();

/* call keyboard handler */
void keyboard_linkage();

#endif

#endif
