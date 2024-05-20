/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

// explicit declaration of function used in this file
int tuxctl_ioctl (struct tty_struct* tty, struct file* file, unsigned cmd, unsigned long arg);
int tux_init (struct tty_struct* tty);
int tux_led(struct tty_struct* tty, unsigned long arg);
int tux_buttons(struct tty_struct* tty, unsigned long arg);


static unsigned char modes[] = {MTCP_BIOC_ON, MTCP_LED_USR}; // enable button interrupt-on-change and usr mode
//static unsigned char buttons[2]; // store the two bytes of buttons pressed
static unsigned long leds; // store the states of the leds
static int acked = 0; // indicates if a command is successfully completed
unsigned long button; // stores the button states
/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */

void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned int l, d;
    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];


	switch(a){
		/* acknowledge that the command is successful */
		case MTCP_ACK:
			acked = 0;
			break;

		case MTCP_BIOC_EVENT:
			l = (c & Left_mask) >> Leftoffset; // store 5th and 6th bits
			d = (c & Down_mask) >> Downoffset;

			/* swap 5th and 6th bits with masked input bytes. */
			button = ((b & 0x0F) | ((c & 0x09) << 4) | (l << 6) | (d << 5));
			break;

		/* reset the values of led and display modes */
		case MTCP_RESET:
			acked = 0;
			tux_led(tty,leds);
			tuxctl_ldisc_put(tty, modes, 2);
			break ;

		default:
			return;
	}
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/

/* 
 * tuxctl_ioctl
 *   DESCRIPTION: tux io control function. switch to cases according to cmd
 *   INPUTS: struct tty_struct* tty -- tux port
 * 			 struct file* file 
	         unsigned cmd -- command issued by the tux
		     unsigned long arg -- package from tux
 *   OUTPUTS: none
 *   RETURN VALUE: 0 or -EINVAL
 *   SIDE EFFECTS: none
 */
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		tux_init(tty);
		return 0;

	case TUX_BUTTONS:
		return tux_buttons(tty, arg);

	case TUX_SET_LED:
		if(acked){
			return -EINVAL;
		}
		tux_led(tty, arg);
		return 0;
		
	case TUX_LED_ACK:
		return 0;
	case TUX_LED_REQUEST:
		return 0;
	case TUX_READ_LED:
		return 0;
	default:
	    return -EINVAL;
    }
}

//------------------------------------- initialization----------------------------//
/* 
 * tux_initial
 *   DESCRIPTION: initialize tux settings, clear button value and led save
 *   INPUTS: struct tty_struct* tty -- tux port
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int tux_init (struct tty_struct* tty){
	tuxctl_ldisc_put(tty, modes, 2);
	// initialize LED saved value 
	leds = 0;
	// intialize button value
	button = 0;
	//set ack to 0
	acked = 0;
	// success, return 0
	return 0;
}	

//------------------------------------- LED -----------------------------------------//
/* 
 * tux_LED
 *   DESCRIPTION: retrieve led information from arg, find correspoding
 * 				  bit, set tux led value according to mask
 *   INPUTS: struct tty_struct* tty -- tux port
 * 					unsigned long arg -- argument sent to tux
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int tux_led(struct tty_struct* tty,unsigned long arg){
/* mask to display each number from 1 to 16 { 0,1,2,3
										      4,5,6,7
										      8,9,A,b,
										      c,d,E,F }*/					
unsigned char sixten_number_mask[16] = {0xE7, 0x06, 0xCB, 0x8F, 
										0x2E, 0xAD, 0xED, 0x86, 
										0xEF, 0xAF, 0xEE, 0x6D, 
										0xE1, 0x4F, 0xE9, 0xE8};

// LED ON or OFF
unsigned int LED_value[4];
// LED buffer
unsigned char buf[6];
// The low 4 bits of the third byte specifies which LED’s should be turned on.
unsigned char LED_ON_OFF;
// The low 4 bits of the highest byte (bits 27:24) specify whether the corresponding decimal points should be turned on. 
unsigned char Decimal;
int i;
// led buffer initialization
buf[0] = MTCP_LED_SET;
buf[1] = 0xF;
buf[2] = 0;
buf[3] = 0;
buf[4] = 0;
buf[5] = 0;
// byte 0, lower 4 bit represent which led is on
LED_ON_OFF = ((arg &(0x0F<<LEDoffset)) >> LEDoffset);

Decimal = ((arg &(0x0F<<Decimaloffset)) >> Decimaloffset);
// If ack =1 return

// 	Mapping from 7-segment to bits
//  The 7-segment display is:
// ;		  _A
// ;		F| |B
// ;		  -G
// ;		E| |C
// ;		  -D .dp
// ;
// ; 	The map from bits to segments is:
// ; 
// ; 	__7___6___5___4____3___2___1___0__
// ; 	| A | E | F | dp | G | C | B | D | 
// ; 	+---+---+---+----+---+---+---+---+

// check which led is on
// Arguments: >= 1 bytes
// byte 0 - Bitmask of which LED's to set:
// __7___6___5___4____3______2______1______0___
// | X | X | X | X | LED3 | LED2 | LED1 | LED0 | 
// ----+---+---+---+------+------+------+------+
// Loop to find correct LED musk, store in array form
// The low 16-bits specify a number whose
// hexadecimal value is to be displayed on the 7-segment displays
for(i=0; i<4; i++){
	LED_value[i] = sixten_number_mask[(( (arg & LED_MASK) >> (4*i) )& 0x0F)];
}

// loop to load LED value if led is on
for(i=0; i<4; i++){
	if ((Onebit_mask & (LED_ON_OFF >> i)) != 0){
		buf[i+2] = LED_value[i];
	}
}

// loop to find decimal value
for(i=0; i<4; i++){
		buf[i+2] |= (((Decimal >> i)& Onebit_mask)<<4);
}
//save led value
leds = arg;
tuxctl_ldisc_put(tty, buf, 6);
	return 0;

}

//--------------------------------------- BUTTON--------------------------------------//
/* 
 * tux_button
 *   DESCRIPTION: retrieve led information from arg, find correspoding
 * 				  bit, copy tux button to user space
 *   INPUTS: struct tty_struct* tty -- tux port
 * 					unsigned long arg -- argument sent to tux
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int tux_buttons(struct tty_struct* tty,unsigned long arg){
	//check return value
	unsigned int user_copy;
	unsigned long* button_pointer;
	// change button value data type
	button_pointer= &(button);
	//copy button value to user space 
	user_copy = copy_to_user((void *)arg, (void*)button_pointer, sizeof(long));			
	// check return value
	if(user_copy == 0 ) 
		return 0;
	else 
		return -EFAULT;

}






// /* tuxctl-ioctl.c
//  *
//  * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
//  *
//  * Mark Murphy 2006
//  * Andrew Ofisher 2007
//  * Steve Lumetta 12-13 Sep 2009
//  * Puskar Naha 2013
//  */

// #include <asm/current.h>
// #include <asm/uaccess.h>

// #include <linux/kernel.h>
// #include <linux/init.h>
// #include <linux/module.h>
// #include <linux/fs.h>
// #include <linux/sched.h>
// #include <linux/file.h>
// #include <linux/miscdevice.h>
// #include <linux/kdev_t.h>
// #include <linux/tty.h>
// #include <linux/spinlock.h>

// #include "tuxctl-ld.h"
// #include "tuxctl-ioctl.h"
// #include "mtcp.h"

// #define debug(str, ...) \
// 	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

// static unsigned char modes[] = {MTCP_BIOC_ON, MTCP_LED_USR}; // enable button interrupt-on-change and usr mode
// static unsigned char buttons[2]; // store the two bytes of buttons pressed
// static unsigned char leds[6]; // store the states of the leds
// static int acked = 0; // indicates if a command is successfully completed

// /************************ Protocol Implementation *************************/

// int tux_init(struct tty_struct* tty);
// int tux_buttons(struct tty_struct* tty, unsigned long arg);
// int tux_led(struct tty_struct* tty, unsigned long arg);

// /* tuxctl_handle_packet()
//  * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
//  * tuxctl-ld.c. It calls this function, so all warnings there apply 
//  * here as well.
//  */
// void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
// {
//     unsigned char a, b, c;

//     a = packet[0]; /* Avoid printk() sign extending the 8-bit */
//     b = packet[1]; /* values when printing them. */
//     c = packet[2];

//     /*printk("packet : %x %x %x\n", a, b, c); */
// 	switch (a) {
// 		/* mark if a command is successfully completed */
// 		case MTCP_ACK:
// 			acked = 0;
// 			return;

// 		/* store the bytes of the buttons pressed */
// 		case MTCP_BIOC_EVENT:
// 			buttons[0] = b;
// 			buttons[1] = c;
// 			return;

// 		/* restore the led states and the state of the tux */
// 		case MTCP_RESET:
// 			acked = 0;
// 			tuxctl_ldisc_put(tty, leds, 6);
// 			tuxctl_ldisc_put(tty, modes, 2);
// 			buttons[0] = 0xFF;
// 			buttons[1] = 0xFF;
// 			return;

// 		default:
// 			return;
// 	}
// }


// /******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
//  *                                                                            *
//  * The ioctls should not spend any time waiting for responses to the commands *
//  * they send to the controller. The data is sent over the serial line at      *
//  * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
//  * transmit; this means that there will be about 9 milliseconds between       *
//  * the time you request that the low-level serial driver send the             *
//  * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
//  * arriving. This is far too long a time for a system call to take. The       *
//  * ioctls should return immediately with success if their parameters are      *
//  * valid.                                                                     *
//  *                                                                            *
//  ******************************************************************************/
// int 
// tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
// 	      unsigned cmd, unsigned long arg)
// {
//     switch (cmd) {
// 		case TUX_INIT:
// 			tux_init(tty);
// 			return 0;

// 		case TUX_BUTTONS:
// 			return tux_buttons(tty, arg);

// 		case TUX_SET_LED:
// 			if(!acked) {
// 				tux_led(tty, arg);
// 				return 0;
// 			}
// 			return -EINVAL;

// 		case TUX_LED_ACK:
// 			return -EINVAL;
// 		case TUX_LED_REQUEST:
// 			return -EINVAL;
// 		case TUX_READ_LED:
// 			return -EINVAL;
// 		default:
// 			return -EINVAL;
//     }
// }


// int tux_init(struct tty_struct* tty) {
// 	int i;
// 	acked = 0;
// 	buttons[0] = 0xFF;
// 	buttons[1] = 0xFF;
// 	for (i = 0 ; i < 6; i++) {
// 		leds[i] = 0;
// 	}
// 	tuxctl_ldisc_put(tty, modes, 2);

// 	return 0;
// }


// int tux_buttons(struct tty_struct* tty, unsigned long arg) {
// 	int buttons1, buttons2;
// 	int bit1, bit2;
// 	int xor;
// 	int result;

// 	if ((unsigned long*)arg == NULL) {
// 		return -EINVAL;                       
// 	}

// 	buttons1 = buttons[0];
// 	buttons2 = buttons[1];
	 
// 	/* swap bit 1 & 2 of buttons2 */
// 	bit1 = (buttons2 >> 1) & 1;
// 	bit2 = (buttons2 >> 2) & 1;
// 	xor = (bit1 ^ bit2);
// 	xor = (xor << 1) | (xor << 2);
// 	buttons2 = buttons2 ^ xor;

// 	result = (buttons1 & 0x0F) | ((buttons2 & 0x0F) << 4);
// 	copy_to_user((unsigned long*)arg, &result, sizeof(result));
// 	return 0;
// }


// int tux_led(struct tty_struct* tty, unsigned long arg) {
// 	int led_binary[16] = {0xE7, 0x06, 0xCB, //0, 1, 2
// 						0x8F, 0x2E, 0xAD, //3, 4, 5
// 						0xED, 0x86, 0xEF, //6, 7, 8
// 						0xAF, 0xEE, 0x6D, //9, A, B
// 						0xE1, 0x4F, 0xE9, 0xE8}; //C, D, E, F
// 	int on_led = (0x0F & (arg >> 16));
// 	int decimal = (0x0F & (arg >> 24));
// 	unsigned char led[6];
// 	int i;
// 	led[0] = MTCP_LED_SET;
// 	led[1] = 0xFF;

// 	for (i = 2; i < 6 ; i++) {
// 		if(!(on_led & (1 << (i - 2)))) {
// 			if(!(decimal & (1 << (i - 2)))) {
// 				led[i] = led_binary[(arg >> (4 * (i - 2))) & 0x0F] + 0x10;
// 			}
// 			else {
// 				led[i] = led_binary[(arg >> (4 * (i - 2))) & 0x0F];
// 			}
// 		}
// 	}

// 	tuxctl_ldisc_put(tty, led, i);
// 	for (i = 0 ; i < 6; i++) {
// 		leds[i] = led[i];
// 	}

// 	return 0;
// }



