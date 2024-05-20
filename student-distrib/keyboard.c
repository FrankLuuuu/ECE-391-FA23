#include "keyboard.h"

uint8_t shift_count = 0;
uint8_t control_count = 0;
uint8_t caps_count = 0;
uint8_t special_char_found = 0;
uint8_t alt_count = 0; 

uint8_t term_1 = 0;
uint8_t term_2 = 0;

//TODO: please review scancodes to check shift was done right. also need to add stuff for
//special keys (ALT, Ctrl, Shift)
//Frank: checked scancodes and added second column for all scancodes, we add things for special keys in handler

unsigned char scancodes[KEYBOARD_SCANCODES][2] = {{0x00, 0x00}, {0x00, 0x00}, {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'}, 
                                                 {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'}, {'-', '_'}, {'=', '+'}, {BACKSPACE, BACKSPACE}, {' ', ' '}, 
                                                 {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'}, {'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, {'i', 'I'}, 
                                                 {'o', 'O'}, {'p', 'P'}, {'[', '{'}, {']', '}'}, {'\n', '\n'}, {0x00, 0x00}, {'a', 'A'}, {'s', 'S'}, 
                                                 {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'}, {'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'}, 
                                                 {'\'', '"'}, {'`', '~'}, {0x00, 0x00}, {'\\', '|'}, {'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, 
                                                 {'b', 'B'}, {'n','N'}, {'m','M'}, {',','<'}, {'.', '>'}, {'/', '?'}, {0x00, 0x00}, {0x00, 0x00}, 
                                                 {0x00, 0x00}, {' ', ' '}};

/* init_keyboard
 * 
 * Initializes the keyboard devide to allow interrupts
 * Inputs: None
 * Outputs: None
 * Side Effects: eneables irq for keyboard
 */
void init_keyboard() {
    enable_irq(0x01);
}

/* keyboard_irq_handler
 * 
 * Handles any keyboard interrupts
 * Inputs: None
 * Outputs: None
 * Side Effects: prints the correct char and sends eoi
 */
void keyboard_irq_handler() 
{
    cli();

    send_eoi(0x01);

    // get the input key amd check if valid
    uint32_t input = inb(KEYBOARD_PORT);

    //check if input was a special character
    //if it is, change special_char_found flag to 1, register which special char we changed
    switch(input)
    {
        case CAPS_PRESSED:
            if(caps_count)
                caps_count = 0;
            else
                caps_count = 1;
            special_char_found = 1;
            break;
        case LSHIFT_PRESSED:
            shift_count = 1;
            special_char_found = 1;
            break;
        case LSHIFT_RELEASED:
            shift_count=0;
            special_char_found = 1;
            break;
        case RSHIFT_PRESSED:
            shift_count=1;
            special_char_found = 1;
            break;
        case RSHIFT_RELEASED:
            shift_count=0;
            special_char_found = 1;
            break;
        case CTRL_PRESSED:
            control_count = 1;
            special_char_found = 1;
            break;
        case CTRL_RELEASED:
            control_count = 0;
            special_char_found = 1;
            break;
        case ALT_PRESSED:
            alt_count = 1;
            special_char_found = 1;
            break;
        case ALT_RELEASE:
            alt_count = 0;
            special_char_found = 1;
            break;
        default:
            special_char_found = 0;
    }


    if(special_char_found)
    {
        special_char_found = 0;
        sti();
        return;
    }

    if(input < 0 || input >= KEYBOARD_SCANCODES)
    {
        sti();
        return;
    }

    // put the corresponding char onto the screen
    uint8_t result = scancodes[input][0];
    uint8_t shift_result = scancodes[input][1];

    //if control l is pressed, clear screen
    if(control_count)
    {
        if(result == 'l')
        {
            terminals[curr_term].numBackspaces = 0;
            clear();
        }
        else if(result == 'k')
        {
            terminal_switch(1);
            if(!term_1) {
                term_1 = 1;
                execute((uint8_t*) "shell");
            }
        }
        else if(result == 'j')
        {
            terminal_switch(2);
            if(!term_2) {
                term_2 = 1;
                execute((uint8_t*) "shell");
            }
        }
        else if(result == 'h')
        {
            terminal_switch(0);
        }
        else if(result == 'c')
        {
            terminals[curr_term].numBackspaces = 0;
            halt(1);
        }
    }
    else if(alt_count)
    {
        if(input == F2)
        {
            terminal_switch(1);
            if(!term_1) {
                term_1 = 1;
                execute((uint8_t*) "shell");
            }
        }
        else if(input == F3)
        {
            terminal_switch(2);
            if(!term_2) {
                term_2 = 1;
                execute((uint8_t*) "shell");
            }
        }
        else if(input == F1)
        {
            terminal_switch(0);
        }
    }

    //if user presses enter, go to next line and reset variables
    else if(result == ENTER)
    {
        terminals[curr_term].numBackspaces = 0;
        putc(result);
        register_typedChar(result);
        sti();
        return;
    }

    //if user presses backspace, check if you can delete character and delete char
    else if(result == BACKSPACE && terminals[curr_term].characterCounter != 0)
    {
        if(terminals[curr_term].numBackspaces > 0)
        {
            terminals[curr_term].numBackspaces-=1;
            putc(result);
            register_typedChar(result);
        }
    }

    //as long as there's space for a character, see how to process it
    else if(terminals[curr_term].numBackspaces < BUFFER_SIZE)
    {
        //if shift is being pressed
        if(shift_count)
        {
            if(caps_count)
            {
                //if caps is pressed, letters should be lowercase
                if(result >= 'a' && result <= 'z')
                {
                    terminals[curr_term].numBackspaces+=1;
                    putc(result);
                    register_typedChar(result);
                }
                //other characters should be shift + character
                else
                {
                    terminals[curr_term].numBackspaces+=1;
                    putc(shift_result);
                    register_typedChar(shift_result);
                }
            }
            //if no caps, just print the shifted character
            else
            {
                terminals[curr_term].numBackspaces+=1;
                putc(shift_result);
                register_typedChar(shift_result);
            }
        }

        //if no shift, then check if caps or no caps
        else if(shift_count == 0)
        {
            //if caps and no shift, print uppercase alphabet and normal other symbols
            if(caps_count)
            {
                if(result >= 'a' && result <= 'z')
                {
                    terminals[curr_term].numBackspaces+=1;
                    putc(shift_result);
                    register_typedChar(shift_result);
                }
                else
                {
                    terminals[curr_term].numBackspaces+=1;
                    putc(result);
                    register_typedChar(result);
                }
            }
            //if no caps and no shift, print unshifted letter or symbol
            else
            {
                terminals[curr_term].numBackspaces+=1;
                putc(result);
                register_typedChar(result);
            }
        }
    }
    
    sti();
}

