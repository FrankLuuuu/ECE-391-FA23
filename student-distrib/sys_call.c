#include "sys_call.h"
#include "rtc.h"
#include "paging.h"
#include "terminal.h"
#include "x86_desc.h"


//variables for keeping track of the pid values

// paging_helper.S
extern void TLB_flushing();

// cur_pid = 0;
// parentPid = 0;
int pidArray[eight];


/*
* int32_t halt(uint8_t status)
* Input: status of the process
* Output: returns result value based on
* Function: Terminates process, returning to parent process
*/
int32_t halt(uint8_t status) {
    int i;
    // pcb_t* pcb = pcb_getpcb(terminals[curr_term].pid);
    pcb_t* pcb = pcb_getactpcb();

    // restart shell if the halting program is the last one
    //  || terminals[curr_term].ppid == -1
    if(!pcb->pid) {
        // cur_pid = -1; 
        // ppid = -1;

        // execute((uint8_t*) "shell");
        // return 0;
        uint32_t eip = pcb->eip;
        uint32_t esp = pcb->esp;

        // context switch with User DS, ESP, EFLAG, CS, EIP
        asm volatile (              "\
            pushl   %%ebx           ;\
            movw    %%bx, %%ds      ;\
            pushl   %%edx           ;\
            pushfl                  ;\
            pushl   %%ecx           ;\
            pushl   %%eax           ;\
            iret                    ;"
            :
            : "b"(USER_DS), "d"(esp), "c"(USER_CS), "a"(eip)
            : "memory"
        );

        return 0;
    }

    // reset pcb fd
    pidArray[pcb->pid] = 0;
    for(i = 0; i < eight; i++) 
        close(i);

    // restore paging
    cur_pid = pcb->ppid;
    // terminals[curr_term].pid = cur_pid;
    pcb_t* parent = pcb_getpcb(cur_pid);
    parentPid = parent->ppid;
    // terminals[curr_term].ppid = parentPid;
    
    page_dir[eight * four].addr = (mem_offset + cur_pid) * fourmb / fourkb; // 4 MB page size & 4KB
    TLB_flushing();

    // store info in tss 
    tss.esp0 = eightmb - (cur_pid * eightkb) - four; // 8MB & 8KB
    tss.ss0 = KERNEL_DS;

    // context switch
    asm volatile ("             \n\
            movl %%ebx, %%eax   \n\
            movl %%edx, %%ebp   \n\
            movl %%ecx, %%esp   \n\
            leave               \n\
            ret                 \n"
            :
            : "b" (status), "d" (pcb->ebp0), "c" (pcb->esp0)
            : "eax", "ebp", "esp");

    return 0;
}

/* int32_t execute(const uint8_t* command)
 * Inputs      : command made up of file name and argument
 * Return Value: -1 if any problem with file to be executed, 0 if no problem
 * Function    :  executes a new program and takes care of context switching
 */
int32_t execute(const uint8_t* command)
{    
    //buffers for file name and argument 
    uint8_t file_to_read[FILENAME_LEN];
    uint8_t file_argument[FILENAME_LEN];
    
    //file length and argument length variables
    int fileLength = 0;
    int argumentLength = 0;

    int i;
    //initialize the buffers
    for (i = 0; i < FILENAME_LEN;i++)
    {
      file_to_read[i] = '\0';
      file_argument[i] = '\0';
    }

    //go through command and parse the file name
    int commandCharacters = 0;  
    int numSpaces = 0; 
    for(i = 0; i < strlen((const int8_t*)command); i++)
    {
        //if there is a space in command, check if it's leading space -- if it isn't we're done reading the file name
        if(command[i] == ' ')
        {
            numSpaces++;
            if(fileLength > 0)
                break;
        }
        else
        {
            //add character to buffer if it isn't a space
            file_to_read[commandCharacters] = command[i];
            fileLength++;
            commandCharacters++;
        }
    }

    i = 0;
    //go through command and parse the arguments in command
    int argStart = commandCharacters+numSpaces;
    int argCharacters = 0;
    for(i = argStart; i< strlen((const int8_t*)command); i++)
    {
        //if there is a space in command, check if it's leading space -- if it isn't we're done reading the argument
        if(command[i] == ' ')
        {
             if(argumentLength > 0)
                break;
        }
        else
        {
            //add character to buffer if it isn't a space
           file_argument[argCharacters] = command[i];
           fileLength++;
           argCharacters++;
        }
    }

    dentry_t dentry;
    uint8_t elf_buf[four];

    //check if file exists -- return -1 if it doesn't
    if(read_dentry_by_name(file_to_read, &dentry)==-1)
        return -1; 

    //read 4 bytes of data from file and store in elf_buf -- if it fails, return -1
    if(fs_read_data(dentry.inode_num, 0, elf_buf, four) == -1)
        return -1; 
    

    //check if file is a valid executable file -- if not, return -1
    uint8_t space = 127;
    uint8_t e = 69;
    uint8_t l = 76;
    uint8_t f = 70;

    if(elf_buf[0] != space || elf_buf[1] != e || elf_buf[2] != l || elf_buf[3] != f) {
        return -1; 
    }

    //create a new pcb for the new process
    pcb_t* new_pcb;
    //flag to see if pid has been found or not
    int pid_found = 0;
    for(i = 0; i < max_shells;i++)
    {        
        //if a pid is not active, we can use it
        if(pidArray[i] == 0)
        {
            //set the pid in the pid array to 1, set pid_found to 1, set cur_pid to i
            pidArray[i] = 1;
            cur_pid = i;                 
            pid_found = 1;    
            break;
        }
    }

    //if pid has not been found, return -1
    if(pid_found == 0)
    {
        printf("pid full");
        return -1;
    }

    terminals[curr_term].pid = cur_pid;

   
    page_dir[USER_I].addr = ((mem_offset + cur_pid) * fourmb) / fourkb;

    //flush the tlb for program switch
    TLB_flushing();


    inode_t* temp_inode_ptr = (inode_t *)(inode_start + dentry.inode_num);
    uint8_t* image_addr = (uint8_t*)prog_img_virt_addr;         
    fs_read_data(dentry.inode_num, (uint32_t)0, image_addr, temp_inode_ptr->length); 

    new_pcb = pcb_getpcb(cur_pid);  

    //set cur_pid and parent pid in the pcb
    new_pcb->pid = cur_pid;
    new_pcb->ppid = parentPid;
    parentPid = cur_pid;
    //store argument into the command argument variable in pcb
    strncpy((int8_t*)new_pcb->cmd_arg, (int8_t*)(file_argument), FILENAME_LEN);

    //initialize the file ops for the fd array as null for all
    for (i = 0; i < eight; i++) 
    {
        new_pcb->fd_table[i].file_op = &null;
        new_pcb->fd_table[i].inode = 0;
        new_pcb->fd_table[i].file_pos = 0;
        new_pcb->fd_table[i].flag = 1;
    }

    //first and second file_ops should be stdin and stdout for reading from and writing to terminal
    new_pcb->fd_table[0].file_op = &stdin;        
    new_pcb->fd_table[0].inode = 0;
    new_pcb->fd_table[0].file_pos = 0;
    new_pcb->fd_table[0].flag = 0;

    new_pcb->fd_table[1].file_op = &stdout;       
    new_pcb->fd_table[1].inode = 0;
    new_pcb->fd_table[1].file_pos = 0;
    new_pcb->fd_table[1].flag = 0;

    uint32_t eip;
    uint32_t esp3;

    //get eip from file and store eip in pcb
    uint8_t eip_buf[four];
    fs_read_data(dentry.inode_num, 24, eip_buf, four); 
    eip = *((int*)eip_buf);
    new_pcb->eip = eip;

    //calculate esp and store esp in pcb
    esp3 = USER_MEM + fourmb - four; 
    new_pcb->esp = esp3;

    //set the kernel stack segment and stack pointer, and store kernel stack pointer in pcb
    tss.ss0 = KERNEL_DS;
    tss.esp0 = eightmb - (eightkb * cur_pid) - four;
    new_pcb->esp0_tss = tss.esp0;

    //copy esp0 and ebp into the corresponding variables
    uint32_t esp0;
    uint32_t ebp;
    asm("\t movl %%esp, %0" : "=r"(esp0));
    asm("\t movl %%ebp, %0" : "=r"(ebp));
    new_pcb->esp0 = esp0;
    new_pcb->ebp0 = ebp;

    //enable interrupts before iret
    sti();

    // context switch with User DS, ESP, EFLAG, CS, EIP
    asm volatile (              "\
        pushl   %%ebx           ;\
        movw    %%bx, %%ds      ;\
        pushl   %%edx           ;\
        pushfl                  ;\
        pushl   %%ecx           ;\
        pushl   %%eax           ;\
        iret                    ;"
        :
        : "a"(eip), "b"(USER_DS), "c"(USER_CS), "d"(esp3)
        : "memory"
    );
    return 0;
}


/*
* int32_t open (const uint8_t* filename)
* Input: filename 
* Output: number of bytes written, if fail return -1
* Function: Writes data to the terminal or a device
*/
int32_t open(const uint8_t* filename) {
    int i;
    dentry_t dentry;

    // check if file name is valid
    if(!strlen((char*)filename) || read_dentry_by_name(filename, &dentry) == -1)
        return -1;  

    pcb_t* pcb = pcb_getactpcb();
    // check the next available fd
    for(i = 2; i < eight; i++) 
        if(pcb->fd_table[i].flag) 
            break;

    // no fd is available
    if(i == eight)
        return -1;

    // initialize fd entry
    pcb->fd_table[i].inode = dentry.inode_num;
    pcb->fd_table[i].file_pos = 0;
    pcb->fd_table[i].flag = 0;

    if(dentry.filetype == 0) // rtc           
        pcb->fd_table[i].file_op = &rtc;
    else if(dentry.filetype == 1) // dir    
        pcb->fd_table[i].file_op = &dir;
    else if(dentry.filetype == 2) // file 
        pcb->fd_table[i].file_op = &file;
    else { // invalid
        pcb->fd_table[i].flag = 1;
        return -1;
    }

    pcb->fd_table[i].file_op->open(filename);

    return i; 
}

/*
* int32_t close (int32_t fd)
* Input: fd: file descriptor
* Output: 0 if success, if fail return -1
* Function: Closes the specified file descriptor and makes it available for return from later calls to open
*/
int32_t close(int32_t fd) {
    // check if the file is opened and fd is valid
    pcb_t* pcb = pcb_getactpcb();
    if(fd < 2 || fd > eight || pcb->fd_table[fd].flag)
        return -1;

    // reset the fd
    pcb->fd_table[fd].inode = 0;
    pcb->fd_table[fd].file_pos= 0;
    pcb->fd_table[fd].flag = 1;
    
    return pcb->fd_table[fd].file_op->close(fd);
}


/*
* int32_t read (int32_t fd, void* buf, int32_t nbytes){
* Input: fd: file descriptor
*        buf: buffer used to write to terminal
*        nbytes: number of bytes to write
* Output: number of bytes read, if fail return -1
* Function: Reads data from the keyboard, a file, device, or directory 
*/
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    sti();
    
    pcb_t* pcb = pcb_getactpcb();
    if(fd < 0 || fd > eight || buf == NULL || pcb->fd_table[fd].flag) 
        return -1;

    return pcb->fd_table[fd].file_op->read(fd, buf, nbytes);
}


/* int32_t write(int32_t fd, void* buf, int32_t nbytes)
 * Inputs      : fd     - file descriptor to write data
 *               buf    - buffer that contains data to write
 *               nbytes - number of bytes to write into
* Function: writes data to the terminal or to a device  (not required)*/
int32_t write(int32_t fd, void* buf, int32_t nbytes) {
    pcb_t* pcb = pcb_getactpcb();
    if(fd < 0 || fd > eight || buf == NULL || pcb->fd_table[fd].flag) 
        return -1;

    return pcb->fd_table[fd].file_op->write(fd, buf, nbytes);
}


/*
* int32_t getargs (uint8_t* buf, int32_t nbytes){
* Input: buf: buffer used to write to terminal
*             nbytes: number of bytes to write
* Output: 0 if success, if fail return -1, otherwise argument is copied into buf
* Function: reads the program command line arguments into a user-level buffer
*/

int32_t getargs(uint8_t* buf, int32_t nbytes) {
    if(buf == NULL){
        return -1;
    }
    pcb_t* pcb = pcb_getactpcb();
    if(pcb->cmd_arg[0] == NULL){
        return -1;
    }

    int len=strlen((int8_t*)pcb->cmd_arg);
    memcpy(buf, pcb->cmd_arg,len+1);

    return 0;
}


/* int32_t vidmap(uint32_t** screen_start)
 * Inputs      : buf    - double pointer to user address space 
* Function: pass virtual address which points to video memory */
int32_t vidmap(uint32_t** screen_start) {
    if (screen_start == NULL)       //check valid in-ut -is this of cur pid
        return -1;
        
    if (((uint32_t)screen_start) < USER_MEM || ((uint32_t)screen_start) > (USER_MEM + fourmb))          //check whether the address falls within the address range covered by the single user-level page
        return -1;

    //page set up is doen in pagineing init, just need to turn on though
    page_dir[VIDEO_INDEX].size = 0;
    page_dir[VIDEO_INDEX].present = 1;
    page_dir[VIDEO_INDEX].user = 1;
    page_dir[VIDEO_INDEX].addr = (uint32_t)page_table_vidmap / fourkb;        //pabe table for vid ma[?]

    page_table_vidmap[0].present = 1;
    page_table_vidmap[0].user = 1;
    page_table_vidmap[0].addr = VADDR / fourkb;   //what is j supposed to be

    // pcb_init(cur_pid);       //check name/numbefr       pabe is to hold allt he memory mappingsis this needed if it isnt really a process thouh?
    //cet all off pcb to whatever it is supposed to be
    TLB_flushing();

    *screen_start = (uint32_t*)VM_VIDEO;    // what is the address
    return 0;           //return same address always, should be written into the memory location provided by the caller (whi

}


int32_t set_handler(int32_t signum, void* handler_address) {
    return -1;
}
int32_t sigreturn(void) {
    return -1;
}


/* pcb_t* pcb_getactpcb(){
 * Function: get addres to current pcb */
pcb_t* pcb_getactpcb() {
    return (pcb_t*)(eightmb - eightkb * (cur_pid + 1));
}


/* pcb_t* pcb_getpcb(){
 * Function: get addres to pcb corresponding to input pid */
pcb_t* pcb_getpcb(uint32_t pid) {
    return (pcb_t*)(eightmb - eightkb * (pid + 1));
}


/* void init_file_ops()
 * Inputs      : none
 * Return Value: none
 * Function    :  initializes fop table   */
void init_file_ops(){
    rtc.read = rtc_read;
    rtc.write = rtc_write;
    rtc.open = rtc_open;
    rtc.close = rtc_close;

    dir.read = fs_dir_read;
    dir.write = fs_dir_write;
    dir.open = fs_dir_open;
    dir.close = fs_dir_close;

    file.read = fs_file_read;
    file.write = fs_file_write;
    file.open = fs_file_open;
    file.close = fs_file_close;

    stdin.read = term_read;
    stdin.write = null_write;
    stdin.open = term_open;
    stdin.close = term_close;

    stdout.read = null_read;
    stdout.write = term_write;
    stdout.open = term_open;
    stdout.close = term_close;

    null.read = null_read;
    null.write = null_write;
    null.open = null_open;
    null.close = null_close;
}


int32_t null_read(int32_t fd, void* buf, int32_t nbytes) {return -1;}
int32_t null_write(int32_t fd, const void* buf, int32_t nbytes) {return -1;}
int32_t null_open(const uint8_t* filename) {return -1;}
int32_t null_close(int32_t fd) {return -1;}


