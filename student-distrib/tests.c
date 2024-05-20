#include "tests.h"
#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "file_sys.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

int testFlag = 0;


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here

/* exception test
 * 
 * Check that exceptions can be triggered
 * Inputs: None
 * Outputs: None
 * Side Effects: Should print the correct exception
 * Coverage: init idt, exception divide by zero
 * Files: idt.c, x86_desc.S
 */
int exc_div_zero_error_test() {
	TEST_HEADER;
	int a = 4;
	int b = 0;
	int c = a / b;
	return c;
}

/* Checkpoint 2 tests */

/* read_by_index test
 * 
 * Check if we can read a dentry by index 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Print file name
 * Coverage: init file, read dentry by index
 * Files: file_sys.h/c
 */
int test_read_by_index() {
	TEST_HEADER;

	clear();

	int i;
	int result = PASS;
	dentry_t dentry;
	int index = 11; // very large long name file

	if (read_dentry_by_index(index, &dentry) == -1) { // calls by index 
		assertion_failure();
		result = FAIL;
	}

	printf("\n");
	for (i = 0 ; i < FILENAME_LEN; i++) {                             
		printf("%c", dentry.filename[i]); // prints the filename
	}
	printf("\n");

	return result;
}

/* read_by_name test
 * 
 * Check if we can read a dentry by name 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Print out the file name
 * Coverage: init file, read dentry by index
 * Files: file_sys.h/c
 */
int test_read_by_name() {
	TEST_HEADER;
	
	clear();

	int i;
	int result = PASS;
	dentry_t dentry;

	if (read_dentry_by_name((uint8_t*)"frame1.txt", &dentry) == -1) { // calls by name 
		assertion_failure();
		result = FAIL;
	}
	
	printf("\n");
	for (i = 0 ; i < FILENAME_LEN; i++) {                            
		printf("%c", dentry.filename[i]); // prints the filename
	}
	printf("\n");

	return result;
}

/* dir_read test
 * 
 * Check if we can read every file inside the directory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: print the filenames and corresponding file size
 * Coverage: init file, dir read
 * Files: filesys.h/c
 */
int test_dir_read() {
	TEST_HEADER;

	clear();

	int result = PASS;
	int bytes;
	int i;
	uint8_t character[33]; // size of the file name                                     
	character[32] = '\0';
	
	printf("\n");
	for(i = 0; i < dentry_num; i++) {
		bytes = fs_dir_read(i, &character, 32); // size of the file name + NULL
		if (bytes == -1) {
			result = FAIL;
			break;
		}
		printf("File name: %s, File size: %d\n", character, bytes);
	}

	return result;
}

/* small file_read test
 * 
 * Check if we can read the content of a small file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Display the file content
 * Coverage: init file, file read
 * Files: filesys.h/c
 */
// int test_small_file_read() {
// 	TEST_HEADER;

// 	clear();
// 	int result = PASS;
// 	int bytes;
// 	// int i;
// 	uint8_t character[200];                                     
// 	uint8_t filename[32] = "frame0.txt";

// 	// read the file
// 	bytes = fs_file_read(filename, &character, 200);                
// 	if (bytes == -1) {
// 		result = FAIL;
// 		return result;
// 	}

// 	// print the content buffer
// 	puts((int8_t*)character);

// 	// for(i = 0; i < bytes; i++) {
// 	// 	printf("%c", character[i]);
// 	// }

// 	printf("\n");
// 	printf("file name: %s\n", filename);
// 	return result;
// }

/* large file_read test
 * 
 * Check if we can read the content of a large file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Display the file content
 * Coverage: init file, file read
 * Files: filesys.h/c
 */
// int test_large_file_read() {
// 	TEST_HEADER;

// 	clear();

// 	testFlag = 1;
// 	int result = PASS;
// 	int bytes;
// 	// int i;
// 	uint8_t character[40000]; // max txt file legnth in file img                                     
// 	uint8_t filename[32] = "verylargetextwithverylongname.tx";

// 	clear();

// 	// read the file
// 	bytes = fs_file_read(filename, &character, 40000); // max txt file legnth in file img                 
// 	if (bytes == -1) {
// 		result = FAIL;
// 		return result;
// 	}

// 	// print the content buffer
// 	puts((int8_t*)character);

// 	// for(i = 0; i < bytes; i++) {
// 	// 	printf("%c", character[i]);
// 	// }

// 	printf("\n");
// 	printf("file name: %s\n", filename);
// 	testFlag = 0;
// 	return result; 
// }

/* executable file_read test
 * 
 * Check if we can read the content of an executable
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Display the file content
 * Coverage: init file, file read
 * Files: filesys.h/c
 */
// int test_exe_file_read() {
// 	TEST_HEADER;

// 	clear();

// 	int result = PASS;
// 	int bytes;
// 	// int i;                                       
// 	uint8_t character[37000]; // max exe file legnth in file img                              
// 	// uint8_t filename[] = "hello";
// 	uint8_t filename[] = "testprint";

// 	// read the file
// 	bytes = fs_file_read(filename, &character, 37000); // max exe file legnth in file img          
// 	if (bytes == -1) {
// 		result = FAIL;
// 		return result;
// 	}

// 	// print the content buffer
// 	// for(i = 0; i < bytes; i++) {
// 	// 	if (character[i] != '\0') 
// 	// 		putc(character[i]);
// 	// }

// 	bytes = term_write(0, character, bytes);

// 	printf("\n");
// 	printf("file name: %s\n", filename);
// 	return result;
// }

/* term test
 * Check the read and write functions of the terminal
 * Inputs: None
 * Outputs: None
 * Files: terminal.h/S, keyboard.h/c
 */
int test_term_read_write()
{
	TEST_HEADER;

	clear();

	int result = PASS;
	int bytes = 0;
	int8_t character[128]; // the keyboard buffer size

	// check read and write with different buffer sizes
	while (1) {
		term_write(0, (int8_t*)"Terminal reading: 100 bytes\n", 28);
		bytes = term_read(0, character, 100);
		term_write(0, character, bytes);
		term_write(0, (int8_t*)"Terminal reading: 10 bytes\n", 27);
		bytes = term_read(0, character, 10);
		term_write(0, character, bytes);
		term_write(0, (int8_t*)"Terminal reading: 150 bytes\n", 28);
		bytes = term_read(0, character, 150);
		term_write(0, character, bytes);
	}
	
	return result;
}

/* rtc_f_chg_test 
 * 
 * test if you can change frequency
 * Inputs: None
 * Outputs: pass/fail
 * Side Effects: none
 * Coverage: rtc read and right
 * Files: rtc.c, rtc.h
 */
int rtc_f_chg_test(){
	TEST_HEADER;

	int result = PASS;						//set variables
	int i,j;
	int32_t fd;

	clear();								//clear screen
	
	for (i = 2; i <=1024; i = i*2){			//for each power of 2s
		clear();								//clear scren

		rtc_write(fd,  &i,  4);					//set new freq

		for (j = 0; j <i; j++){					//write 1 at freq for every interupt
			printf("1");
			rtc_read(fd, &i, 4);				//read is what 'delays',ensuring 1 is printed at rate desired
		}

		rtc_read(fd, &i, 4);	
	}
	return result;							//ret pass faol 
}

/* rtc_inv_param_test 
 * 
 * test if invalid parameters return -1
 * Inputs: None
 * Outputs: pass/fail
 * Side Effects: none
 * Coverage: parameters
 * Files: rtc.c, rtc.h
 */
int rtc_inv_param_test(){
	TEST_HEADER;
	int32_t fd;
	
	int result1 = PASS;						//set params
	int result2 = PASS;
	int result3 = PASS;

	//tests invalid num bytes
	int nbytes = 5;
	int buf = 4;
	if (rtc_write(fd, &buf, nbytes) != -1)
		result1 = FAIL;
	
	//tests invaild buffer
	buf = NULL;
	nbytes = 4;
	if (rtc_write(fd, &buf, nbytes) != -1)
		result2 = FAIL;
	

	// tests invalid buffer range
	buf = 1;
	if (rtc_write(fd, &buf, nbytes) != -1)
		result3 = FAIL;

	return result1 & result2 & result3;
}


// /* Checkpoint 3 tests */
// int test_systerm_read() {
// 	TEST_HEADER;

// 	clear();
// 	int result = PASS;
// 	int bytes, fd;
// 	// int i;
// 	uint8_t character[200];                                     
// 	uint8_t filename[32] = "frame0.txt";

// 	// read the file
// 	fd = open(filename);
// 	bytes = read(fd, &character, 200);                
// 	if (bytes == -1) {
// 		result = FAIL;
// 		return result;
// 	}

// 	// print the content buffer
// 	puts((int8_t*)character);

// 	// for(i = 0; i < bytes; i++) {
// 	// 	printf("%c", character[i]);
// 	// }

// 	printf("\n");
// 	printf("file name: %s\n", filename);
// 	return result;
// }

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// launch your tests here
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("exc_div_zero_error_test", exc_div_zero_error_test());
	// TEST_OUTPUT("test_read_by_index", test_read_by_index());
	// TEST_OUTPUT("test_read_by_name", test_read_by_name());
	// TEST_OUTPUT("test_dir_read", test_dir_read());
	// TEST_OUTPUT("test_small_file_read", test_small_file_read());
	// TEST_OUTPUT("test_large_file_read", test_large_file_read());
	// TEST_OUTPUT("test_exe_file_read", test_exe_file_read());
	// TEST_OUTPUT("test_term_read_write", test_term_read_write());
	// TEST_OUTPUT("rtc_f_chg_test", rtc_f_chg_test());
	// TEST_OUTPUT("rtc_inv_param_test", rtc_inv_param_test());

	// TEST_OUTPUT("test_systerm_read", test_systerm_read());
    // printf("i was outside of execute");
	// while(1)
	// 	open((uint8_t*) "testprint");
}
