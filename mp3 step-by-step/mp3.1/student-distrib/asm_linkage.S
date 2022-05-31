
# asm_linkage.S
# 
# Ahthor: Guanshujie Fu
# Modifier: 
# Description: functions for asm linkages used in idt
# Creat Date: 2022.5.18 - add linkages rtc_handler_linkage and keyboard_handler_linage
#
#define ASM 1
#include "asm_linkage.h"
.globl rtc_handler_linkage, keyboard_handler_linkage




# rtc_handler_linkage
#   Description: asm linkage for rtc_handler
#   Input: none
#   Output: none
#   Notice: iret is required as it is returned from an interrupt
#
rtc_handler_linkage:
    pushfl
    pushal
    call rtc_handler
    popal
    popfl
    iret



# keyboard_handler_linkage
#   Description: asm linkage for keyboard_handler
#   Input: none
#   Output: none
#   Notice: iret is required as it is returned from an interrupt
#
keyboard_handler_linkage:
    pushfl
    pushal
    call keyboard_handler
    popal
    popfl
    iret
