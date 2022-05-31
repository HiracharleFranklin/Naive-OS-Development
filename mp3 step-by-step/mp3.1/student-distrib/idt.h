/**
 * @file idt.c
 * @author: Xiaomin Qiu
 * @modifier: 
 * @description: functions headers for idt, including exceptions
 * @creat_date: 2022.5. - 
 *             
 */
#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "lib.h"
#include "asm_linkage.h"
								

#define SYSCALL 	0x80
#define RTC 		0x28
#define KEYBOARD 	0x21
#define PIT			0x20
#define DPL_KERNEL  0
#define DPL_USER    3

void interrupt_init(void);

#endif
