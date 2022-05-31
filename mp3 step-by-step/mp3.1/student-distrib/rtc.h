/* rtc.h - constants and function interface for rtc.c */
#ifndef _RTC_H
#define _RTC_H

/* Constants */

// two IO ports used for the RTC and CMOS
#define	IDX_PORT		0x70
#define DATA_PORT		0x71

// three status registers
/*	 0A  RTC Status register A:
;
;	    |7|6|5|4|3|2|1|0|  RTC Status Register A
;	     | | | | `---------- rate selection Bits for divider output
;	     | | | |		 frequency (set to 0110 = 1.024kHz, 976.562æs)
;	     | `-------------- 22 stage divider, time base being used;
;	     |			  (initialized to 010 = 32.768kHz)
;	     `-------------- 1=time update in progress, 0=time/date available
;	 
;	 0B  RTC Status register B:
;
;	    |7|6|5|4|3|2|1|0|  RTC Status Register B
;	     | | | | | | | `---- 1=enable daylight savings, 0=disable (default)
;	     | | | | | | `----- 1=24 hour mode, 0=12 hour mode (24 default)
;	     | | | | | `------ 1=time/date in binary, 0=BCD (BCD default)
;	     | | | | `------- 1=enable square wave frequency, 0=disable
;	     | | | `-------- 1=enable update ended interrupt, 0=disable
;	     | | `--------- 1=enable alarm interrupt, 0=disable
;	     | `---------- 1=enable periodic interrupt, 0=disable
;	     `----------- 1=disable clock update, 0=update count normally
;
;	 0C  RTC Status register C (read only):
;
;	    |7|6|5|4|3|2|1|0|  RTC Status Register C  (read only)
;	     | | | | `---------- reserved (set to 0)
;	     | | | `---------- update ended interrupt enabled
;	     | | `---------- alarm interrupt enabled
;	     | `---------- periodic interrupt enabled
;	     `---------- IRQF flag
*/
#define SREG_A		0x8A
#define SREG_B		0x8B
#define SREG_C		0x8C

// basic frequency/interrupt rate
#define BASE_RATE   1024    //base frequency
#define BOTTOM_RATE 2       //theoretical lowest frequency
#define TOP_RATE    32768   //theoretical highest frequency

// use for enable & disable NMI
#define NMI_ENABLE  0x7F
#define NMI_DISABLE 0x80
#define BIT_SIX     0x40
#define UP_MASK     0x0F
#define DOWN_MASK   0xF0

/* Externally-visible functions */

/* Initialize real time clock */
void rtc_init(void);
/* handles interrupt and execute test_interrupt handler */
void rtc_handler(void);

#endif
