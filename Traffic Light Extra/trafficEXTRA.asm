;THIS IS THE WORK OF Anthony Constantino

;*******************************************************************************
;   Lab 5b - traffic.asm
;
;   Description:  1. Turn the large green LED and small red LED on and
;                    delay 20 seconds while checking for orange LED.
;                    (If orange LED is on and 10 seconds has expired, immediately
;                    skip to next step.)
;                 2. Turn large green LED off and yellow LED on for 5 seconds.
;                 3. Turn yellow LED off and large red LED on.
;                 4. If orange LED is on, turn small red LED off and small green
;                    LED on.  After 5 seconds, toggle small green LED on and off
;                    for 6 seconds at 1 second intervals.  Finish by toggling
;                    small green LED on and off for 4 seconds at 1/5 second
;                    intervals.
;                    Else, turn large red LED on for 5 seconds.
;                 5. Repeat the stoplight cycle.
;
;   I certify this to be my source code and not obtained from any student, past
;   or current.
;
;*******************************************************************************
;                            MSP430F2274
;                  .-----------------------------.
;            SW1-->|P1.0^                    P2.0|<->LCD_DB0
;            SW2-->|P1.1^                    P2.1|<->LCD_DB1
;            SW3-->|P1.2^                    P2.2|<->LCD_DB2
;            SW4-->|P1.3^                    P2.3|<->LCD_DB3
;       ADXL_INT-->|P1.4                     P2.4|<->LCD_DB4
;        AUX INT-->|P1.5                     P2.5|<->LCD_DB5
;        SERVO_1<--|P1.6 (TA1)               P2.6|<->LCD_DB6
;        SERVO_2<--|P1.7 (TA2)               P2.7|<->LCD_DB7
;                  |                             |
;         LCD_A0<--|P3.0                     P4.0|-->LED_1 (Green)
;        i2c_SDA<->|P3.1 (UCB0SDA)     (TB1) P4.1|-->LED_2 (Orange) / SERVO_3
;        i2c_SCL<--|P3.2 (UCB0SCL)     (TB2) P4.2|-->LED_3 (Yellow) / SERVO_4
;         LCD_RW<--|P3.3                     P4.3|-->LED_4 (Red)
;   TX/LED_5 (G)<--|P3.4 (UCA0TXD)     (TB1) P4.4|-->LCD_BL
;             RX-->|P3.5 (UCA0RXD)     (TB2) P4.5|-->SPEAKER
;           RPOT-->|P3.6 (A6)          (A15) P4.6|-->LED 6 (R)
;           LPOT-->|P3.7 (A7)                P4.7|-->LCD_E
;                  '-----------------------------'
;
;*******************************************************************************
;*******************************************************************************
            .cdecls  C,LIST,"msp430.h"      ; MSP430

            .asg    "bis.b #0x08,&P4OUT",RED_ON
            .asg    "bic.b #0x08,&P4OUT",RED_OFF
            .asg    "xor.b #0x08,&P4OUT",RED_TOGGLE
            .asg    "bit.b #0x08,&P4OUT",RED_TEST

            .asg   "bis.b #0x04,&P4OUT",YELLOW_ON
	        .asg   "bic.b #0x04,&P4OUT",YELLOW_OFF
	        .asg   "xor.b #0x04,&P4OUT",YELLOW_TOGGLE
	        .asg   "bit.b #0x04,&P4OUT",YELLOW_TEST

	        .asg   "bis.b #0x02,&P4OUT",ORANGE_ON
	        .asg   "bic.b #0x02,&P4OUT",ORANGE_OFF
	        .asg   "xor.b #0x02,&P4OUT",ORANGE_TOGGLE
	        .asg   "bit.b #0x02,&P4OUT",ORANGE_TEST
	        .asg   "bit.b #0x02,&P4OUT",ORANGE_TEST

	        .asg   "bis.b #0x01,&P4OUT",GREEN_ON
	        .asg   "bic.b #0x01,&P4OUT",GREEN_OFF
	        .asg   "xor.b #0x01,&P4OUT",GREEN_TOGGLE
	        .asg   "bit.b #0x01,&P4OUT",GREEN_TEST

	        .asg   "bis.b #0x40,&P4OUT",RED2_ON
	        .asg   "bic.b #0x40,&P4OUT",RED2_OFF
	        .asg   "xor.b #0x40,&P4OUT",RED2_TOGGLE
	        .asg   "bit.b #0x40,&P4OUT",RED2_TEST

	        .asg   "bis.b #0x10,&P3OUT",GREEN2_ON
	        .asg   "bic.b #0x10,&P3OUT",GREEN2_OFF
	        .asg   "xor.b #0x10,&P3OUT",GREEN2_TOGGLE
	        .asg   "bit.b #0x10,&P3OUT",GREEN2_TEST

FOUR		.equ	4
FIVE		.equ	5
SIX			.equ	6

ZEROSEC		.equ    0
HALFSEC		.equ	4	;this will be smaller bcz there are much more cycles?
ONESEC		.equ	10
FIVESEC		.equ	50
TENSEC		.equ	100
TWENTYSEC	.equ	200
TENTH		.equ	34000



;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
            .retain                         ; Override ELF conditional linking
;-------------------------------------------------------------------------------
start:
	    mov.w   #__STACK_END,SP         ; init stack pointer
        mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT
        bis.b   #0x08,&P4DIR            ; set P4.3 as output
        bis.b	#0x04,&P4DIR			; set Yellow Light P4.2
        bis.b	#0x02,&P4DIR			; set Orange Light P4.1
        bis.b	#0x01,&P4DIR			; set Green Light P4.0
        bis.b	#0X40,&P4DIR			; set Red 2
        bis.b	#0x10,&P3DIR			; set Green 2

		bic.b	#0x01,&P1SEL          ; select GPIO
		bic.b	#0x01,&P1DIR          ; configure P1.0 as inputs
		bis.b	#0x01,&P1OUT          ; use pull-up
		bis.b	#0x01,&P1REN          ; enable pull-up
		bis.b	#0x01,&P1IES          ; trigger on high to low transition
		bis.b	#0x01,&P1IE           ; P1.0 interrupt enabled
		bic.b	#0x01,&P1IFG          ; P1.0 IFG cleared

		bis.w	#GIE,SR					;enable general interrupts

startingreset:							;clears bits in all outputs
				RED_OFF
				YELLOW_OFF
				ORANGE_OFF
				GREEN_OFF
				RED2_OFF
				GREEN2_OFF



;r15 green
;r14 yellow
;r13 red
;r12 tenth second
;r11 one second
;r10 one digit #'s
mainloop:
		call	#setup				;make the call to sub routine
		jmp 	mainloop			;repeat

setup:
		mov.w	#TENSEC,r15			;green light base
		mov.w	#FIVESEC,r14		;yellow light
		mov.w	#FIVESEC,r13		;red light
		GREEN_ON
		RED2_ON

green:
		call	#tenth				;1/10 sec
		sub.w	#1,r15
		jne		green				;times 100 = 10 sec
		mov.w	#TENSEC,r15			;another 10 sec

green2:
		ORANGE_TEST					;checks orange
		jne		intermediate		;skips to yellow
		call	#tenth				;1/10 sec
		sub.w	#1,r15
		jne		green2				;times 100 = 10 sec

intermediate:
		GREEN_OFF
		YELLOW_ON


yellow:
		call	#tenth				;1/10
		sub.w	#1,r14
		jne		yellow				;times 50 = 5 sec
		YELLOW_OFF
		RED_ON
		ORANGE_TEST					;Checks ORANGE (if statement)
		jeq		red					;GREEN2 off
		ORANGE_OFF
		RED2_OFF
		GREEN2_ON
		jne		red2				;GREEN2 on

red:
		call 	#tenth				;1/10 sec
		sub.w	#1,r13
		jne		red					;times 50 = 5 sec
		RED_OFF
		ret

red2:
		call	#tenth				;1/10 sec
		sub.w	#1,r13
		jne		red2				;times 50 = 5 sec
		mov.w	#SIX,r10

crossdelaysetup:
		mov.w	#ONESEC,r11

cross:
		call	#tenth				;1/10 sec
		sub.w	#1,r11
		jne		cross				;times 10 = 1 sec
		GREEN2_TOGGLE
		sub.w	#1,r10
		jne		crossdelaysetup		;times 6 = 6 sec
		mov.w	#FIVE,r10			;five instead of four to compensate for cycles and sec?
		GREEN2_ON

crossFASTERsetup:
		mov.w	#HALFSEC,r11
crossFASTER2:
		mov.w	#2,r9				;converts 1/10 to 1/5

crossFASTER:
		call	#tenth				;1/10 sec
		sub.w	#1,r9
		jne		crossFASTER			;times 2 = 1/5 sec
		GREEN2_TOGGLE
		sub.w	#1,r11
		jne		crossFASTER2		;times 4 = 4/5 sec		THIS + NEXT WORK IN COMBINATION
		sub.w	#1,r10
		jne		crossFASTERsetup	;times 5 = 4 sec		TO ALLOW 10 BLINKS IN THE 4 SEC INTERVAL
		;PREPARE TO RESTART
		RED_OFF
		GREEN2_OFF
		RED2_ON
		ret

				;TENTH SEC SUB ROUTINE
tenth:
		push.w	r12
		mov.w	#TENTH,r12

tenthloop:
		sub.w	#1,r12
		jne		tenthloop
		pop.w	r12
		ret

				;INTERUPT THAT TURNS ON THE ORANGE LED WHEN BUTTON IS PUSHED
P1_ISR:
		bic.b  #0x01,&P1IFG			; clear P1.0 Interrupt Flag
		ORANGE_ON					; turn on ORANGE LED
		reti

		.sect  ".int02"				; P1 interrupt vector
		.word  P1_ISR

;-------------------------------------------------------------------------------
;           Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect 	.stack

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   start                   ; start address
            .end
