//******************************************************************************
//	snake_events.c
//
//  Author:			Paul Roper, Brigham Young University
//  Revisions:		1.0		11/25/2012	RBX430-1
//
//******************************************************************************
//
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "snake.h"
#include "snakelib.h"


extern volatile uint16 sys_event;			// pending events
extern volatile uint16 move_cnt;			// snake speed

extern volatile enum modes game_mode;		// 0=idle, 1=play, 2=next
extern volatile uint16 score;				// current score
extern volatile uint16 level;				// current level (1-4)
extern volatile uint16 direction;			// current move direction

extern volatile uint8 head;					// head index into snake array
extern volatile uint8 tail;					// tail index into snake array
extern SNAKE snake[MAX_SNAKE];				// snake segments
FOOD* rock[2]; // This stores the rocks
int food_hit; //boolean
int self_hit; //boolean
int rock_hit; //boolean
FOOD* food[10];
int food_eaten; // total number of food eaten on the level
int snake_size;
int cur_time; // current time
int timer_up; //boolean
int winner; //boolean
int cur_score;
//POINT points[10];

extern const uint16 snake_text_image[];
extern const uint16 snake1_image[];
extern const uint16 king_snake_image[];

static uint8 move_right(SNAKE* head);		// move snake head right
static uint8 move_up(SNAKE* head);			// move snake head up
static uint8 move_left(SNAKE* head);		// move snake head left
static uint8 move_down(SNAKE* head);		// move snake head down
static void new_snake(uint16 length, uint8 dir);
static void delete_tail(void);
static void add_head(void);


//-- switch #1 event -----------------------------------------------------------
//
void SWITCH_1_event(void)
{
	switch (game_mode)
	{
		case IDLE:
			sys_event |= START_LEVEL;
			level = 1;
			break;

		case PLAY:
			if (direction != LEFT)
			{
				if (snake[head].point.x < X_MAX)
				{
					direction = RIGHT;
					sys_event |= MOVE_SNAKE;
				}
			}
			break;

		case NEXT:
			sys_event |= NEXT_LEVEL;
			break;
		case END:
			sys_event |= NEW_GAME;
			break;
	}
	return;
} // end SWITCH_1_event


//-- switch #2 event -----------------------------------------------------------
//
void SWITCH_2_event(void)
{
	switch (game_mode)
	{
		case IDLE:
			break;

		case PLAY:
			if (direction != UP)
			{
				if (snake[head].point.x < X_MAX)
				{
					direction = DOWN;
					sys_event |= MOVE_SNAKE;
				}
			}
			break;

		case NEXT:
			sys_event |= NEXT_LEVEL;
			break;
		case END:
			sys_event |= NEW_GAME;
			break;
	}
	return;

} // end SWITCH_2_event


//-- switch #3 event -----------------------------------------------------------
//
void SWITCH_3_event(void)
{
	switch (game_mode)
		{
			case IDLE:
				break;

			case PLAY:
				if (direction != DOWN)
				{
					if (snake[head].point.x < X_MAX)
					{
						direction = UP;
						sys_event |= MOVE_SNAKE;
					}
				}
				break;

			case NEXT:
				sys_event |= NEXT_LEVEL;
				break;
			case END:
				sys_event |= NEW_GAME;
				break;
		}
		return;
} // end SWITCH_3_event


//-- switch #4 event -----------------------------------------------------------
//
void SWITCH_4_event(void)
{
	switch (game_mode)
	{
		case IDLE:
			break;

		case PLAY:
			if (direction != RIGHT)
			{
				if (snake[head].point.x < X_MAX)
				{
					direction = LEFT;
					sys_event |= MOVE_SNAKE;
				}
			}
			break;

		case NEXT:
			sys_event |= NEXT_LEVEL;
			break;
		case END:
			sys_event |= NEW_GAME;
			break;
	}
	return;
} // end SWITCH_4_event


//-- next level event -----------------------------------------------------------
//
void NEXT_LEVEL_event(void)
{
	food_eaten = 0;
	food_hit = 0;
	self_hit = 0;
	rock_hit = 0;
	level++;
	int i = 0;
	for(i = 0; i < 10; i++) {
		free(food[i]);
		food[i] = NULL;
	}
	for(i = 0; i < 2; i++) {
		if(rock[i] == NULL) break;
		free(rock[i]);
		rock[i] = NULL;
	}

	sys_event |= START_LEVEL;
}


//-- update LCD event -----------------------------------------------------------
//
void LCD_UPDATE_event(void)
{
	lcd_cursor(0, 0);
	lcd_rectangle(7, 10, 146, 140, 1);
	lcd_cursor(7, 0);
	lcd_printf("LEFT   UP   DOWN   RIGHT");
	lcd_cursor(7, 150);
	if(cur_time > 9) {
		lcd_printf("Score %d   Level %d   0:%d", cur_score, level, cur_time);
	}
	else {
		lcd_printf("Score %d   Level %d   0:0%d", cur_score, level, cur_time);
	}
	lcd_cursor(0, 0);
} // end LCD_UPDATE_event


//-- end game event -------------------------------------------------------------
//
void END_GAME_event(void)
{
	int i;
	for(i = 0; i < 10; i++) {
		free(food[i]);
	}
	for(i = 0; i < 2; i++) {
		if(rock[i] == NULL) break;
		free(rock[i]);
	}
	//free(rock[0]);
	cur_time = -1;
	timer_up = 0;
	snake_size = 0;
	food_hit = 0;
	level = 0;
	food_eaten = 0;
	game_mode = END;
	if(winner) {
		lcd_clear();
		lcd_cursor(0, 0);
		lcd_wordImage(king_snake_image, 150/4, 160/4, 1);
		lcd_rectangle(7, 0, 146, 40, 17);
		lcd_mode(6);
		lcd_cursor(((7+146)/4)-5, 20);
		lcd_printf("SCORE:%d", cur_score);
		lcd_mode(0);
		lcd_cursor(0, 0);
		imperial_march();
	}
	else {
		lcd_rectangle(7, 40, 146, 80, 17);
		lcd_mode(6);
		lcd_cursor(((7+146)/4)-5, 80);
		lcd_printf("SCORE:%d", cur_score);
		lcd_mode(0);
		lcd_cursor(0, 0);
	}


} // end END_GAME_event


//-- move snake event ----------------------------------------------------------
//
void MOVE_SNAKE_event(void)
{
	if (level > 0)
	{
		add_head();						// add head
		lcd_point(COL(snake[head].point.x), ROW(snake[head].point.y), PENTX);

		//check for collision

		if(timer_up) {
			timer_up = 0;
			rasberry();
			sys_event = END_GAME;
			return;
		}
        self_col();

        if (level > 1) rock_col();
        if(self_hit||rock_hit) {
            self_hit=0;
            rock_hit=0;
            sys_event = END_GAME;
            return;
        }
		food_col();
		if(food_hit) {
			food_hit = 0;
			if(food_eaten == 2) {
				if(level == 4) {
					winner = 1;
					sys_event = END_GAME;
					return;
				}
				game_mode = NEXT;
				charge();
			}
			return;
		}
		else {
			delete_tail();					// delete tail
		}

	}
	return;
} // end MOVE_SNAKE_event


//-- start level event -----------------------------------------------------------
//
void START_LEVEL_event(void)
{
	if(level == 1) {
		snake_size = 2;
		lcd_clear();
		new_snake(snake_size,UP);
		cur_time = TIME_1_LIMIT;
		food_hit = 0;
		self_hit = 0;
		rock_hit = 0;
		cur_score = 0;
		LCD_UPDATE_event();
		move_cnt = WDT_MOVE1;	//This changes speed based on the level
		fill_food(10, LEVEL_1_VALUE);
	}
	if(level == 2) {
		lcd_clear();
		new_snake(snake_size,UP);
		cur_time = TIME_2_LIMIT;
		LCD_UPDATE_event();
		move_cnt = WDT_MOVE2;
		fill_rocks();
		fill_food(10, LEVEL_2_VALUE);
	}
	if(level == 3) {
		lcd_clear();
		new_snake(snake_size,UP);
		cur_time = TIME_3_LIMIT;
		LCD_UPDATE_event();
		move_cnt = WDT_MOVE3;
		fill_rocks();
		fill_food(1, LEVEL_3_VALUE);
	}
	if(level == 4) {
		lcd_clear();
		new_snake(snake_size,UP);
		cur_time = TIME_4_LIMIT;
		LCD_UPDATE_event();
		move_cnt = WDT_MOVE4;
		fill_rocks();
		fill_food(1, LEVEL_4_VALUE);
	}

	//	Add code here to setup playing board for next level
	//	Draw snake, foods, reset timer, etc
	game_mode = PLAY;					// start level
	return;
} // end START_LEVEL_event


//-- new game event ------------------------------------------------------------
//
void NEW_GAME_event(void)
{
	lcd_clear();						// clear lcd
	lcd_backlight(1);					// turn on backlight
	level = 0;
	food_eaten = 0;
	cur_time = -1;
	timer_up = 0;
	cur_score = 0;
	winner = 0;
	int i;
	for(i = 0; i < 10; i++) {
		food[i] = NULL;
	}
	for(i = 0; i < 2; i++) {
		rock[i] = NULL;
	}
	lcd_wordImage(snake1_image, (159-60)/2, 60, 1);
	lcd_wordImage(snake_text_image, (159-111)/2, 20, 1);

	game_mode = IDLE;
	return;
} // end NEW_GAME_event


//-- new snake -----------------------------------------------------------------
//
void new_snake(uint16 length, uint8 dir)
{
	int i;
	head = 0;
	tail = 0;
	snake[head].point.x = 5;
	snake[head].point.y = 5;
	direction = dir;

	// build snake
	score = length;
	for (i = score - 1; i > 0; --i)
	{
		add_head();
	}
	return;
} // end new_snake


//-- delete_tail  --------------------------------------------------------------
//
void delete_tail(void)
{
	lcd_point(COL(snake[tail].point.x), ROW(snake[tail].point.y), PENTX_OFF);
	tail = (tail + 1) & (~MAX_SNAKE);
} // end delete_tail


//-- add_head  -----------------------------------------------------------------
//
void add_head(void)
{
	static uint8 (*mFuncs[])(SNAKE*) =	// move head function pointers
	             { move_right, move_up, move_left, move_down };
	uint8 new_head = (head + 1) & (~MAX_SNAKE);
	snake[new_head] = snake[head];		// set new head to previous head
	head = new_head;
	// iterate until valid move
	while ((*mFuncs[direction])(&snake[head]));
} // end add_head


//-- move snake head right -----------------------------------------------------
//
uint8 move_right(SNAKE* head)
{
	if ((head->point.x + 1) < X_MAX)		// room to move right?
	{
		++(head->point.x);					// y, move right
		return FALSE;
	}
	if (level != 2)							// n, right fence
	{
		if (level > 2) sys_event = END_GAME;
		head->point.x = 0;					// wrap around
		return FALSE;
	}
	if (head->point.y) direction = DOWN;	// move up/down
	else direction = UP;
	return TRUE;
} // end move_right


//-- move snake head up --------------------------------------------------------
//
uint8 move_up(SNAKE* head)
{
	if ((head->point.y + 1) < Y_MAX)
	{
		++(head->point.y);					// move up
		return FALSE;
	}
	if (level != 2)							// top fence
	{
		if (level > 2) sys_event = END_GAME;
		head->point.y = 0;					// wrap around
		return FALSE;
	}
	if (head->point.x) direction = LEFT;	// move left/right
	else direction = RIGHT;
	return TRUE;
} // end move_up


//-- move snake head left ------------------------------------------------------
//
uint8 move_left(SNAKE* head)
{
	if (head->point.x)
	{
		--(head->point.x);					// move left
		return FALSE;
	}
	if (level != 2)							// left fence
	{
		if (level > 2) sys_event = END_GAME;
		head->point.x = X_MAX - 1;			// wrap around
		return FALSE;
	}
	if (head->point.y) direction = DOWN;	// move down/up
	else direction = UP;
	return TRUE;
} // end move_left


//-- move snake head down ------------------------------------------------------
//
uint8 move_down(SNAKE* head)
{
	if (head->point.y)
	{
		--(head->point.y);					// move down
		return FALSE;
	}
	if (level != 2)							// bottom fence
	{
		if (level > 2) sys_event = END_GAME;
		head->point.y = Y_MAX - 1;			// wrap around
		return FALSE;
	}
	if (head->point.x) direction = LEFT;	// move left/right
	else direction = RIGHT;
	return TRUE;
}// end move_down

FOOD* newFood(uint8 x, uint8 y, uint8 value, uint8 size, void (*draw)(FOOD*))
{  FOOD* food = (FOOD*)malloc(sizeof(FOOD));
   if (!food) ERROR2(SYS_ERR_MALLOC);
   food->point.x = x;
   food->point.y = y;
   food->value = value;
   food->size = size;
   food->draw = draw;
   (food->draw)(food);
   return food;
}

void draw_diamond_food(FOOD* food)
{  lcd_diamond(COL(food->point.x), ROW(food->point.y),
              food->size, SINGLE);
}

void draw_star_food(FOOD* food)
{  lcd_star(COL(food->point.x), ROW(food->point.y),
              food->size, SINGLE);
}

void draw_circle_food(FOOD* food)
{  lcd_circle(COL(food->point.x), ROW(food->point.y),
              food->size, SINGLE);
}

void draw_square_food(FOOD* food)
{  lcd_square(COL(food->point.x), ROW(food->point.y),
              food->size, SINGLE);
}

void draw_triangle_food(FOOD* food)
{  lcd_triangle(COL(food->point.x), ROW(food->point.y),
              food->size, SINGLE);
}

FOOD* replace_food(int n) {
	FOOD* f;
	int x;
	int y;
	if(n == 0) {
		x = rand() % X_MAX;
		y = rand() % Y_MAX;
		x = new_food_check(x,y);
		f = newFood(x, y, level, 2, draw_square_food);
		return f;
	}
	if(n == 1) {
		x = rand() % X_MAX;
		y = rand() % Y_MAX;
		x = new_food_check(x,y);
		f = newFood(x, y, level, 2, draw_triangle_food);
		return f;
	}
	if(n == 2) {
		x = rand() % X_MAX;
		y = rand() % Y_MAX;
		x = new_food_check(x,y);
		f = newFood(x, y, level, 2, draw_circle_food);
		return f;
	}
	if(n == 3) {
		x = rand() % X_MAX;
		y = rand() % Y_MAX;
		x = new_food_check(x,y);
		f = newFood(x, y, level, 2, draw_star_food);
		return f;
	}
	if(n == 4) {
		x = rand() % X_MAX;
		y = rand() % Y_MAX;
		x = new_food_check(x,y);
		f = newFood(x, y, level, 2, draw_diamond_food);
		return f;
	}
	return NULL;
}

void fill_food(int n, uint8 val)
{
	unsigned int i;
	unsigned int mod;
	int x;
	int y;
	FOOD* cur_Food;
	for(i = 0; i < n; i++) {
		mod = rand() % 5;
		if(mod == 0) {
			x = rand() % X_MAX;
			y = rand() % Y_MAX;
			x = new_food_check(x,y);
			cur_Food = newFood(x, y, val, 2, draw_square_food);
			food[i] = cur_Food;
		}
		if(mod == 1) {
			x = rand() % X_MAX;
			y = rand() % Y_MAX;
			x = new_food_check(x,y);
			cur_Food = newFood(x, y, val, 2, draw_triangle_food);
			food[i] = cur_Food;
		}
		if(mod == 2) {
			x = rand() % X_MAX;
			y = rand() % Y_MAX;
			x = new_food_check(x,y);
			cur_Food = newFood(x, y, val, 2, draw_circle_food);
			food[i] = cur_Food;
		}
		if(mod == 3) {
			x = rand() % X_MAX;
			y = rand() % Y_MAX;
			x = new_food_check(x,y);
			cur_Food = newFood(x, y, val, 2, draw_star_food);
			food[i] = cur_Food;
		}
		if(mod == 4) {
			x = rand() % X_MAX;
			y = rand() % Y_MAX;
			x = new_food_check(x,y);
			cur_Food = newFood(x, y, val, 2, draw_diamond_food);
			food[i] = cur_Food;
		}
	}
}
void fill_rocks() {
	FOOD* r;
	unsigned int i;
	int x;
	int y;
	for(i = 0; i < 2; i++) {
		x = rand() % X_MAX;
		y = rand() % Y_MAX;
		x = new_food_check(x,y);
		r = newFood(x, y, 1, 3, draw_square_food);
		rock[i] = r;
	}
}

void food_col() {

	if(level < 3) {
		int i;
		for(i = 0; i < 10; i++) {
			if(snake[head].point.x == food[i]->point.x) {
				if(snake[head].point.y == food[i]->point.y) {
					food_hit = 1;
					cur_score += food[i]->value;
					if(level == 1) {
						free(food[i]);
						food[i] = replace_food(i % 5);
					}
					else {
						food[i]->point.x = 50;
					}
					snake_size++;
					food_eaten+=1;
					beep();
					blink();
					LCD_UPDATE_event();
				}
			}
		}
	}
	else {
		if(snake[head].point.x == food[0]->point.x) {
			if(snake[head].point.y == food[0]->point.y) {
				food_hit = 1;
				cur_score += food[0]->value;
				free(food[0]);
				food[0] = replace_food(rand()%5);
				snake_size++;
				food_eaten+=1;
				beep();
				blink();
				LCD_UPDATE_event();
			}
		}
	}
}

void self_col() {
	int i;
	for(i=head-1;i>= tail;i--) {

	    if(snake[head].point.x == snake[i].point.x) {
	        if(snake[head].point.y == snake[i].point.y) {
	            self_hit=1;
	        }
	    }
	}
	return;
}
void rock_col() {
	int i;
	for(i = 0; i < 2; i++) {
		if(rock[i] == NULL) break;
		if(snake[head].point.x == rock[i]->point.x) {
			if(snake[head].point.y == rock[i]->point.y) {
				rock_hit = 1;
			}
		}
	}

}
int new_food_check(int x, int y) {
	int i = 0;
	for(i = 0; i < 10; i++) {
		if(food[i] == NULL) break;
		//add rock drawing check right here

		if(x == food[i]->point.x){
			if(y == food[i]->point.x){
				x = rand() % X_MAX;
			}
		}
	}
	for(i = 0; i < 2; i++) {
		if(food[i] == NULL) break;
		//add rock drawing check right here

		if(x == rock[i]->point.x){
			if(y == rock[i]->point.x){
				x = rand() % X_MAX;
			}
		}
	}
	for(i = head; i >= tail; i--) {
		if(x == snake[i].point.x){
			if(y == snake[i].point.x){
				x = rand() % X_MAX;
			}
		}
	}
	return x;
}
