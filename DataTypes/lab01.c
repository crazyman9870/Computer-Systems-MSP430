//	Lab01.c	2014/06/16
#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lab01.h"

unsigned long MCLK_HZ = 1050000;		// SMCLK frequency in Hz
unsigned long BPS = 9600;				// ASYNC serial baud rate

int main(void)
{
    signed char biggest_char, next_char;
	lab01_init();

	// uncomment call to find_baud_rate to find MCLK_HZ
	//find_baud_rate();

	TERMINAL("\n\r** INTEGERS *************");
  	TERMINAL1("Signed int: 21000 + 200 = %d", 21000 + 200);
    next_char = 1;						// start w/1
    biggest_char = 1;
    while((next_char <<= 1) > 0)
    {
    	biggest_char = (biggest_char << 1) + 1;
    }
	TERMINAL2("Largest signed char = %d (0x%2x)", biggest_char);
	TERMINAL3("Largest Q4.4 = %f (0x%2x)", (((long)biggest_char)/16.0), biggest_char);

	float f = 0.01f;
	float sum = 0.0;
	int i;

	for (i = 0; i < 100; ++i) sum += f;
	TERMINAL2("sum = %f (0x%04lx)", sum);
	TERMINAL2("mul = %f (0x%04lx)", f * 100);

	TERMINAL("\n\r** END ******************");
	TERMINAL("\n\nBEGIN STUDENT LAB\n\n");

	unsigned char unsignedC = 1;
	for(i = 7; i > 0; --i) {
		unsignedC = (unsignedC << 1) + 1;
		if(i == 1) {TERMINAL2("Unsigned Max Number = %u (0x%2x)", unsignedC);}
	}
	++unsignedC;
	TERMINAL2("Unsigned Min/Overflow Number = %u (0x%02x)\n", unsignedC);

	signed char signedC = 1;
	for(i = 6; i > 0; --i) {
		signedC = (signedC << 1) + 1;
		if(i == 1) {TERMINAL2("Signed Max Number = %d (0x%2x)", signedC);}
//		TERMINAL1("CHARS\nSigned Max Number = %d", signedC);
	}
	++signedC;
	TERMINAL2("Signed Min Number = %d (0x%2x)\n", signedC);


//--------------------------------------------------------------------
	unsigned int testUnsigned = 1;
	for(i = 15; i > 0; --i) {
		testUnsigned = (testUnsigned << 1) + 1;
		if(i == 1) {TERMINAL2("Unsigned Max Number = %u (0x%4x)", testUnsigned);}
	}
	++testUnsigned;
	TERMINAL2("Unsigned Min/Overflow Number = %u (0x%04x)\n", testUnsigned);


	int testSigned = 1;
	for(i = 14; i > 0; --i) {
		testSigned = (testSigned << 1) + 1;
		if(i == 1) {TERMINAL2("Signed Max Number = %d (0x%4x)", testSigned);}
	}
	++testSigned;
	TERMINAL2("Signed Min Number = %d (0x%4x)\n", testSigned);
//----------------------------------------------------------------------
	unsigned long testUnsignedLong = 1;
	for(i = 31; i > 0; --i) {
		testUnsignedLong = (testUnsignedLong << 1) + 1;
		if(i == 1) {TERMINAL2("Unsigned Max Number = %lu (0x%04lx)", testUnsignedLong);}
	}
	++testUnsignedLong;
	TERMINAL2("Unsigned Min/Overflow Number = %lu (0x%04lx)\n", testUnsignedLong);

	long testSignedLong = 1;
	for(i = 30; i > 0; --i) {
		testSignedLong = (testSignedLong << 1) + 1;
		if(i == 1) {TERMINAL2("Signed Max Number = %ld (0x%04lx)", testSignedLong);}
	}
	++testSignedLong;
	TERMINAL2("Signed Min Number = %ld (0x%04lx)\n", testSignedLong);
	//-------------------------------------------------------------------
	//Fixed Max and Min

	long fixed1 = 1;
	for(i = 30; i > 0; --i) {
		fixed1 = (fixed1 << 1) + 1;
	}
	TERMINAL3("Max fixed value = %f (0x%4lx)\n", (fixed1/(16.0*16.0*16.0*16.0)),fixed1);
	//-------------------------------------------------------------------
	//Floating point round off errors

	float f1 = 0.02;
	float f2 = 0.02;

	for(i = 100; i > 1; --i) {
		f1 += 0.02;
	}
//	f2 = f2 * 100;

	TERMINAL2("Sum of float %f (0x%04lx)", f1);
	TERMINAL2("Multiplication of float %f (0x%04lx)\n", f2 * 100);
	//--------------------------------------------------------------------
	//Associative property

	f1 = (0.00005 + 5) - 5;
	f2 = 0.00005 + (5 - 5);

	TERMINAL2("Sum of float %f (0x%04lx)", f1);
	TERMINAL2("Multiplication of float %f (0x%04lx)", f2);


    return 0;
}
