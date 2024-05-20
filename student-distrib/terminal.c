#include "terminal.h"

// char buffer[BUFFER_SIZE];
// volatile int32_t characterCounter;
// volatile int32_t start_read;

char enter = '\n';

extern void TLB_flushing();

void term_init()
{
    int i;

    for(i = 0; i < MAX_TERM; i++)
    {
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
        terminals[i].characterCounter = 0;
        terminals[i].start_read = 0;
        terminals[i].prog_count = 0;
        terminals[i].pid = -1;

        terminals[i].ebp = 0;
        terminals[i].esp = 0;
        terminals[i].numBackspaces = 0;
        terminals[i].ppid = -1;
    }

    terminals[0].vid_mem_addr = T1ADDR;
    terminals[1].vid_mem_addr = T2ADDR;
    terminals[2].vid_mem_addr = T3ADDR;

    sc_term = 0;
    curr_term = 0;
}
/* term_open
 * Inputs: filename
 * Return Value: 0
 * Function: open terminal with given filename
 */
int32_t term_open(const uint8_t* filename)
{
    return 0;   
}
/* term_close
 * Inputs: filename
 * Return Value: 0
 * Function: close terminal with given filename
 */
int32_t term_close(int32_t filename)
{
    return 0;
}
/* term_read
 * Inputs: fd: file descriptor
           buf: pointer to buffer where read_bytes will be stored
           nbytes:number of bytes to be read
 * Return Value: int32_t value of bytes read
 * Function: reads characters from buffer into a local buffer stopping at a newline or byte limit, clears
 * read portion of buffer and then returns the total number of bytes read
 */
int32_t term_read(int32_t fd, void* buf, int32_t nbytes)
{
    int bytes_read = 0;
    int i;

    while(terminals[curr_term].start_read == 0); //waits for start_read to become 0

    cli(); //disables interrupts

    if(nbytes >= BUFFER_SIZE) // TODO: if 10 letter change condition to >
    {
        for(i = 0;i < BUFFER_SIZE; i++) //reads up to the size of the buffer into buf until newline is detected
        {
            ((char *) buf)[i] = terminals[curr_term].buffer[i];
                                                     
            if(terminals[curr_term].buffer[i] == '\n')
            {
                bytes_read = i+1; //increment bytes read
                break;
            }
        }
        for(i = 0; i < BUFFER_SIZE; i++)
            terminals[curr_term].buffer[i]= ' ';

        sti(); //enables interrupts
        terminals[curr_term].characterCounter = 0;
        terminals[curr_term].start_read = 0;
        return bytes_read;  //returns bytes read
    }

    int lastPos = nbytes - 1; // TODO: if 10 letter remove -1
    for(i = 0;i < nbytes; i++) // TODO: if 10 letter change condition to <=
    {
        ((char *) buf)[i] = terminals[curr_term].buffer[i];
        bytes_read = i + 1;
            
        if(terminals[curr_term].buffer[i] == '\n') //encounters new line
            break;
        else if(i == lastPos) 
        {
            ((char *) buf)[lastPos] = '\n'; //assignes newline character to lastPos 
            break;
        }
    }
    
    
    for(i = 0; i < nbytes;i++)
        terminals[curr_term].buffer[i] = ' ';

    //resets the count and start_read var
    terminals[curr_term].characterCounter = 0; 
    terminals[curr_term].start_read = 0;
    sti();
    return bytes_read; 
}
/* term_write
 * Inputs: fd: file descriptor
           buf: pointer to buffer where read_bytes will be stored
           nbytes:number of bytes to be written
 * Return Value: int32_t value of bytes to write
 * Function: writes characters from buf to output skipping null characters and returns the number 
 * of bytes to write
 */
int32_t term_write(int32_t fd, const void* buf, int32_t nbytes)
{
    int i;
    cli(); //disable interrrupts
    for(i = 0; i < nbytes; i++)
    {
        if(((char *)buf)[i] != '\0') //if character isnt null, its written to output
            putc(((char *)buf)[i]);
    }
    
    // for(i = 0; i < nbytes;i++)
    //     terminals[curr_term].buffer[i] = ' ';

    // //resets the count and start_read var
    // terminals[curr_term].characterCounter = 0; 
    // terminals[curr_term].start_read = 0;
    sti(); //enable interrupts
    return nbytes;
}
/* register_typedChar
 * Inputs: character that was typed
 * Return Value: none
 * Function: processes typed characters and updates buffer based on the type of the character
 */
void register_typedChar(char typed)
{
    char enter = '\n';
    if(typed == BACKSPACE)
    {
        if(terminals[curr_term].characterCounter > 0 && terminals[curr_term].characterCounter <= BUFFER_SIZE) 
        {
            int prevPosition = terminals[curr_term].characterCounter - 1; //most recent character is erased
            terminals[curr_term].buffer[prevPosition] = ' ';
            terminals[curr_term].characterCounter -= 1; //decremented
        }
    }
    else if(typed == enter) //if character is a newline
    {
        if(terminals[curr_term].characterCounter < BUFFER_SIZE)
            terminals[curr_term].buffer[terminals[curr_term].characterCounter] = typed;    //newline is added to current position in buffer
        else
            terminals[curr_term].buffer[BUFFER_SIZE-1] = typed; //newline replaces last character in buffer

        terminals[curr_term].characterCounter = 0;
        terminals[curr_term].start_read = 1;
    }
    else if(terminals[curr_term].characterCounter < BUFFER_SIZE - 1)
    {
        terminals[curr_term].buffer[terminals[curr_term].characterCounter] = typed; //added to current position
        terminals[curr_term].characterCounter += 1;
    }
}
/* terminal_switch
 * Inputs: new terminal
 * Return Value: none
 * Function: Switching between terminals and calls to corresponding functions
 */
void terminal_switch(int new_term) {
    if(new_term == curr_term || new_term < 0 || new_term > 2)
        return;

    update_vid_mem_paging(curr_term);

    // copy memory contents
    memcpy((void*)terminals[curr_term].vid_mem_addr, (const void*)VADDR, FOURKB);
    memcpy((void*)VADDR, (const void*)terminals[new_term].vid_mem_addr, FOURKB);

    // update new term vars
    curr_term = new_term;
    update_term_cursor(terminals[new_term].screen_x, terminals[new_term].screen_y);

    update_vid_mem_paging(curr_term);
}
/* update_vid_mem_paging
 * Inputs: terminal
 * Return Value: none
 * Function: Establishes correct page table entry for video memory by terminal number 
 */
void update_vid_mem_paging(int term) {
    if(curr_term == term) { // map to actual vid mem
        page_table_vidmap[0].present = 1;
        page_table_vidmap[0].read_write = 1;
        page_table_vidmap[0].user = 1;
        page_table_vidmap[0].addr = (uint32_t)(VADDR / FOURKB);
    }
    else { // map to background page
        page_table_vidmap[0].present = 1;
        page_table_vidmap[0].read_write = 1;
        page_table_vidmap[0].user = 1;
        // page_table_vidmap[0].addr = (uint32_t)(VADDR / FOURKB + (term + 1));
        page_table_vidmap[0].addr = (uint32_t)(T1ADDR / FOURKB + term);
    }

    TLB_flushing();
}
