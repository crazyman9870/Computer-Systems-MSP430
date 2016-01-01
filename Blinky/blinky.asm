
;This is the work of Anthony Constantino 12 Feb 2015

;*******************************************************************************
;   CS/ECEn 124 Lab 3 - blinky.asm: Software Toggle P1.0
;
;   Description: Quickly blink P1.0 every 10 seconds.  Calculate MCLK, CPI, MIPS
;        Author:
;
;             MSP430G5223
;             -----------
;            |       P1.0|-->LED1-RED LED
;            |       P1.3|<--S2
;            |       P1.6|-->LED2-GREEN LED
;
; Show all calculations:
;
;   MCLK = __10.81___ cycles / __10___ interval = ___1.08__ Mhz
;    CPI = __23___ cycles/ __16__ instructions = __1.4375__ Cycles/Instruction
;   MIPS = MCLK / CPI / 1000000 = __.752227__ MIPS

; max int number 65535
; 65535subtractions&jumps * 55times * 3cycles = 10813275 = 10.813275 cycles * 10^6
; approx 10.81 / 10 seconds = 1.08 Mhz
; mov of count into r15 to the end is 23 cycles, and that is 16 instructions
;
;*******************************************************************************
           .cdecls  C,"msp430.h"            ; MSP430

COUNT      .equ     0                     ; delay count
REPEAT	   .equ		55
;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
;------------------------------------------------------------------------------
start:      mov.w   #0x0280,SP              ;    init stack pointer
            mov.w   #WDTPW|WDTHOLD,&WDTCTL  ;    stop WDT
            bis.b   #0x40,&P1DIR            ;    set P1.0 as output

setup:		xor.b   #0x40,&P1OUT			; 5 cycles

mainloop:   xor.b   #0x40,&P1OUT            ; 5 cycles
			mov.w	#COUNT,r12				; 1 cycles
			jmp		flash					; 2 cycles

mainloop2:  xor.b   #0x40,&P1OUT			; cycles
			mov.w   #COUNT,r15              ; 1 cycles
            mov.w	#REPEAT,r14				; 1 cycles
            jmp		x5						; 2 cycles

flash:		sub.w	#1,r12					; 1 cycles
			jne		flash					; 2 cycles
			jmp		mainloop2				; 2 cycles

x5:			mov.w	#COUNT,r13				; 1 cycles
			sub.w	#1,r14					; 1 cycles
			jne		count					; 2 cycles
			jmp		mainloop				; 2 cycles

count:		sub.w	#1,r13					; 1 cycles
			jne		count					; 2 cycles
			jmp		x5						; 2 cycles

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   start                   ; start address
            .end
