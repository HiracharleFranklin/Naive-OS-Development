/**
 * @file keyboard.c
 * @author: Xiaomin Qiu
 * @modifier: Guanshujie Fu
 * @description: functions for keyboard operation
 * @creat_date: 2022.5. - 
 *             2022.3.19 - move scancode_set and flags from header to here
 *             2022.3.20 - 
 */
#include "lib.h"
#include "keyboard.h"



char scancode_set[NUM_MODE][NUM_SCAN] = {
/* mode 0: CAPS_FLAG =0 and SHIFT_FLAG =0 */  
    {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 
    'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, 0,0,' '
    },
/* mode 1: CAPS_FLAG = 0 and SHIFT_FLAG = 1 */
    {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 
    'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
    },
/* mode 2: CAPS_FLAG = 1 and SHIFT_FLAG = 0*/
    {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '[', ']', 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 
    'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', 0, 0, 0,' '
    },

/* mode 3: CAPS_FLAG = 1 and SHIFT_FLAG = 1 */
    {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '{', '}', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 
    'j', 'k', 'l', ':', '\"', '~', 0, '|', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', '<', '>', '?', 0, 0, 0, ' '
    }
    };
    
/* 
 * keyboard_init
 *  DESCRIPTION: initialize keyboard
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: enable keyboard IRQ 
 */
volatile unsigned char CAPS_FLAG = 0;
volatile unsigned char L_SHIFT_FLAG = 0;
volatile unsigned char R_SHIFT_FLAG = 0;
volatile unsigned char CTRL_FLAG = 0;
volatile unsigned char ALT_FLAG = 0;
volatile unsigned char SCAN_MODE = 0;

void keyboard_init(void){
    CAPS_FLAG = 0;
    L_SHIFT_FLAG = 0;
    R_SHIFT_FLAG = 0;
    CTRL_FLAG = 0;
    ALT_FLAG = 0;
    enable_irq(KB_IRQ);
}


/* keyboard_handler - handles the keyboard interrupt
 *                  output the input character
 *
 * Inputs: None
 * Outputs: None
 * Side Effects: puts char on screen
 */
void keyboard_handler(void){
    cli();
    uint8_t scancode = inb(KB_DATA);     // get the scancode
    SCAN_MODE = (CAPS_FLAG<<1) | (R_SHIFT_FLAG | L_SHIFT_FLAG);
    //printf("%u", scancode);
    int i;

    switch(scancode) {
        case TAB:
            for(i = 0; i < TAB_SPACE; i++){
                printf("%c",' ');
            }
            break;
        case CTRL_RELEASE:
            printf("CTRL_RELEASE");
            CTRL_FLAG = 0;
            break;
        case ALT_RELEASE:
            printf("ALT_RELEASE");
            ALT_FLAG = 0;
            break;
        case L_SHIFT_RELEASE:
            printf("L_SHIFT_RELEASE");
            L_SHIFT_FLAG = 0;
            break;
        case R_SHIFT_RELEASE:
            printf("R_SHIFT_RELEASE");
            R_SHIFT_FLAG= 0;
            break;
        case CTRL_PRESS:
            printf("CTRL_PRESS");
            CTRL_FLAG = 1 ;
            break;
        case ALT_PRESS: 
            printf("ALT_PRESS");
            ALT_FLAG = 1;
            break;
        case R_SHIFT_PRESS:
            printf("R_SHIFT_PRESS");
            R_SHIFT_FLAG = 1;
            key_pressed = R_SHIFT_PRESS;
            break;
        case L_SHIFT_PRESS:
            printf("L_SHIFT_PRESS");
            L_SHIFT_FLAG = 1;
            break;
        case CAPS_PRESS:
            printf("CAPS_PRESS");
            CAPS_FLAG ^= 1;
            break;
        case BACKSPACE:
            printf("BACKSPACE");
            if(KB_BUF_INDEX == 0){
                sti();
                send_eoi(KB_IRQ);
                return; 
            }
            if(KB_BUF_INDEX--<0) KB_BUF_INDEX=0;
            printf("%c",'\b');
            break;

        case ENTER_PRESS:
            printf("ENTER_PRESS");
            keyboard_buffer[KB_BUF_INDEX]='\n';
            KB_BUF_INDEX=0;
            printf("%c", '\n');
            // record key
            key_pressed = ENTER_PRESS;
            break;
        case F1_PRESS:
            printf("F1_PRESS");
            if(ALT_FLAG){
                break;
            }
            break;
        case F2_PRESS:
            printf("F2_PRESS");
            if(ALT_FLAG){
                break;
            }
            break;
        case F3_PRESS:
            printf("F3_PRESS");
            if(ALT_FLAG){
                break;
            }
            break;

        default :
            if(CTRL_FLAG) {
                //not defined now
                break;
            }else{  
                if(scancode_set[SCAN_MODE][scancode]==0 || (scancode > NUM_SCAN)) break;
                if (KB_BUF_INDEX < KB_BUF_SIZE-1){
                    keyboard_buffer[KB_BUF_INDEX]=scancode_set[SCAN_MODE][scancode];
                    KB_BUF_INDEX++; 
                    printf("%c", scancode_set[SCAN_MODE][scancode]);
                    // record key pressed
                    key_pressed = scancode_set[SCAN_MODE][scancode];
                }
            }
    }
     sti();   // end of critical section
     send_eoi(KB_IRQ);   // send EOI to activate interrupt
}
