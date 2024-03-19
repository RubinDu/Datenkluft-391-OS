#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "rtc.h"
#include "keyboard.h"

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
 * Outputs: PASS/FAIL
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

/* systemcall_test
 * 
 * Raise system call
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Freezes the kernel and display unimplemented if success
 * Coverage: System calls
 * Files: idt.h/c
 */
int systemcall_test(){
	TEST_HEADER;
	__asm__("int	$0x80");
	return FAIL;
}

/* systemcall_test
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

int kb_test(){
	TEST_HEADER;
	__asm__("int	$0x21");
	return 1;
}

int clock_test(){
	TEST_HEADER;
	__asm__("int	$0x28");
	return 1;
}

int irq_enable_test(){
	TEST_HEADER;
	enable_irq(100); // invalid irq number will be return
	return PASS;
}

int irq_disable_test(){
	TEST_HEADER;
	disable_irq(100);
	return PASS;
}

int eoi_test(){
	TEST_HEADER;
	send_eoi(100);
	return PASS;
}

// int rtc_test( void )
// {
// 	TEST_HEADER;
// 	int i;
// 	for(i=0; i<1000000; i++){};
// 	rtc_init();
// 	for(i=0; i<1000000; i++){};
// 	disable_irq(RTC_IRQ);
// 	return PASS;
// }

// int keyboard_test(){
// 	TEST_HEADER;
// 	keyboard_init();
// 	disable_irq(1); //keyboard irq = 0x1
// 	return PASS;
// }

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("Div by 0 exception test", EXP0_test());
	//TEST_OUTPUT("System call test", systemcall_test());
	TEST_OUTPUT("Valid pointer test: Kernel", paging_test((int*)0x400000));
	TEST_OUTPUT("Valid pointer test: Video Memory", paging_test((int*)0xB8000));
	TEST_OUTPUT("Invalid pointer test", paging_test((int*)0x3FFFFF));
	
	while (1);
	
	// launch your tests here
	// TEST_OUTPUT("irq_enable_test", irq_enable_test());
	// TEST_OUTPUT("irq_disable_test", irq_disable_test());
	// TEST_OUTPUT("eoi_test", eoi_test());
	//TEST_OUTPUT("rtc_test", rtc_test());	
	//TEST_OUTPUT("keyboard_test", keyboard_test());
	// TEST_OUTPUT("kb_test", kb_test());
	//TEST_OUTPUT("clock_test", clock_test());
}
