#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "i8259.h"
#include "lib.h"
#include "types.h" 
#include "terminal.h"
#include "sys_call.h"

#define KEYBOARD_PORT 0x60
#define KEYBOARD_SCANCODES 58
#define CAPS_PRESSED 0x3A
#define LSHIFT_PRESSED    0x2A
#define LSHIFT_RELEASED  0xAA
#define RSHIFT_PRESSED  0x36
#define RSHIFT_RELEASED 0xB6
#define CTRL_PRESSED 0x1D
#define CTRL_RELEASED 0x9D
#define ALT_PRESSED 0x38 
#define ALT_RELEASE 0xB8
#define BACKSPACE 0x08
#define BUFFER_SIZE 128
#define ENTER '\n'
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

extern void init_keyboard();
extern void keyboard_irq_handler();

#endif
