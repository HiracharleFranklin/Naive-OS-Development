/**
 * @file: rtc.c
 * 
 * @ahthor: Zitai Kong
 * @modifier: Guanshujie Fu
 * @description: functions for rtc initialization and rate set
 * @creat_date: 2022.3. - 
 *             2022.3.20 - rtc_switch tests
 */
;
#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "rtc.h"


/* 
 * rtc_init
 *  DESCRIPTION: Initialize real time clock
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: enable rtc IRQ 
 */
void rtc_init(void)
{
    /* set the flag
     * since cli_and_save needs long type, use 32 bits */
    uint32_t flag;

    /* create the critical section */
	cli_and_save(flag);     // disable interrupts

    /* disable rtc showing */
    turn_off_rtc();

    /* Avoiding NMI and Other Interrupts While Programming */
    // disable other interrupts - done by cli()

    // disable NMI
    // taken from osdev/NMI 
    // outb(0x70, inb(0x70) | 0x80);
    // inb(0x71);
    outb(inb(IDX_PORT) | NMI_DISABLE, IDX_PORT);

    /* Turn on IRQ8 */
    outb(SREG_B,IDX_PORT);              // select register B, and disable NMI
    char prev=inb(DATA_PORT);	        // read the current value of register B
    outb(SREG_B,IDX_PORT);              // set the index again (a read will reset the index to register D)
    outb(prev | BIT_SIX, DATA_PORT);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
                                        // This enables periodic interrupt
    enable_irq(8);                      // Turn on IRQ8

    /* Set to base interrupt rate */
    //set_irate(BASE_RATE);
    int32_t frequency = 2; //BASE_RATE;
    int32_t rate = 0;
    char prev2;
    // find rate by applying
    // frequency =  32768 >> (rate-1);
	frequency = TOP_RATE / frequency;
	while(frequency > 0)
	{
		rate = rate + 1;
		frequency = frequency >> 1;
	}
    rate &= UP_MASK;			// rate must be above 2 and not over 15

    // disable interrupts and NMI
    cli();

    // set interrupt rate
    outb(SREG_A,IDX_PORT);		                // set index to register A, disable NMI
    prev2=inb(DATA_PORT);	                // get initial value of register A
    outb(SREG_A,IDX_PORT);		                // reset index to A
    outb((prev2 & DOWN_MASK) | rate, DATA_PORT); //write only our rate to A. Note, rate is the bottom 4 bits.

    // enable interrupts
    sti();

    /* enable NMI */
    // taken from osdev/NMI 
    // outb(0x70, inb(0x70) & 0x7F);
    // inb(0x71);
    //outb(inb(IDX_PORT) & NMI_ENABLE, IDX_PORT);

    /* end the critical section */
	restore_flags(flag);
	sti();
}


/* 
 * set_irate
 *  DESCRIPTION: handles rtc interrupt
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: execute test_interrupt handler
 */
void rtc_handler(void)
{
        cli();

        /* Execute test_interrupt handler */
        if (get_rtc_switch() == 1)
            test_interrupts();
        // printf("RTC interrupt happens!\n");

        /* get another interrupt */
        outb(SREG_C,IDX_PORT);  // select register C
        // This line read the data from data port but throw it away
        inb(DATA_PORT);		    // just throw away contents


        sti();

        /* send eoi to end the interrupt */
        send_eoi(8);    
    
}
