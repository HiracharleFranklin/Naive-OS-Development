/**
 * @file: asm_linkage.h
 * 
 * @ahthor: Guanshujie Fu
 * @description: headers for asm linkages used in idt
 * @creat_date: 2022.5.18 - add headers
 */
#ifndef ASM_LINKAGE_H
#define ASM_LINKAGE_H
#ifndef ASM


#include "rtc.h"
#include "keyboard.h"


// linkages for handlers
extern void rtc_handler_linkage();
extern void keyboard_handler_linkage();

#endif
#endif
