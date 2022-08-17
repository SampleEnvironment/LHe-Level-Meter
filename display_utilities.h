// Display_utilities.h - Copyright 2016, HZB, ILL/SANE & ISIS

#ifndef DISPLAY_UTILITIES_H
#define DISPLAY_UTILITIES_H

#include <avr/io.h>



#include "disp/display_lib.h"
#include "main.h"
#include "Controller/base_controller.h"
#include "DS3231M.h"

// Variables for Display

extern uint8_t XOffset;
extern  uint8_t xoff;

extern globalModesType global_mode;

//Control PINs
#define DISPLAY_ON 				(PINB & (1<<PB3))    	// Set PORTB.3 as true
#define DISPLAY_TURN_ON 		PORTB|=(1<<PB3);		// Set PORTB.3 as true
#define DISPLAY_TURN_OFF 		PORTB&=~(1<<PB3);		// Set PORTB.3 as false

///Layout Colors
#define FGC 0b0101101011011111				///< Foreground color
#define BGC black				///< Background color
#define ERR	white				///< Warning/Active color
#define D_BGC blue				///< Dialog Background color
#define D_FGC white				///< Dialog Foreground color






#define RIGHT 		1
#define LEFT 		0
#define	UPDATE 		3

//#define DEVICE_ID 	1			// What is the interest and what is the link with the display ???
#define PASSWORD  	0

#define PAINT_ALL 	0
#define UPDATE_ONLY 1

typedef struct  
{
	uint8_t digits[4];
	uint8_t xCoords[4];
	int8_t active_digit;
	int8_t active_last;
	
	_Bool dev_id; // false: get_num for entering a Password
				  // true: get_um  for entering a Vessel-number with a three digit alphanumeric code (extended charset)
	
}get_numType;



extern LVM_ModelType LVM;
extern TFontInfo FontInfo[];


// Call variables declared into main.c
// extern uint8_t dewar_volume;		// Max volume of Helium dewar

void draw_int_without_erasing(int32_t number, uint16_t x, uint16_t y, char *unit, unsigned int color, uint8_t fontnr);
void draw_int(int32_t number,uint16_t x, uint16_t y, char *unit, unsigned int color, uint8_t fontnr);
void draw_double_without_erasing(double number, uint16_t x, uint16_t y, uint8_t prec, char *unit, unsigned int color, uint8_t fontnr);
void draw_double(double number, uint16_t x, uint16_t y, uint8_t prec, char *unit, unsigned int color, uint8_t fontnr);
void paint_wait_screen(void);
void paint_he_level(double he_level, double total_volume, _Bool print_Percentage);

void paint_info_line(char *line, _Bool update);
void paint_time_pressure(struct tm ltime, double lpress, _Bool update);
void paint_batt(uint8_t batt, uint8_t critical_batt);
void paint_progress_bar(uint8_t x, uint8_t y, uint8_t progress);
void clear_progress_bar(uint8_t x, uint8_t y);
void draw_current_wait_time(uint8_t x, uint8_t y, uint16_t sec_must, uint16_t secs_is, unsigned int color);
void paint_buttons(char *top, char *bottom, uint8_t mitte);
void paint_enter_number(uint8_t param);
void paint_main(struct tm ltime, _Bool offline, _Bool update);
void paint_Notification( LCD_MessageType *Message);



void paint_start_filling(posType * PosModel, _Bool update);

void paint_filling(struct tm ltime, _Bool netstat, _Bool update, _Bool draw_wait);

char* itoc(char * letterstr,uint8_t letter_num);
_Bool  update_filling_pos(posType *PosModel,char *pos );


void paint_diag(uint8_t page);
uint8_t toY(double value, uint16_t max_value, uint16_t points, uint8_t yzero_line);

_Bool display_on(void);
uint8_t get_Device_ID_old(uint8_t *status);

void InitScreen_AddLine(const char* Text, const char first_line);
_Bool LCD_Dialog(char *title, char *text, unsigned int BackColor, unsigned int ForeColor,uint8_t timeout);

void paint_Dev_ID(void);
void paint_get_password(void);

#endif  // sreenutils.h
