### P2 Solution
As part of MP2, you will also write a device driver for the Tux controller
boards in the lab. The documentation for this board can be found in the file
mtcp.h in the class directory under mp2.  You will need to read it for the
following questions.  Write your solution in p2/solution.md (Note this is a
markdown file so you can use markdown formatting if you like)
    
For each of the following messages sent from the computer to the Tux
controller, briefly explain when it should be sent, what effect it has on the
device, and what message or messages are returned to the computer as a result:
MTCP_BIOC_ON, MTCP_LED_SET.

#define MTCP_CMD_CHECK 0xC0
#define MTCP_CMD(c)  (MTCP_CMD_CHECK | (c))

#define MTCP_LED_SET  MTCP_CMD(0x6) //  1100 0110
#define MTCP_BIOC_ON   MTCP_CMD(0x3) // 1100 0011



For each of the following messages sent from the Tux controller to the
computer, briefly explain when the device sends the message and what
information is conveyed by the message: MTCP_ACK, MTCP_BIOC_EVENT,
MTCP_RESET.


Now read the function header for tuxctl_handle_packet in
tuxctl-ioctl.c---you will have to follow the pointer there to answer the
question, too. In some cases, you may want to send a message to the Tux
controller in response to a message just received by the computer (using
tuxctl_ldisc_put). However, if the output buffer for the device is full,
you cannot do so immediately. Nor can the code (executing in
tuxctl_handle_packet) wait (for example, go to sleep).  Explain in one
sentence why the code cannot wait.


a.

MTCP_BIOC_ON is sent when the computer wants to enable interrupts from the Mouse/Tux controller buttons.
The effect is that the Tux controller can send interrupt signals when the button is pressed. The result is that 
the MTCP_ACK signal is returned. MTCP_ACK is simply the signal that is returned when the Mouse/Tux controller successfully completes a command.

MTCP_LED_SET is sent when the LED is in USR mode and the user wants to set the LED values.
The first byte of this argument determines which of the LEDs to set, and consequently how many bytes to set for each LED.
The number of bytes that follow equal the number of bits set to 1 in the first byte. These bytes correspond to particular LEDs, and the
bits of each byte (which corresponds to an LED) correspond to the seven segments of its respective LED's seven-segment display.

b.

MTCP_ACK is the response returned when the Mouse/Tux controller successfully completes a command. The information 
conveyed by the message is that the Mouse/Tux controller successfully completed a command that the computer sent it.

MTCP_BIOC_EVENT is the response generated when interrupts are enabled for the buttons for the Mouse/Tux controller and a button is
pressed/released. This response conveys the information that a button has been pressed or released.

MTCP_RESET is the response generated when the device is powered-up and re-initialized, or if the reset button is pressed, or
when an MTCP_RESET_DEV command is sent from the computer to the Mouse/Tux controller. An MTCP_RESET_DEV command can be sent by the
computer to the Mouse/Tux controller, and this causes the device to reset. The device doesn't respond until it finishes initializing 
itself, at which point it sends the MTCP_RESET signal. This message conveys that some sort of reset was applied to the Mouse/Tux
controller.

c.
The code cannot wait because the function is called by an interrupt, so it can't take too much time, otherwise the program will 
take too long.