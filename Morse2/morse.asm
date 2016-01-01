			.title	"morse.asm"
;*******************************************************************************
;     Project:  morse.asm
;      Author:  Student Code
;
; Description:  Outputs a message in Morse Code using a LED and a transducer
;               (speaker).  The watchdog is configured as an interval timer.
;               The watchdog interrupt service routine (ISR) toggles the green
;               LED every second and pulse width modulates (PWM) the speaker
;               such that a tone is produced.
;
;	Morse code is composed of dashes and dots:
;
;        1. A dot is equal to an element of time.
;        2. One dash is equal to three dots.
;        3. The space between parts of the letter is equal to one dot.
;        4. The space between two letters is equal to three dots.
;        5. The space between two words is equal to seven dots.
;
;    5 WPM = 60 sec / (5 * 50) elements = 240 milliseconds per element.
;    element = (WDT_IPS * 6 / WPM) / 5
;
;******************************************************************************
; System equates --------------------------------------------------------------
            .cdecls C,"msp430.h"            ; include c header
            .cdecls C,"morse2.h"

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

	        .asg   "bis.b #0x10,&P4OUT",LED_ON
	        .asg   "bic.b #0x10,&P4OUT",LED_OFF
	        .asg   "xor.b #0x10,&P4OUT",LED_TOGGLE
	        .asg   "bit.b #0x10,&P4OUT",LED_TEST

;myCLOCK     .equ    1200000                 ; 1.2 Mhz clock
;WDT_CTL     .equ    WDT_MDLY_0_5            ; WD: Timer, SMCLK, 0.5 ms
;WDT_CPI     .equ    500                     ; WDT Clocks Per Interrupt (@1 Mhz)
;WDT_IPS     .equ    (myCLOCK/WDT_CPI)       ; WDT Interrupts Per Second
;STACK       .equ    0x0600                  ; top of stack

; Morse Code equates ----------------------------------------------------------
;END         .equ    0
;DOT         .equ    1
;DASH        .equ    2
;ELEMENT     .equ    ((WDT_IPS*240)/1000)    ; (WDT_IPS * 6 / WPM) / 5
;DEBOUNCE	.equ	10

; External references ---------------------------------------------------------
            .ref    numbers                 ; codes for 0-9
            .ref    letters                 ; codes for A-Z
            .def	beep
            .def	delay
            .def    beep_cnt
            .def    delay_cnt
            .def	green2cnt
            .def	debounce
                            
; Global variables ------------------------------------------------------------
            .bss    beep_cnt,2              ; beeper flag
            .bss    delay_cnt,2             ; delay flag
            .bss	green2cnt,2				;used to time second intervals
            .bss	debounce,2			;turn on and off sound

; Program section -------------------------------------------------------------
            .text                           ; program section
message:    .string "HELLO CS 124 WORLD"	; PARIS message
            .byte   0
            .align  2                       ; align on word boundary

; power-up reset --------------------------------------------------------------
;RESET:      mov.w   #STACK,SP               ; initialize stack pointer
;            call    #main_asm               ; call main function
;            jmp     $                       ; you should never get here!

; start main function vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv	
            .def	main_asm

main_asm:   mov.w   #WDT_CTL,&WDTCTL        ; set WD timer interval
            mov.b   #WDTIE,&IE1             ; enable WDT interrupt
            bis.b   #0x60,&P4DIR            ; set P4.5 as output (speaker) + red2 (was 0x20 changed to include red)
            bis.b	#0x1f,&P4DIR			;enable the rest of the lights
            bis.b	#0x10,&P3DIR			;green2 enabled
            clr.w   &beep_cnt               ; clear counters
            clr.w   &delay_cnt

            mov.w	#WDT_IPS,&green2cnt
            clr.w	&debounce

            bic.b	#0x0f,&P1SEL          ; select GPIO
			bic.b	#0x0f,&P1DIR          ; configure P1.0 as inputs
			bis.b	#0x0f,&P1OUT          ; use pull-up
			bis.b	#0x0f,&P1REN          ; enable pull-up
			bis.b	#0x0f,&P1IES          ; trigger on high to low transition
			bis.b	#0x0f,&P1IE           ; P1.0 interrupt enabled
			bic.b	#0x0f,&P1IFG          ; P1.0 IFG cleared

            bis.w   #GIE,SR                 ; enable interrupts

			RED_OFF
			YELLOW_OFF
			ORANGE_OFF
			GREEN_OFF
			GREEN2_OFF
			RED2_OFF
			LED_OFF


; output 'A' in morse code (DOT, DASH, space)
;loop:       mov.w   #ELEMENT,r15            ; output DOT
;            call    #beep
;            mov.w   #ELEMENT,r15            ; delay 1 element
;            call    #delay

;            mov.w   #ELEMENT*3,r15          ; output DASH
;            call    #beep
;            mov.w   #ELEMENT,r15            ; delay 1 element
;           call    #delay

;            mov.w   #ELEMENT*7,r15          ; output space
;           call    #delay                  ; delay
;            jmp     loop                    ; repeat


loop:		mov.w	#message,r4            ; point to message

loop02:		mov.b	@r4+,r5                ; get character
			cmp.b	#0,r5
			jeq		loop
			cmp.b	#32,r5
			jeq		space
			cmp.b	#58,r5
			jlo		number
			cmp.b	#64,r5
			jge		letter

letter:		sub.w	#'A',r5                ; make index 0-25
			add.w	r5,r5                  ; make word index
			mov.w	letters(r5),r5         ; get pointer to letter codes
			jmp		loop10

number:		sub.w	#48,r5					;make index 0-9
			add.w	r5,r5					;make number index
			mov.w	numbers(r5),r5			;get pointer to number codes
			jmp loop10

space:		call	#doSpace
			jmp		loop02

loop10:		mov.b	@r5+,r6                ; get DOT, DASH, or END

			cmp.b	#DOT,r6                ; dot?
			jne		dash
			call	#doDot
			jmp		loop10

dash:		cmp.b	#DASH,r6
			jne		end
			call	#doDash
			jmp		loop10

end:		cmp.b	#END,r6
			jeq		loop02
			call	#doSpace
			jmp		loop10
; end main function ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

;doBeep & doDash sub routines

doDot:		mov.w   #ELEMENT,r12            ; output DOT
			call    #beep
			mov.w   #ELEMENT,r12            ; delay 1 element
			call    #delay
			ret

doDash:		mov.w   #ELEMENT*3,r12          ; output DASH
			call    #beep
			mov.w   #ELEMENT,r12            ; delay 1 element
			call    #delay
			ret

doSpace:	mov.w   #ELEMENT*7,r12          ; output space
			call    #delay                  ; delay
			ret

; beep (r15) ticks subroutine -------------------------------------------------
beep:       push	r12
			mov.w   r12,&beep_cnt           ; start beep
			pop		r12

beep02:     tst.w   &beep_cnt               ; beep finished?
              jne   beep02                  ; n
            ret                             ; y


; delay (r15) ticks subroutine ------------------------------------------------
delay:      push	r12
			mov.w   r12,&delay_cnt          ; start delay
			pop		r12

delay02:    tst.w   &delay_cnt              ; delay done?
              jne   delay02                 ; n
            ret                             ; y


; Watchdog Timer ISR ----------------------------------------------------------
WDT_ISR:    tst.w   &beep_cnt               ; beep on?
			jeq   WDT_02					; n
            dec.w   &beep_cnt               ; y, decrement count
			xor.b 	#0x40,&P4OUT
			xor.b   #0x20,&P4OUT            ; beep using 50% PWM

WDT_02:     tst.w   &delay_cnt              ; delay?
              jeq   WDT_03                  ; n
            dec.w   &delay_cnt              ; y, decrement count

WDT_03:		tst.w	&green2cnt
			jeq		sectoggle
			dec.w	&green2cnt

WDT_04:		tst.w	&debounce				;pressed?
			jeq		WDT_10

			dec.w	&debounce
			jne		WDT_10					;-1 leads to done?

			push	r15
			mov.b	&P1IN,r15
			and.b	#0x0f,r15
			xor.b	#0x0f,r15
			jeq		WDT_09
			xor.b	#0x20,&P4DIR

WDT_09:		pop		r15
WDT_10:     reti                            ; return from interrupt

sectoggle:	GREEN2_TOGGLE
			mov.w	#WDT_IPS,&green2cnt
			jmp		WDT_04

; Other Interrupts   ----------------------------------------------------------

P1_ISR:		GREEN_TOGGLE
			bic.b	#0x0f,&P1IFG
			mov.w	#DEBOUNCE,&debounce
			reti

; Interrupt Vectors -----------------------------------------------------------
            .sect   ".int10"                ; Watchdog Vector
            .word   WDT_ISR                 ; Watchdog ISR

;			.sect	".int02"
;			.word	P1_ISR

;            .sect   ".reset"                ; PUC Vector
;            .word   RESET                   ; RESET ISR
            .end
