//	life.h	03/23/2015
//*******************************************************************************

#ifndef LIFE_H_
#define LIFE_H_

#define myCLOCK			19200000			// 1.2 Mhz clock
#define CLOCK			_16MHZ

#define	MASK(col)					(0x80 >> ((col) & 0x07))
#define	CELLDEATH(a2d,row,col)		a2d[(row)][(col) >> 3] &= ~MASK(col)
#define CELLBIRTH(a2d,row,col)		a2d[(row)][(col) >> 3] |= MASK(col)
#define TESTCELL(a2d,col)			a2d[(col) >> 3] & MASK(col)
#define BIRTH(row,col)				lcd_point(col << 1, row << 1, 7)
#define DEATH(row,col)				lcd_point(col << 1, row << 1, 6)


#endif /* LIFE_H_ */
