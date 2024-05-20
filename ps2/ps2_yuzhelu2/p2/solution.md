### P2 Solution

a.  **MTCP_BIOC_ON**: It should be sent when we want to enable button interrupt-on-change. It allows the Tux controller to send interrupts when the button is pressed. MTCP_ACK is returned as a result.  
    **MTCP_LED_SET**: It should be sent when we want to set the User-set LED display values. It displays these values on the LED displays when the LED display is in USR mode. MTCP_ACK is returned as a result.  

b.  **MTCP_ACK**: It is sent when the MTC successfully completes a command.  
    **MTCP_BIOC_EVENT**: It is sent when the button interrupt-on-change is enabled and a button is either pressed or released.  
    **MTCP_RESET**: It is sent when the device re-initializes itself after a power-up, a RESET button press, or an MTCP_RESET_DEV command.

c.  Since it is called by an interrupt, it cannot wait or sleep since it would take too much time and might break the code.
