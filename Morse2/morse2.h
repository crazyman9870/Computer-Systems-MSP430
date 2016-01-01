/*
 * morse2.h
 *
 *  Created on: Mar 11, 2015
 *      Author: WebUser
 */

#ifndef MORSE2_H_
#define MORSE2_H_

#include <stdlib.h>
#include <ctype.h>

#define myCLOCK		1100000
#define WDT_CTL     WDT_MDLY_0_5            //; WD: Timer, SMCLK, 0.5 ms
#define WDT_CPI     500                     //; WDT Clocks Per Interrupt (@1 Mhz)
#define WDT_IPS     (myCLOCK/WDT_CPI)       //; WDT Interrupts Per Second
#define STACK       0x0600                  //; top of stack

#define END         0
#define DOT         1
#define DASH        2
#define ELEMENT     ((WDT_IPS*240)/1000)    //; (WDT_IPS * 6 / WPM) / 5
#define DEBOUNCE	10

#endif /* MORSE2_H_ */
