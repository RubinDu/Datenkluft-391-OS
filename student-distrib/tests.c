#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"
#include "filesys.h"
#include "malloc.h"

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


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 20 IDT entries are not NULL
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
	for (i = 0; i < 20; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here

/* EXP0_test
 * 
 * Raise division by 0 exception
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Freezes the kernel and display exception if success
 * Coverage: Exceptions
 * Files: idt.h/c
 */
int EXP0_test(){
	TEST_HEADER;
	int a = 1, b = 0;
    a = a / b;
    return FAIL;
}

/* EXP6_test
 * 
 * Raise invalid opcode exception
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Freezes the kernel and display exception if success
 * Coverage: Exceptions
 * Files: idt.h/c
 */
int EXP6_test(){
	TEST_HEADER;
	__asm__("ud2");
	return FAIL;
}

/* Missing_idt_test
 * 
 * Attempt to access a missing idt entry
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Freezes the kernel and display missing segment exception if success
 * Coverage: Exceptions
 * Files: idt.h/c
 */
int Missing_idt_test(){
	TEST_HEADER;
	__asm__("int	$0x1F");
	return FAIL;
}

/* systemcall_test
 * 
 * Raise system call
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Freezes the kernel and display unimplemented if success
 * Coverage: System calls
 * Files: idt.h/c
 */
int systemcall_test(){
	TEST_HEADER;
	__asm__("int	$0x80");
	return FAIL;
}

/* paging_test
 * 
 * Dereference a pointer to a given memory address
 * Inputs: a pointer to an address
 * Outputs: the content of the address pointed by the pointer
 * Side Effects: If the pointer points to an invalid address, Freezes the kernel and display page fault exception
 * Coverage: Paging, Exceptions
 * Files: paging.h/c, idt.h/c
 */
int paging_test(int *ptr){
	TEST_HEADER;
	return *ptr;
}

/* irq_enable_test
 * 
 * Enables an irq of the PIC
 * Inputs: irq index
 * Outputs: PASS
 * Side Effects: Enables the given irq if it is valid
 * Coverage: PIC
 * Files: i8259.c/h
 */
int irq_enable_test(int irq){
	TEST_HEADER;
	enable_irq(irq); // invalid irq number will be return
	return PASS;
}

/* irq_disable_test
 * 
 * Disables an irq of the PIC
 * Inputs: irq index
 * Outputs: PASS
 * Side Effects: Disables the given irq if it is valid
 * Coverage: PIC
 * Files: i8259.c/h
 */
int irq_disable_test(int irq){
	TEST_HEADER;
	disable_irq(irq);
	return PASS;
}

/* eoi_test
 * 
 * Checks if send_eoi can handle invalid irq
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: PIC
 * Files: i8259.c/h
 */
int eoi_test(){
	TEST_HEADER;
	send_eoi(100);
	return PASS;
}

/* rtc_test
 * 
 * Sets the rate of the RTC
 * Inputs: rate
 * Outputs: PASS
 * Side Effects: Sets the rate of the RTC if it is valid
 * Coverage: RTC
 * Files: rtc.c/h
 */
int rtc_test(int32_t rate)
{
	TEST_HEADER;
	rtc_set_rate(rate);
	return PASS;
}

/* all_paging
 * 
 * Dereference all valid memory addresses
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: Paging
 * Files: paging.h/c
 */
int all_paging(){
	int i;
	char val;
	for (i = 0x400000; i < 0x800000; i++)
		val = *(char*)i;
	for (i = 0xB8000; i < 0xB9000; i++)
		val = *(char*)i;
	return PASS;
}


/* Checkpoint 2 tests */
/* terminal_test
 * 
 * read from and then write to terminal
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: Terminal, Keyboard
 * Files: terminal.h/c, keyboard.h/c
 */
int terminal_test(){
	TEST_HEADER;
	char *buf[128];
	memset(buf, 0, 128);
	terminal_read(0, buf, 64);
	terminal_write(0, buf, 64);
	return PASS;
}

/* read_test
 * 
 * read from terminal and output bytes read
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: Terminal, Keyboard
 * Files: terminal.h/c, keyboard.h/c
 */
int read_test(){
    TEST_HEADER;
    char *buf[128];             // buffer to store input, max 128 bytes
    memset(buf, 0, 128);
    printf("Read %d bytes\n", terminal_read(0, buf, 128));
    return PASS;
}


/* rtc_driver_test
 * 
 * test open, close, read, write functions of rtc
 * Inputs: ch - printed character
 * Outputs: PASS
 * Side Effects: none
 * Coverage: RTC
 * Files: rtc.c/h
 */
int rtc_driver_test(char ch)
{
	TEST_HEADER;
	rtc_open(NULL);

	int32_t freq, i, j, max_count = 10;
	char seq[MAX_RATE + 1] = {ch, '\0'};

	for (freq = MIN_FREQUENCY, i = 1; freq <= MAX_FREQUENCY; freq = (freq << 1), ++i, max_count = max_count << 1) {
		clear();
		seq[i] = ++ch;
		rtc_write(NULL, &freq, sizeof(int32_t));

		for (j = 0; j < max_count; ++j) {
			printf("Frequency: %d #%d ", freq, j);
			printf(seq);
			printf("\n");
			rtc_read(NULL, NULL, NULL);
		}
	}

	rtc_close(NULL);
	return PASS;
}

/* rtc_driver_test_timer
 * 
 * Sets the rate of the RTC
 * Inputs: rate
 * Outputs: PASS
 * Side Effects: Sets the rate of the RTC if it is valid
 * Coverage: RTC
 * Files: rtc.c/h
 */
int rtc_driver_test_timer()
{
	TEST_HEADER;
	rtc_open(NULL);

	int32_t i, frequency = 0x100;
	printf("Default Frequency: 0\n");
	for (i = 1; i <= 16; ++i) { /* 8 seconds total, 2 Hz * 16 = 8s */
		rtc_read(NULL, NULL, NULL);
		if (!(i & 1))
			printf("Default Frequency: %d\n", i >> 1);
	}

	printf("\n");
	rtc_write(NULL, &frequency, sizeof(int32_t));
	printf("Frequency %d : 0\n", frequency);
	for (i = 1; i <= 0x2000; ++i) { /* */
		rtc_read(NULL, NULL, NULL);
		if (!(i & 0xFF))
			printf("Frequency %d : %d\n", frequency, i >> 8);
	}

	rtc_close(NULL);
	return PASS;
}

/* rtc_write_test
 * 
 * test write with invalid input
 * Inputs: none
 * Outputs: PASS or FAIL
 * Side Effects: none
 * Coverage: RTC
 * Files: rtc.c/h
 */
int rtc_write_test()
{
	TEST_HEADER;
	rtc_open(NULL);
	int32_t ref;
	if (rtc_write(NULL, NULL, NULL) != -1) { 			/* nullptr input */
		return FAIL;
	}
	ref = 0xECE391;
	if (rtc_write(NULL, &ref, sizeof(int32_t)) != -1) { /* not power of 2 */
		return FAIL;
	}
	ref = 0x8000;
	if (rtc_write(NULL, &ref, sizeof(int32_t)) != -1) { /* too large */
		return FAIL;
	}
	ref = -0x8000;
	if (rtc_write(NULL, &ref, sizeof(int32_t)) != -1) { /* too small */
		return FAIL;
	}
	ref = 0x100;
	if (rtc_write(NULL, &ref, sizeof(int32_t)) != 0) {  /* correct output */
		return FAIL;
	}
	if (rtc_write(NULL, &ref, 0) != -1) {				/* invalid size */
		return FAIL;
	}
	rtc_close(NULL);
	return PASS;
}


int read_name_test(uint8_t* filename) {
    TEST_HEADER;
    dentry_t test_dentry;
    unsigned int result;

    result = read_dentry_by_name(filename, &test_dentry);

    if (result == -1) {
        return FAIL;
    } else {
        return PASS;
    }
}

int dir_read_test( void )
{
    dentry_t curr_dentry;

    uint8_t test_buf[MAX_FILE_NAME+1];
    char test_dir[] = ".";
    int32_t test_fd = 2;
    clear();
    dir_open((const uint8_t*) test_dir);

    while(dir_read(test_fd, test_buf, MAX_FILE_NAME) > 0) {
        printf("file_name: ");
        terminal_write(0, test_buf, MAX_FILE_NAME);
		
        read_dentry_by_name(test_buf, &curr_dentry);

        uint32_t size = inode_block[curr_dentry.inode_num].file_size;

        printf("  file_type: %d, file_size: %d", curr_dentry.file_type, size);
        printf("\n");
    }
    dir_close(test_fd);
    return PASS;		
}

int file_read_test(uint8_t* filename) {
    clear();
    dentry_t test;
    char buff[40000] = {'\0'}; // Buffer to hold the file data
    int byte_counter; // hold the number of bytes read

    if (read_dentry_by_name(filename, &test) != 0) {
        printf("Failed to read directory entry for %s\n", filename);
        return FAIL;
    }

    byte_counter = read_data(test.inode_num, 0, (uint8_t*)buff, 10000);
    printf("read %d bytes\n", byte_counter);
    terminal_write(0, buff, byte_counter);
    return PASS;
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("Div by 0 exception test", EXP0_test());
	//TEST_OUTPUT("Invalid opcode exception test", EXP6_test());
	//TEST_OUTPUT("Missing idt entry test", Missing_idt_test());
	//TEST_OUTPUT("System call test", systemcall_test());
	//TEST_OUTPUT("Valid pointer test: Kernel", paging_test((int*)0x700000));
	//TEST_OUTPUT("Valid pointer test: Video Memory", paging_test((int*)0xB8000));
	//TEST_OUTPUT("Invalid pointer test", paging_test((int*)0x3FFFFF));
	//TEST_OUTPUT("RTC rate test", rtc_test(15));
	//TEST_OUTPUT("IRQ enable test", irq_enable_test(100));
	//TEST_OUTPUT("IRQ disable test", irq_disable_test(1));
	// TEST_OUTPUT("All paging test", all_paging());


	
    /* **************************************************
     * *           Checkpoint #2 Testcases              *
     * **************************************************/
	// while (1)
	// 	TEST_OUTPUT("Terminal test", terminal_test());

	// TEST_OUTPUT("Read test", read_test());

	// TEST_OUTPUT("test dentry_by_name", read_name_test((uint8_t*)"frame0.txt"));
	// TEST_OUTPUT("test dentry_by_name", read_name_test((uint8_t*)"verylargetextwithverylongname.tx"));
	// TEST_OUTPUT("dir read test", dir_read_test());
	// TEST_OUTPUT("read file test", file_read_test((uint8_t*)"frame0.txt"));
	// TEST_OUTPUT("read file test", file_read_test((uint8_t*)"verylargetextwithverylongname.txt"));
	// TEST_OUTPUT("read file test", file_read_test((uint8_t*)"shell"));

	// TEST_OUTPUT("RTC Write Input Test", rtc_write_test());
	// TEST_OUTPUT("RTC Driver Test", rtc_driver_test_timer());
	// TEST_OUTPUT("RTC Driver Test", rtc_driver_test('0'));

	void *zero = malloc(0);
	TEST_OUTPUT("zero-size memory", !zero);
	
	void *small = malloc(16);
	TEST_OUTPUT("small-size memory", small);
	void *has_not_padding = realloc(small, 32);
	TEST_OUTPUT("small-size memory realloc (free blocks behind)", has_not_padding == small);
	void *padding = malloc(128);
	void *has_padding = realloc(small, 64);
	TEST_OUTPUT("small-size memory realloc (no free blocks behind)", has_padding != small);
	free(has_padding);
	free(padding);

	void *big = malloc(0xECE391);
	TEST_OUTPUT("big-size memory", big);
	free(big);

	void *too_big = malloc(0xECEB3026);
	TEST_OUTPUT("too-big-size memory", !too_big);
	free(too_big);

	// while (1); //freezes the kernel so we can see the output
}
