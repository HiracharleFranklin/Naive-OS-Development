/**
 * @file tests.c
 * @author: 
 * @modifier: Jiahao Wei, Zitai Kong, Guanshujie Fu
 * @description: functions for keyboard operation
 * @creat_date: 2022.3. -  add tests functions for cp1
 *             2022.3.19 - add page fault test
 *             2022.3.20 - add system call test
 */

#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "rtc.h"
#include "paging.h"
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
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	clear();
	set_cursor(0, 0);
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

	printf("\n=============== System Call Test ===============\n");
	printf("Try to access INT 0x80\n");
	printf("Press Enter to test System Call or Right Shift to skip.\n");
	while (key_pressed != 0x1c && key_pressed != 0x36);
	if (key_pressed == 0x1c)
		asm ("int $0x80");
	key_pressed = 0;

	return result;
}

/**
 * brief: show keyboard input on screen
 * 
 * @return int
 */
int keyboard() {
	clear();
	set_cursor(0, 0);
	TEST_HEADER;

	printf("\nPress ` to exit test.\n");
	printf("Start Enter: ");
	while (key_pressed != '`'); key_pressed = 0;
	return PASS;
}

/* RTC Test
 * 
 * Receiving an RTC interrupt
 * Inputs: None
 * Outputs: None
 * Side Effects: print effects on screen
 * Coverage: RTC handler
 * Files: rtc.h/c
 */
int rtc_test(){
	clear();
	set_cursor(0, 0);
	TEST_HEADER;

	printf("Press Enter to test rtc.\n");
	while (key_pressed != 0x1c); key_pressed = 0;
	turn_on_rtc();
	while (key_pressed != 0x1c); key_pressed = 0;
	turn_off_rtc();
	clear();
	set_cursor(0, 0);

	return PASS;
}


/**
 * brief: 1. show the contents of page dir and table
 *        2. dereferencing different address ranges with paging turned on
 * inputs: none
 * outputs: print the contents to screen
 */
int paging_test(void) {
	clear();
	set_cursor(0, 0);
	TEST_HEADER;

	uint32_t addr;
	uint32_t page_fault;
		
	show_page_dir(kernel_page_dir);
    show_page_table(kernel_page_table_0_4M);

	for (addr = 0x400000; addr < 0x800000; addr += 0x100000 + 42) {
		printf("Addr: %#x\tContent: %#x\n", addr, *(uint32_t *)addr);
	}
	addr = 0x8F0000;
	printf("\n=============== Page Fault Exception Test ===============\n");
	printf("Try to access address: %#x \n", addr);
	printf("Press Enter to test page fault or Right Shift to skip\n");
	while (key_pressed != 0x1c && key_pressed != 0x36); 
	if (key_pressed == 0x1c)
		page_fault = *(uint32_t *)addr;
	else
		page_fault = 0;
	key_pressed = 0;
	clear();
	set_cursor(0, 0);

	return PASS;
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// launch your tests here
	printf("Press Enter to start rtc test.\n");
	while (key_pressed != 0x1c); key_pressed = 0;
	TEST_OUTPUT("rtc_test", rtc_test());

	printf("Press Enter to start idt test.\n");
	while (key_pressed != 0x1c); key_pressed = 0;
	TEST_OUTPUT("idt_test", idt_test());

	printf("Press Enter to start page test.\n");
	while (key_pressed != 0x1c); key_pressed = 0;
	TEST_OUTPUT("paging_test", paging_test());

	printf("Press Enter to start keyboard test.\n");
	while (key_pressed != 0x1c); key_pressed = 0;
	TEST_OUTPUT("keyboard_test", keyboard());

	clear();
	printf("Test Ends!\n");
}
