### P1 Solution

a. The Line Compare field in the VGA, of which bit 9 is in the Maximum Scan Line Register, bit 8 is in the 
Overflow Register, and bits 7-0 are in the Line Compare Register, specifies the scan line address of the horizontal division. 
When the line counter reaches the value in the Line Compare Register, the current scan line start address is reset to 0. 
If the Pixel Panning Mode field is set to 1 then the Pixel Shift Count and Byte Panning fields are reset to 0 for the 
remainder of the display cycle allowing the top window to pan while the bottom window remains fixed. Otherwise, both windows 
pan by the same amount.

b. To write a palette entry, output the palette entry's index value to the DAC Address Write Mode Register then 
perform 3 writes to the DAC Data Register, loading the red, green, then blue values into the palette RAM. 
The internal write address automatically advances allowing the next value's RGB values to be loaded without having 
to reprogram the DAC Address Write Mode Register.  This allows the entire palette to be loaded in one write operation.