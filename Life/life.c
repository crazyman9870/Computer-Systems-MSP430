//	life.c	03/23/2015
//******************************************************************************
//  The Game of Life
//
//  Lab Description:
//
//  The universe of the Game of Life is an infinite two-dimensional orthogonal
//  grid of square cells, each of which is in one of two states, alive or dead.
//  With each new generation, every cell interacts with its eight neighbors,
//  which are the cells horizontally, vertically, or diagonally adjacent
//  according to the following rules:
//
//  1. A live cell stays alive (survives) if it has 2 or 3 live neighbors,
//     otherwise it dies.
//  2. A dead cell comes to life (birth) if it has exactly 3 live neighbors,
//     otherwise it stays dead.
//
//  An initial set of patterns constitutes the seed of the simulation. Each
//  successive generation is created by applying the above rules simultaneously
//  to every cell in the current generation (ie. births and deaths occur
//  simultaneously.)  See http://en.wikipedia.org/wiki/Conway's_Game_of_Life
//
//  Author:    Paul Roper, Brigham Young University
//  Revisions: June 2013   Original code
//             07/12/2013  life_pr, life_cr, life_nr added
//             07/23/2013  generations/seconds added
//             07/29/2013  100 second club check
//             12/12/2013  SWITCHES, display_results, init for port1 & WD
//	           03/24/2014  init_life moved to lifelib.c, 0x80 shift mask
//	                       blinker added, 2x loops
//             03/23/2015  start_generation() added, display_results(void)
//
//  Built with Code Composer Studio Version: 5.5.0.00090
//******************************************************************************
//  Lab hints:
//
//  The life grid (uint8 life[80][10]) is an 80 row x 80 column bit array.  A 0
//  bit is a dead cell while a 1 bit is a live cell.  The outer cells are always
//  dead.  A boolean cell value (0 or non-zero) is referenced by:
//
//         life[row][col >> 3] & (0x80 >> (col & 0x07))
//
//  Each life cell maps to a 2x2 lcd pixel.
//
//                     00       01             08       09
//  life[79][0-9]   00000000 00000000  ...  00000000 00000000 --> life_pr[0-9]
//  life[78][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_cr[0-9]
//  life[77][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_nr[0-9]
//  life[76][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0         |
//     ...                                                            |
//  life[75-4][0-9]   ...      ...            ...      ...            v
//     ...
//  life[03][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[02][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[01][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[00][0-9]   00000000 00000000  ...  00000000 00000000
//
//  The next generation can be made directly in the life array if the previous
//  cell values are held in the life_pr (previous row), life_cr (current row),
//  and life_nr (next row) arrays and used to count cell neighbors.
//
//  Begin each new row by moving life_cr values to life_pr, life_nr values to
//  life_cr, and loading life_nr with the row-1 life values.  Then for each
//  column, use these saved values in life_pr, life_cr, and life_nr to
//  calculate the number of cell neighbors of the current row and make changes
//  directly in the life array.
//
//  life_pr[0-9] = life_cr[0-9]
//  life_cr[0-9] = life_nr[0-9]
//  life_nr[0-9] = life[row-1][0-9]
//
//******************************************************************************
//******************************************************************************
// includes --------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include "msp430.h"
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "life.h"
#include "lifelib.h"
#include <cstring>
#include <cctype>

extern volatile uint16 switches;		// debounced switch values
extern const uint16 life_image[];

// global variables ------------------------------------------------------------
uint8 life[NUM_ROWS][NUM_COLS/8];		// 80 x 80 life grid
uint8 life_pr[NUM_COLS/8];				// previous row
uint8 life_cr[NUM_COLS/8];				// current row
uint8 life_nr[NUM_COLS/8];				// next row

//------------------------------------------------------------------------------
//	draw RLE pattern -----------------------------------------------------------
void draw_rle_pattern(int row, int col, const uint8* object)
{
	char* ptr = (char*)object;
	int y = 0;

	while(*ptr++ != 'y');
	while(!isdigit(*ptr)) ++ptr;
	while(isdigit(*ptr))
		y = y * 10 + (*ptr++ - '0');
	while(*ptr++ != '\n');

	int number = 0;
	int myrow = row + y - 1;
	int mycol = col;
	int i;

	while(*ptr != '!') {
		if(isdigit(*ptr)) {
			number = number * 10 + (*ptr - '0');
		}
		if(*ptr == 'o') {
			if(number == 0) {
				number++;
			}
			for(i = number; i > 0; --i) {
				CELLBIRTH(life, myrow, mycol);
				BIRTH(myrow,mycol);
				mycol++;
			}
			number = 0;
		}
		if(*ptr == 'b') {
			if(number == 0) number++;
			for(i = number; i > 0; --i) {
				mycol++;
			}
			number = 0;
		}
		if(*ptr == '$') {
			mycol = col;
			if(number == 0) number++;
			for(i = number; i > 0; --i) {
				myrow--;
			}
			number = 0;
		}
		ptr++;
	}
	return;
}


//------------------------------------------------------------------------------
// main ------------------------------------------------------------------------
void main(void)
{
	RBX430_init(CLOCK);					// init board
	ERROR2(lcd_init());					// init LCD
	//lcd_volume(376);					// ***increase LCD brightness(if necessary)***
	watchdog_init();					// init watchdog
	port1_init();						// init P1.0-3 switches
	__bis_SR_register(GIE);				// enable interrupts


	// >>>>>>>> DELETE vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	lcd_clear();						// clear LCD
	memset(life, 0, sizeof(life));		// clear life array
	lcd_backlight(ON);					// turn on LCD backlight
	lcd_rectangle(0, 0, NUM_COLS*2, NUM_ROWS*2, 1);
	lcd_wordImage(life_image, 17, 50, 1);
	lcd_cursor(10, 20);
	printf("\b\tPress Any Key");
	switches = 0;						// clear switches flag
	// >>>>>>>> DELETE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	while (1)							// new pattern seed
	{
		uint16 row, col;
		while (!switches);					// wait for any switch

		if(switches) {
			init_life(switches);
			switches = 0;
		}


		while (1)						// next generation
		{
			memcpy(life_nr, life[NUM_ROWS - 3], sizeof(life_cr));
			memcpy(life_cr, life[NUM_ROWS - 2], sizeof(life_cr));
			memset(life_pr, 0, sizeof(life_pr));

			for(row = NUM_ROWS - 2; row; --row) {

				int neighbors = 0;
				for(col = NUM_COLS - 2; col; --col) {


					//if(life_nr[col/8] || life_cr[col/8]) {

						if(TESTCELL(life_cr, (col + 1))) neighbors++;
						if(TESTCELL(life_cr, (col - 1))) neighbors++;
						if(TESTCELL(life_nr, (col + 1))) neighbors++;
						if(TESTCELL(life_nr, (col))) neighbors++;
						if(TESTCELL(life_nr, (col - 1))) neighbors++;
						if(TESTCELL(life_pr, (col + 1))) neighbors++;
						if(TESTCELL(life_pr, (col))) neighbors++;
						if(TESTCELL(life_pr, (col - 1))) neighbors++;


						if(life[row][col >> 3] & (0x80 >> (col & 0x07))) {
							if(neighbors < 2 || neighbors > 3) {
								CELLDEATH(life, row, col);
								DEATH(row,col);
							}
						}
						else {
							if(neighbors == 3) {
								CELLBIRTH(life, row, col);
								BIRTH(row, col);
							}
						}
						neighbors = 0;
//					}
//					else {
//						col - (col % 8 + 1);
//					}
				}

				memcpy(life_pr, life_cr, sizeof(life_cr));
				memcpy(life_cr, life_nr, sizeof(life_cr));
				memcpy(life_nr, life[row - 2], sizeof(life_cr));

			}

			// display life generation and generations/second on LCD
			if (display_results()) break;
		}
	}
} // end main()
