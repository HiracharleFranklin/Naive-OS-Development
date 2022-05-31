/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

// global variables
unsigned char buttons;
unsigned int led;
unsigned int cmd_free = 1;
unsigned long flag;
//static spinlock_t lock = SPIN_LOCK_UNLOCKED;
/* seven-segment display codes */
char sevenseg[16] = 
{
	0xE7, //0
	0x06, //1
	0xCB, //2
	0x8F, //3
	0x2E, //4
	0xAD, //5
	0xED, //6
	0x86, //7
	0xEF, //8
	0xAF, //9
	0xEE, //A
	0x6D, //B
	0xE1, //C
	0x4F, //D
	0xE9, //E
	0xE8  //F

};

/*
; 	Mapping from 7-segment to bits
; 	The 7-segment display is:
;		  _A
;		F| |B
;		  -G
;		E| |C
;		  -D .dp
;
; 	The map from bits to segments is:
; 
; 	__7___6___5___4____3___2___1___0__
; 	| A | E | F | dp | G | C | B | D | 
; 	+---+---+---+----+---+---+---+---+
*/
static int tux_set_led (struct tty_struct* tty, unsigned long arg);

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	char buf[2];
    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];
	/* packet is what device sends back to driver
	 * a - opcode
	 * b - Data[1]
	 * c - Data[2]
	*/

	switch (a)
	{
	case MTCP_ACK:
		//printk("MTCP_ACK called\n");
		// free the occupation indicate flag
		cmd_free = 1;
		break;
	case MTCP_BIOC_EVENT:
		//printk("MTCP_BIOC_EVENT called\n");
		//printk("%02x\n",b);
		//printk("%02x\n",c);
		// First reverse since active low
		b = ~b;
		c = ~c;
		// put them in required order
		buttons = 0;
		buttons = (b & 0x0F) | ((c & 0x09)<<4) | ((c & 0x02)<<5) | ((c & 0x04)<<3);
		//printk("button presses:");
		//printk("%02x",(int)buttons);
		//printk("\n");
		break;
	case MTCP_RESET:
		//printk("MTCP_RESET called\n");
		cmd_free = 1;
		//first initialize
		// use MTCP_BIOC_ON to enable the button
		buf[0] = MTCP_BIOC_ON;
		// To make content to display
		// First Put the LED display into user-mode
		// Only In this mode, the value specified by 
		// the MTCP_LED_SET command is displayed.
		buf[1] = MTCP_LED_USR;
		//Give command to the device.
		tuxctl_ldisc_put(tty, buf, 2);

		//Then set LED
		tux_set_led (tty, led);
		break;
	default:
		break;
	}
	return;
    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
	char buf[2];
	unsigned long copy_check;
	//printk("cp1\n");
    switch (cmd) {
	case TUX_INIT:
		/* initialize any variables with driver 
		 * input: none
		 * return: 0 
		*/
		//printk("tux init called\n");
		//spin_lock_irqsave (&lock, flag);
		if (cmd_free == 0){
			//spin_unlock_irqrestore (&lock, flag);
			return -EINVAL;
		}
		cmd_free = 1;
		//spin_unlock_irqrestore (&lock, flag);
		// use MTCP_BIOC_ON to enable the button
		buf[0] = MTCP_BIOC_ON;
		// To make content to display
		// First Put the LED display into user-mode
		// Only In this mode, the value specified by 
		// the MTCP_LED_SET command is displayed.
		buf[1] = MTCP_LED_USR;
		//Give command to the device.
		tuxctl_ldisc_put(tty, buf, 2);

		return 0;

	case TUX_BUTTONS:
		//printk("tux button called\n");
		/* 
		 * Enable the device driver to report when buttons are pressed and released
		 * input: arg: pointer to a 32 bit integer
		 * return: 0 if success, -EFAULT if not valid  
		*/

		// check if pointer is valid
		if ((int*) arg == NULL)
			return -EINVAL;

		// copy a 32bit button setting to user level
		// to-from-copy 4 bytes
		copy_check = copy_to_user((int*)arg, &buttons, 4);
		// Check if success
		if (copy_check > 0)
			return -EINVAL;
		else
			return 0;

	case TUX_SET_LED:
		/* set LED */
		//spin_lock_irqsave (&lock, flag);
		if (cmd_free == 0){
			//spin_unlock_irqrestore (&lock, flag);
			return -EINVAL;
		}
		cmd_free = 0;
		// put user mode
		buf[0] = MTCP_LED_USR;
		//Give command to the device.
		tuxctl_ldisc_put(tty, buf, 1);
		//spin_unlock_irqrestore (&lock, flag);
		tux_set_led (tty, arg);
		//if (flag == 1) {break;}
		//flag = 1;
		//printk("TUX_SET_LED called\n");
		return 0;

	case TUX_LED_ACK:
		return 0;

	case TUX_LED_REQUEST:
		return 0;

	case TUX_READ_LED:
		return 0;

	default:
	    return -EINVAL;
    }
	return 0;
}

/* tux_set_led
 * input: tty - the device
 * 		  arg - LED display information
 * output: none
 * return: 0 if success
 * side effect: set led information
 */
int tux_set_led (struct tty_struct* tty, unsigned long arg)
{

	char ledbuf[6];
	unsigned int decimal;
	unsigned int ledno;
	unsigned int display;
	int i;
	int oneledno[4];
	int onedecimal[4];
	int onedisplay[4];
	int index;
	//int skip_count;

	//printk("tux set led called\n");
	//printk("led: %lx \n", arg);
	//printk("check\n");

	// Then extract the values
	led = arg;							// store current led feature for reset
	//printk("led: %08x", led);
	display = arg & 0x0000FFFF;			// The content to display
	//printk("display: %08x", display);
	ledno = (arg & 0x000F0000) >> 16;	// which LED to turn on
	//printk("ledno: %08x", ledno);
	decimal = (arg & 0x0F000000) >> 24;	// whther to turn on the decimal
	//printk("decimal: %08x", decimal);
	oneledno[0]= (ledno & 1);
	oneledno[1]= (ledno & 2)>>1;
	oneledno[2]= (ledno & 4)>>2;
	oneledno[3]= (ledno & 8)>>3;
	onedecimal[0] = (decimal & 1);
	onedecimal[1] = (decimal & 2)>>1;
	onedecimal[2] = (decimal & 4)>>2;
	onedecimal[3] = (decimal & 8)>>3;
	onedisplay[0] = (0x000F & display);
	onedisplay[1] = (0x00F0 & display) >> 4;
	onedisplay[2] = (0x0F00 & display) >> 8;
	onedisplay[3] = (0xF000 & display) >> 12;
	
	//create buffer to transmit to Tux Controller
	ledbuf[0] = MTCP_LED_SET;
	ledbuf[1] = 0x0F;
	//skip_count = 2;
	for (i = 0; i < 4; i++) {
		// If light is off, print nothing
		if (oneledno[i] == 1) {
			index = onedisplay[i];
			if (onedecimal[i] == 1) {
				ledbuf[2+i] = (sevenseg[index] | 0x10);
			} else {
				ledbuf[2+i] = sevenseg[index];
			}
			//skip_count++;
		} else {
			ledbuf[2+i] = 0x00;
		}
	}
	//printk("ledno: %08x \n", ledno); -> if not removed, the game will conflict!
	//printk("skip_count: %d \n", skip_count);
	// write LED values to Tux 
	tuxctl_ldisc_put(tty, ledbuf, 6);
	return 0;
}
