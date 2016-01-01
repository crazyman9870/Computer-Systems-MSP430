#include <msp430.h>
#include "msp430.h"          // .cdecls C,"msp430.h"
#include "morse2.h"
#include <stdlib.h>
#include <ctype.h>
//#include <string>

//extern int main_asm(void);
extern int beep_cnt;
extern int delay_cnt;
extern int green2cnt;
extern int debounce;

extern char* letters[];
extern char* numbers[];
extern void beep(int);
extern void delay(int);
const int numOfWords = 18;

void intercharacter() {
	int wait = ELEMENT * 2;
	delay(wait);
}
void doSpace() {
	int wait = ELEMENT * 4;
	delay(wait);
}

void doDot() {
	int wait = ELEMENT;
	beep(wait);
	wait = ELEMENT;
	delay(wait);
}

void doDash() {
	int wait = ELEMENT * 3;
	beep(wait);
	wait = ELEMENT;
	delay(wait);
}
//letter:		sub.w	#'A',r5                ; make index 0-25
		//			mov.w	letters(r5),r5         ; get pointer to letter codes
		//			jmp		loop10
		//
		//number:		sub.w	#48,r5					;make index 0-9
		//			add.w	r5,r5					;make number index
		//			mov.w	numbers(r5),r5			;get pointer to number codes
		//			jmp loop10
		//
		//space:		call	#doSpace
		//			jmp		loop02
		//
char processLetter(char c) {
	c = toupper(c);
	c = c - 'A';
	return c;
}

char processNum(char c) {
	c = c - '0';
	return c;
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1_ISR(void) {
	P1IFG&=~0x0f;
	debounce = DEBOUNCE;
	return;
}

int main(void)
{
	char message[18] = "HELLO CS 124 WORLD";
									// Stop watchdog timer
									//.def	main_asm
	WDTCTL = WDT_CTL;				//main_asm:   mov.w #WDT_CTL,&WDTCTL        ; set WD timer interval
	IE1 = WDTIE;					//            mov.b #WDTIE,&IE1             ; enable WDT interrupt
	P4DIR = 0x6f;					//            bis.b #0x20,&P4DIR            ; set P4.5 as output (speaker)
									//            bis.b	#0x1f,&P4DIR			;enable the rest of the lights
	P3DIR = 0x10;					//            bis.b	#0x10,&P3DIR			;green2 enabled
									//
									//
	beep_cnt = 0;				//            clr.w &beep_cnt               ; clear counters
	delay_cnt = 0;				//            clr.w &delay_cnt
	green2cnt = WDT_IPS;		//            mov.w	#WDT_IPS,&green2cnt
	debounce = 0;				//            clr.w	&debounce
									//
	P1SEL &= ~0x0f;					//            bic.b	#0x0f,&P1SEL          ; select GPIO
	P1DIR &= ~0x0f;					//			bic.b	#0x0f,&P1DIR          ; configure P1.0 as inputs
	P1OUT |= 0x0f;					//			bis.b	#0x0f,&P1OUT          ; use pull-up
	P1REN |= 0x0f;					//			bis.b	#0x0f,&P1REN          ; enable pull-up
	P1IES |= 0x0f;					//			bis.b	#0x0f,&P1IES          ; trigger on high to low transition
	P1IE |= 0x0f;					//			bis.b	#0x0f,&P1IE           ; P1.0 interrupt enabled
	P1IFG &= ~0x0f;					//			bic.b	#0x0f,&P1IFG          ; P1.0 IFG cleared
									//
//	__bis_SR_register(GIE);			//            bis.w   #GIE,SR                 ; enable interrupts
	__enable_interrupt();			//
	P4OUT &= ~0x6f;					//			RED_OFF
									//			YELLOW_OFF
									//			ORANGE_OFF
									//			GREEN_OFF
									//			GREEN2_OFF
									//			RED2_OFF
									//			LED_OFF
									//
	while(1) {						//loop:		mov.w	#message,r4            ; point to message
		unsigned int i;

		for(i = 18; i > 0; i--)	{
			char c = message[numOfWords - i];	//loop02:		mov.b	@r4+,r5                ; get character
			char* code;
			char value;
			if(c == 0) {			//			cmp.b	#0,r5
				break;				//			jeq		loop
			}


			else if(c == 32) {		//			cmp.b	#32,r5
				doSpace();			//			jeq		space
				continue;
			}

			else if(c < 58) {		//			cmp.b	#58,r5
				value = processNum(c);		//			jlo		number
				code = numbers[value];
			}

			else {					//			cmp.b	#64,r5
				value = processLetter(c);	//			jge		letter
				code = letters[value];
			}						//

			while(value = *code++) {	//loop10:		mov.b	@r5+,r6                ; get DOT, DASH, or END
				if(value == DOT) {		//			cmp.b	#DOT,r6                ; dot?
					doDot();			//			call	#doDot
				}
				else if (value == DASH) {	//dash:		cmp.b	#DASH,r6
					doDash();				//			call	#doDash
				}
			}
			intercharacter();

		}
	}
    return 0;
}

