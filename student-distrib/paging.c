#include "paging.h"

extern void enable_paging(int directory);

void paging_init(){
  //Loop variables
  unsigned int j;
  unsigned int i;

  //Setup the first 4MB of memory with 4 kB tables
  page_dir[0].present       = 1;
  page_dir[0].read_write    = 1;
  page_dir[0].user          = 0;
  page_dir[0].write_through = 0;  
  page_dir[0].cache_disable = 0;  
  page_dir[0].accessed      = 0;
  page_dir[0].size          = 0;  
  page_dir[0].reserved      = 0;

  //Aligned value for the table address
  page_dir[0].addr = ((int)page_table)/ALIGN_CONS;

  //Sets up the rest as 4MB pages
  for(i = 1; i < MAX_ENTRIES; ++i){
    if(i == 1){

      //Bit setup for kernel memory.
      page_dir[i].present     = 1;
      page_dir[i].user        = 0;
      page_dir[i].addr = KADDR/ALIGN_CONS;
    }
    else if(i == USER_I){

      //set present and set user for user memory space
      page_dir[i].present = 1;
      page_dir[i].user    = 1;
      page_dir[i].addr = USER_MEM/ALIGN_CONS;
    }
    else{
      //set not present and not user for everything else
      page_dir[i].present     = 0;
      page_dir[i].user        = 0;
    }
    page_dir[i].read_write    = 1;
    page_dir[i].write_through = 0;  
    page_dir[i].cache_disable = 0;  
    page_dir[i].accessed      = 0;
    page_dir[i].size          = 1;  
    page_dir[i].reserved      = 0;
  }


  //Fill in the page table for the first 4MB
  for(j = 0; j < MAX_ENTRIES; ++j){ // align_cons = 4
    //Setup video memory page
    if(j * ALIGN_CONS == VADDR || j * ALIGN_CONS  == T1ADDR || j * ALIGN_CONS  == T2ADDR || j * ALIGN_CONS  == T3ADDR){
      page_table[j].present     = 1;
      page_table[j].user        = 1;
    }
    //Set up for unused 4kB pages
    else{
      page_table[j].present     = 0;
      page_table[j].user        = 0;
    }
    page_table[j].read_write    = 1;
    page_table[j].write_through = 0;    //Not sure
    page_table[j].cache_disable = 0;    //Not sure
    page_table[j].accessed      = 0;
    page_table[j].dirty         = 0;
    page_table[j].reserved      = 0;
    page_table[j].global        = 0;    //Not sure
    page_table[j].addr = j;
  }

// for(j = 0; j < MAX_ENTRIES; ++j){
    //Setup video memory page
    page_table_vidmap[0].present     = 0;
    page_table_vidmap[0].read_write    = 1;
    page_table_vidmap[0].user          = 0;
    page_table_vidmap[0].write_through = 0;    //Not sure
    page_table_vidmap[0].cache_disable = 1;    //Not sure
    page_table_vidmap[0].accessed      = 0;
    page_table_vidmap[0].dirty         = 0;
    page_table_vidmap[0].reserved      = 0;
    page_table_vidmap[0].global        = 0;    //Not sure
    page_table_vidmap[0].addr = (uint32_t)(VADDR / ALIGN_CONS);
  // }

  //Sets up the control registers for paging
  enable_paging((int)page_dir);
}

