#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "rtc.h"
#include "keyboard.h"
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
	
	// TEST_OUTPUT("RTC Write Input Test", rtc_write_test());
	// TEST_OUTPUT("RTC Driver Test", rtc_driver_test_timer());
	// TEST_OUTPUT("RTC Driver Test", rtc_driver_test('0'));
	
	while (1); //freezes the kernel so we can see the output
}
