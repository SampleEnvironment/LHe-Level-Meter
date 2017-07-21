// Display_utilities.c - Copyright 2016, HZB, ILL/SANE & ISIS
#define RELEASE_DISPUTILS 1.00

// HISTORY -------------------------------------------------------------
// 1.00 - First release on March 2016

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <util/delay.h>

#include "main.h"

#include "display.h"
#include "display_utilities.h"
#include "keyboard.h"
#include "timer_utilities.h"

///draws an unsigned integer32 at (x,y) with unit-string added
///tweak screen clearing for large numbers
void draw_int(uint32_t number, uint8_t x, uint8_t y, char *unit, unsigned int color)
{	
	char temp[20];
	
	//convert number to string and print it 
	utoa(number,temp,10);
	strcat(temp, unit);	
	
	//clear screen 5 chars...
	LCD_Print("     ", x, y, 2, 1,1, color, BGC);
	LCD_Print(temp, x, y, 2, 1,1, color, BGC);
}
///draws an double at (x,y) with unit-string added
///tweak screen clearing for large numbers
void draw_double(double number, uint8_t x, uint8_t y, uint8_t prec, char *unit, unsigned int color)
{
	char str_temp[20];
	
	switch(prec)
	{	
		case 0:		//convert number to string (3 digits total with 0 decimal place)
					dtostrf(number,3,0,str_temp);
					break;
		case 1:		//convert number to string (4 digits total with 1 decimal place)
					dtostrf(number,4,1,str_temp);
					break;
		case 2:	
					dtostrf(number,5,2,str_temp);
					break;
		case 3:	
					dtostrf(number,6,3,str_temp);
					break;
		case 4:	
					dtostrf(number,7,4,str_temp);
					break;
		default:	
					dtostrf(number,4,1,str_temp);
	}
	strcat(str_temp, unit);
	//clear screen, 6 chars...
	LCD_Print("      ", x, y, 2, 1,1, color, BGC);
	LCD_Print(str_temp, x, y, 2, 1,1, color, BGC);
}

void paint_wait_screen(void)
{
	LCD_Cls(BGC);
	LCD_Print("Please wait...", 5, 50, 2, 1, 1, FGC, BGC);
}

void paint_he_level(double he_level, double total_volume)
{
//	he_level in % / dewar_volume total volume in L
//	uint8_t dewar_volume = 200; // volume in liters
	
	char temp[10];
	LCD_Print("    ", 10, 37, 2, 4,4, ERR, BGC);
	
	if(he_level == errCode_TooHighRes)
		LCD_Print("Cable?", 25, 37, 2, 2,2, ERR, BGC);
	else	
	{
		sprintf(temp, "%dL",(uint8_t)((he_level/100)*total_volume));
		LCD_Print(temp, 2, 37, 2, 4, 4, ERR, BGC);
	}
	
	
/*
	switch((int8_t)he_level)
	{	
		// Dewar empty
		case 0:
		LCD_Print("0L", 25, 37, 2, 4,4, ERR, BGC);
		break;

		//0 or negative current
		case 101://-1:	
			LCD_Print("cable?", 25, 37, 2, 2,2, ERR, BGC);
			break;
		//ubervoll
		case 100://-2:	
//			LCD_Print("100%", 2, 37, 2, 4,4, ERR, BGC);
			sprintf(temp, "%dL",((uint8_t) dewar_volume));
			LCD_Print(temp, 2, 37, 2, 4,4, ERR, BGC);
			break;
		//uberleer
		default:	
					if(he_level < 100)
					{
//						sprintf(temp, "%d%%",((uint8_t) he_level);
						sprintf(temp, "%dL",((uint8_t) (he_level/100)*dewar_volume));
						LCD_Print(temp, 25, 37, 2, 4,4, ERR, BGC);
					}
					else LCD_Print("err", 25, 37, 2, 4,4, ERR, BGC);
	}
*/
}

///paints static battery indicator with range 0-100%
void paint_batt(uint8_t batt, uint8_t critical_batt)
{
	if(batt > 100) batt = 100;
	
	//clear screen
	LCD_Box(38,119,140,131,BGC);
	
	//draw borders
	LCD_Rect(38,120,140,130,0, FGC);	
	
	//if lower then BATT_LOW draw in ERROR-color
	(batt < critical_batt)?
		LCD_Box(39,121,batt+39,129,red)
	:	LCD_Box(39,121,batt+39,129,green);
	
	LCD_Print("Batt",1,118,2,1,1,FGC, BGC);
}

///paints progress bar with progress parameter range 1-10 
void paint_progress_bar(uint8_t x, uint8_t y, uint8_t progress)
{
	char temp[5];
	static uint8_t step = 0;		
	
	if (!progress) return;					//return on 0 progress
	if (step == progress) return;			//do not draw if progress did not changed
	if (progress > 10) progress = 10;
	
	//clear screen if new progress bar is being painted
	if (step > progress) clear_progress_bar(x,y);	
	
	step = progress;						//remember last progress
	LCD_Rect(x-2,y-2,x+100,y+7, 0, FGC);	//draw borders
	
	for(uint8_t i=0;i<progress;i++)		//draw progress bar
	{
		LCD_Box(x+i*10,y,x+i*10+8,y+5,FGC);
	}
	
	//draw xx%
	utoa((progress)*10,temp,10);
	strcat(temp, "%");	
	
	//clear screen 4 chars... 100%
	LCD_Print("    ", x+102, y-6, 2, 1,1, FGC, BGC);
	LCD_Print(temp, x+102, y-6, 2, 1,1, FGC, BGC);
}

void clear_progress_bar(uint8_t x, uint8_t y)
{
	//LCD_Box(x-3,y-3,x+101,y+8, BGC);  nur balken
	LCD_Box(x-3,y-3,x+136,y+8, BGC);
}


void draw_current_wait_time(uint8_t x, uint8_t y, uint16_t sec_must, uint16_t secs_is, unsigned int color)
{
	static uint16_t last_value = 0;
	if (last_value == secs_is) return;
	
	last_value = secs_is;
	
	char temp[10];
	uint8_t mins = (sec_must - secs_is)/60;
	uint8_t rest_secs = (sec_must - secs_is) % 60; 
	
	if (mins > 9)
	{
		draw_int(mins, x, y, ":", color);
	}
	else {
		sprintf(temp,"0%d:", mins);
		LCD_Print(temp, x, y, 2, 1,1, color, BGC);
	}
	
	if (rest_secs > 9)
	{
		//draw_int(rest_secs, x+30, y, "", color);
		utoa(rest_secs,temp,10);
		
		//clear screen 2 chars... 100%
		LCD_Print("  ", x+30, y, 2, 1,1, FGC, BGC);
		LCD_Print(temp, x+30, y, 2, 1,1, FGC, BGC);
	}
	else {
		sprintf(temp,"0%d", rest_secs);
		LCD_Print(temp, x+30, y, 2, 1,1, color, BGC);
	}
}

///paints buttons on the left of the screen
void paint_buttons(char *top, char *bottom, uint8_t type)
{	
	//clear screen
	LCD_Box(144,0,176,131,BGC);
	
	//draw captions according to type
	switch(type)
	{
		case 1:
				LCD_Print("+",150,25,2,2,2,FGC, BGC);
				LCD_Print("-",151,77,2,2,2,FGC, BGC);
				break;
		case 2:
				LCD_Print("<-",145,61,1,1,1,FGC, BGC);
				LCD_Print("->",161,61,1,1,1,FGC, BGC);
				break;
		case 3:				
				LCD_Print("+",150,25,2,2,2,FGC, BGC);
				LCD_Print("<-",145,61,1,1,1,FGC, BGC);	
				LCD_Print("->",161,61,1,1,1,FGC, BGC);
				LCD_Print("-",151,77,2,2,2,FGC, BGC);
				break;
		default://no captions other then oben unten
				break;			
	}
	LCD_Print(top,148,5,1,1,2,FGC, BGC);
	LCD_Print(bottom,148,109,1,1,2,FGC, BGC);

	//box
	LCD_Draw(144,1,144,130,0,FGC); 	// |
	LCD_Draw(144,1,174,1,0,FGC);		// -
	LCD_Draw(174,1,174,130,0,FGC);		// |
	LCD_Draw(144,130,174,130,0,FGC);	// -
	
	//separators
	LCD_Draw(144,26,174,26,0,FGC);
	LCD_Draw(144,52,174,52,0,FGC);
	LCD_Draw(144,78,174,78,0,FGC);
	LCD_Draw(144,104,174,104,0,FGC);
	LCD_Draw(159,52,159,78,0,FGC); 	// |	
}

///paints login window
void paint_enter_number(uint8_t param) 
{
	LCD_Cls(BGC);
	
	switch(param)
	{
		case 0:	
				LCD_Print("Connection", 5, 2, 2, 1,1, ERR, BGC);
				LCD_Print("to server", 5, 15, 2, 1,1, ERR, BGC);
				LCD_Print("Enter Vessel ID:", 5, 40, 2, 1, 1, FGC, BGC);
				break;
		case 1:
				LCD_Print("Options", 52, 2, 2, 1,1, ERR, BGC);
				LCD_Print("Enter Password:", 5, 35, 2, 1, 1, FGC, BGC);
				break;
		default:
				break;
	}

	LCD_Print("0", 30, 70, 2, 2, 3, FGC, BGC);
	LCD_Print("0", 60, 70, 2, 2, 3, FGC, BGC);
	LCD_Print("0", 90, 70, 2, 2, 3, ERR, BGC);
	
	paint_buttons("canc"," ok", 3);
}
///paints or updates main window depending on update parameter
void paint_main(uint16_t nr, char *position, double he_level, double total_volume, uint8_t batt, uint8_t critical_batt, _Bool update) 
{
	char temp[40];
	
	if (!update)
	{
		LCD_Cls(BGC);
		//device id
		LCD_Print("Vessel#", 1, 1, 2, 1,1, FGC, BGC);
		/*(CHECK_NETWORK_ERROR)?
			LCD_Print("No connection", 1, 20, 1, 1,1, ERR, BGC)
		:	LCD_Print("Connected    ", 1, 20, 1, 1,1, ERR, BGC);*/
		
		itoa(nr, temp, 10);
		LCD_Print(temp, 65, 2, 2, 1,1, ERR, BGC);
		
		//position
		LCD_Print(position, 100, 2, 2, 1,1, ERR, BGC);
		
		//HE-Level
		paint_he_level(he_level, total_volume);
			
		paint_buttons(" ","opts",0);
//		LCD_Print("Fill",148,31,1,1,2,FGC, BGC);
		LCD_Print("Conn",148,83,1,1,2,FGC, BGC);
	}
	else
	{
		//HE-Level
		LCD_Print("   ", 25, 37, 2, 4,4, ERR, BGC);
		paint_he_level(he_level, total_volume);
	}
	paint_batt(batt, critical_batt);
}

///paints offline window
void paint_offline(uint16_t nr, char *position, double he_level, double total_volume, uint8_t batt, uint8_t critical_batt, _Bool update)
{
	if(!update)
	{
		paint_main(nr,"OFF",he_level, total_volume,batt,critical_batt,0);
		paint_buttons(" ","opts",0);
	}
	else {
		//HE-Level
		paint_he_level(he_level, total_volume);
		paint_batt(batt, critical_batt);
	}
}
///paints given options page // 1. options is always active (highlightened)
void paint_options(uint8_t page) 
{
	LCD_Cls(BGC);
	switch(page)
	{
		case 1:		
				LCD_Print("Options 1/5", 35, 2, 2, 1,1, ERR, BGC);
				
				LCD_Print(STRING_TRANSMIT_SLOW, 2, 20, 2, 1,1, ERR, BGC);		//<- ERR color = 1. option active
				//LCD_Print("0h", 85, 20, 2, 1,1, FGC, BGC);
				
				LCD_Print(STRING_TRANSMIT_FAST, 2, 40, 2, 1,1, FGC, BGC);
				//LCD_Print("0min  ", 85, 40, 2, 1,1, FGC, BGC);
				
				LCD_Print(STRING_HEAT_TIME, 2, 60, 2, 1,1, FGC, BGC);
				//LCD_Print("0.0s", 85, 60, 2, 1,1, FGC, BGC);
				
				LCD_Print(STRING_MEASUREMENT_CYCLES, 2, 80, 2, 1,1, FGC, BGC);
				//LCD_Print("0.0s", 85, 80, 2, 1,1, FGC, BGC);
				
				LCD_Print(STRING_FILLING_TIMEOUT, 2, 100, 2, 1,1, FGC, BGC);
				//LCD_Print("0min", 85, 100, 2, 1,1, FGC, BGC);
				
				paint_buttons("esc", "next", 3);
				break;
				
		case 2:		
				LCD_Print("Options 2/5", 35, 2, 2, 1,1, ERR, BGC);
				
				LCD_Print(STRING_RES_MIN, 2, 20, 2, 1,1, ERR, BGC);
				//LCD_Print("0.0o", 85, 20, 2, 1,1, FGC, BGC);
				LCD_Print(STRING_RES_MAX, 2, 40, 2, 1,1, FGC, BGC);
				//LCD_Print("0.0o", 85, 40, 2, 1,1, FGC, BGC);
				LCD_Print(STRING_BATTMIN, 2, 60, 2, 1,1, FGC, BGC);
				LCD_Print(STRING_BATTMAX, 2, 80, 2, 1,1, FGC, BGC);
				LCD_Print(STRING_CRITVOLT, 2, 100, 2, 1,1, FGC, BGC);
				
				paint_buttons("prev", "next", 3);
				break;
				
		case 3:
				LCD_Print("Options 3/5", 35, 2, 2, 1,1, ERR, BGC);
				
				LCD_Print(STRING_POS, 2, 20, 2, 1,1, ERR, BGC);
				//LCD_Print("off", 85, 20, 2, 1,1, FGC, BGC);
				LCD_Print(STRING_AUTOFILL, 2, 40, 2, 1,1, FGC, BGC);
				LCD_Print("off", 85, 40, 2, 1,1, FGC, BGC);
				
				LCD_Print(STRING_HE_MIN_LVL, 2, 60, 2, 1,1, FGC, BGC);
				LCD_Print(STRING_FILLING_TIMEOUT, 2, 80, 2, 1,1, FGC, BGC);
				
				LCD_Print(STRING_WAIT_TIME, 2, 100, 2, 1,1, FGC, BGC);

				paint_buttons("prev", "next", 3);
				break;
		case 4:		
				LCD_Print("Options 4/5", 35, 2, 2, 1,1, ERR, BGC);
				
				LCD_Print(STRING_SPAN, 2, 20, 2, 1, 1, ERR, BGC);
				//LCD_Print("off", 85, 20, 2, 1,1, FGC, BGC);
				LCD_Print(STRING_ZERO, 2, 40, 2, 1, 1, FGC, BGC);
				//LCD_Print("PresMax:", 2, 60, 2, 1,1, FGC, BGC);
				LCD_Print(STRING_ENABLE_PR, 2, 60, 2, 1, 1, FGC, BGC);
				LCD_Print(STRING_QUENCH_CURRENT, 2, 80, 2, 1, 1, FGC, BGC);
				LCD_Print(STRING_MEAS_CURRENT, 2, 100, 2, 1, 1, FGC, BGC);
				
				paint_buttons("prev", "next", 3);
				break;
		case 5:
				LCD_Print("Options 5/5", 35, 2, 2, 1,1, ERR, BGC);
				
				LCD_Print(STRING_SHUTDOWN, 2, 20, 2, 1,1, ERR, BGC);
				LCD_Print("off", 85, 20, 2, 1,1, FGC, BGC);
				LCD_Print(STRING_DIAG, 2, 40, 2, 1,1, FGC, BGC);
				LCD_Print("off", 85, 40, 2, 1,1, FGC, BGC);
				
				LCD_Print(STRING_ADCSPAN, 2, 60, 2, 1,1, FGC, BGC);				
				LCD_Print(STRING_ADCZERO, 2, 80, 2, 1,1, FGC, BGC);
				
				LCD_Print(STRING_TOTAL_VOL, 2, 100, 2, 1, 1, FGC, BGC);

				paint_buttons("prev", "esc", 3);
		default:
				break;
	}
}

void paint_opt_values_page1(_Bool transmit_slow_min, uint16_t transmit_slow, uint8_t transmit_fast, double heat_time, uint8_t meas_cycles, uint8_t fill_timeout)
{
	(transmit_slow_min) ? 
		draw_int(transmit_slow, 85, 20, "min", FGC) 
	: 	draw_int(transmit_slow, 85, 20, "h", FGC);
	
	draw_int(transmit_fast, 85, 40, "min", FGC);
	draw_double(heat_time, 76, 60, 0, "s", FGC);
	draw_int(meas_cycles, 85, 80, "c", FGC);
	draw_int(fill_timeout, 85, 100, "min", FGC);
}

void paint_opt_values_page2(double res_min, double res_max, double batt_min, double batt_max, uint8_t critical_batt)
{
	draw_double(res_min, 76, 20, 1, "o", FGC);
	draw_double(res_max, 76, 40, 1, "o", FGC);
	draw_double(batt_min, 76, 60, 1, "V", FGC);
	draw_double(batt_max, 76, 80, 1, "V", FGC);
	draw_int(critical_batt, 85, 100, "%", FGC);
}

void paint_opt_values_page3(char *device_pos, _Bool auto_fill_enabled, int8_t he_min, uint8_t fill_timeout, double wait_time)
{
	LCD_Print(device_pos, 85, 20, 2, 1,1, FGC, BGC);
	(auto_fill_enabled)?
		LCD_Print("on ", 85, 40, 2, 1,1, FGC, BGC)
	:	LCD_Print("off", 85, 40, 2, 1,1, FGC, BGC);
	draw_int(he_min, 85, 60, "%", FGC);
	draw_int(fill_timeout, 85, 80, "min", FGC);
	draw_double(wait_time, 85, 100, 0, "s", FGC);
}

void paint_opt_values_page4(double span, double zero, _Bool enable_pressure, double quench_current, double meas_current)
{
	draw_double(span, 76, 20, 1, "", FGC);
	draw_double(zero, 76, 40, 1, "", FGC);
	(enable_pressure)?
		LCD_Print("on ", 85, 60, 2, 1,1, FGC, BGC)
	:	LCD_Print("off", 85, 60, 2, 1,1, FGC, BGC);
	draw_double(quench_current, 85, 80, 0, "mA", FGC);
	draw_double(meas_current, 85, 100, 0, "mA", FGC);
}

void paint_opt_values_page5(double adc_span, double adc_zero, double total_volume)
{
	draw_double(adc_span, 76, 60, 1, "", FGC);
	draw_double(adc_zero, 76, 80, 1, "", FGC);

	draw_double(total_volume, 76, 100, 1, "L", FGC);
}

void paint_current_opt_page1(uint8_t option, uint8_t key)
{
	if(KEY_DOWN_S9 == key)
	{
		switch(option) 
		{
			case 1:	//1 is active, 5 was active
					LCD_Print(STRING_TRANSMIT_SLOW, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_FILLING_TIMEOUT, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 1 was active
					LCD_Print(STRING_TRANSMIT_FAST, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_TRANSMIT_SLOW, 2, 20, 2, 1,1, FGC, BGC);
					break;
			case 3:	//3 is active, 2 was active
					LCD_Print(STRING_HEAT_TIME, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_TRANSMIT_FAST, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 4:	//4 is active, 3 was active
					LCD_Print(STRING_MEASUREMENT_CYCLES, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_HEAT_TIME, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 5:	//5 is active, 4 was active
					LCD_Print(STRING_MEASUREMENT_CYCLES, 2, 80, 2, 1,1, FGC, BGC);
					LCD_Print(STRING_FILLING_TIMEOUT, 2, 100, 2, 1,1, ERR, BGC);
					break;
			default:
					break;
		}
	} else {
		switch(option) 
		{
			case 1:	//1 is active, 2 was aktiv
					LCD_Print(STRING_TRANSMIT_SLOW, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_TRANSMIT_FAST, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 3 was aktiv
					LCD_Print(STRING_TRANSMIT_FAST, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_HEAT_TIME, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 3:	//3 is active, 4 was aktiv
					LCD_Print(STRING_HEAT_TIME, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_MEASUREMENT_CYCLES, 2, 80, 2, 1,1, FGC, BGC);
					break;
			case 4:	//4 is active, 5 was aktiv
					LCD_Print(STRING_MEASUREMENT_CYCLES, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_FILLING_TIMEOUT, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 5:	//5 is active, 1 was aktiv
					LCD_Print(STRING_FILLING_TIMEOUT, 2, 100, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_TRANSMIT_SLOW, 2, 20, 2, 1,1, FGC, BGC);
					break;
			default:
					break;	
		}
	}
}
void paint_current_opt_page2(uint8_t option, uint8_t key)
{
	if(KEY_DOWN_S9 == key)
	{
		switch(option) 
		{
			case 1:	//1 it active, 2 was active
					LCD_Print(STRING_RES_MIN, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_CRITVOLT, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 1 was active
					LCD_Print(STRING_RES_MAX, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_RES_MIN, 2, 20, 2, 1,1, FGC, BGC);
					break;
			case 3:	
					LCD_Print(STRING_BATTMIN, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_RES_MAX, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 4:	
					LCD_Print(STRING_BATTMAX, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_BATTMIN, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 5:	
					LCD_Print(STRING_CRITVOLT, 2, 100, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_BATTMAX, 2, 80, 2, 1,1, FGC, BGC);
					break;
			default:
					break;
		}
	} else {
		//switch between options showing the current active
		switch(option) 
		{
			case 1:	//1 is active, 2 was active
					LCD_Print(STRING_RES_MIN, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_RES_MAX, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 3 was active
					LCD_Print(STRING_RES_MAX, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_BATTMIN, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 3:	//2 is active, 1 was active
					LCD_Print(STRING_BATTMIN, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_BATTMAX, 2, 80, 2, 1,1, FGC, BGC);
					break;
			case 4:	//2 is active, 1 was active
					LCD_Print(STRING_BATTMAX, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_CRITVOLT, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 5:	//2 is active, 1 was active
					LCD_Print(STRING_CRITVOLT, 2, 100, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_RES_MIN, 2, 20, 2, 1,1, FGC, BGC);
					break;	
			default:
					break;	
		}
	}
}
void paint_current_opt_page3(uint8_t option, uint8_t key)
{
	if(KEY_DOWN_S9 == key)
	{
		switch(option) 
		{
			case 1:	//1 is active, 5 was active
					LCD_Print(STRING_POS				, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_WAIT_TIME			, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 1 was active
					LCD_Print(STRING_AUTOFILL			, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_POS				, 2, 20, 2, 1,1, FGC, BGC);
					break;
			case 3:	//3 is active, 2 was active
					LCD_Print(STRING_HE_MIN_LVL			, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_AUTOFILL			, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 4:	//4 is active, 3 was active
					LCD_Print(STRING_FILLING_TIMEOUT	, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_HE_MIN_LVL			, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 5:	//5 is active, 4 was active
					LCD_Print(STRING_WAIT_TIME			, 2, 100, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_FILLING_TIMEOUT	, 2, 80, 2, 1,1, FGC, BGC);
					break;
			default:
					break;
		}
	} else {
		switch(option) 
		{
			case 1:	//1 is active, 2 was active
					LCD_Print(STRING_POS				, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_AUTOFILL			, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 3 was active
					LCD_Print(STRING_AUTOFILL			, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_HE_MIN_LVL			, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 3:	//3 is active, 4 was active
					LCD_Print(STRING_HE_MIN_LVL			, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_FILLING_TIMEOUT	, 2, 80, 2, 1,1, FGC, BGC);
					break;
			case 4:	//4 is active, 5 was active
					LCD_Print(STRING_FILLING_TIMEOUT	, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_WAIT_TIME			, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 5:	//5 is active, 1 was active
					LCD_Print(STRING_WAIT_TIME			, 2, 100, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_POS				, 2, 20, 2, 1,1, FGC, BGC);
					break;
			default:
					break;
		}
	}
}
void paint_current_opt_page4(uint8_t option, uint8_t key)
{
	if(KEY_DOWN_S9 == key)
	{
		switch(option) 
		{
			case 1:	//1 is active, 5 was active
					LCD_Print(STRING_SPAN, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_MEAS_CURRENT, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 1 was active
					LCD_Print(STRING_ZERO, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_SPAN, 2, 20, 2, 1,1, FGC, BGC);
					break;
			case 3:	//3 is active, 2 was active
					LCD_Print(STRING_ENABLE_PR, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_ZERO, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 4:	//4 is active, 3 was active
					LCD_Print(STRING_QUENCH_CURRENT, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_ENABLE_PR, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 5:	//5 is active, 4 was active
					LCD_Print(STRING_MEAS_CURRENT, 2, 100, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_QUENCH_CURRENT, 2, 80, 2, 1,1, FGC, BGC);
					break;
			default:
					break;
		}
	} else {
		switch(option) 
		{
			case 1:	//1 is active, 2 was active
					LCD_Print(STRING_SPAN, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_ZERO, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 3 was active
					LCD_Print(STRING_ZERO, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_ENABLE_PR, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 3:	//3 is active, 4 was active
					LCD_Print(STRING_ENABLE_PR, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_QUENCH_CURRENT, 2, 80, 2, 1,1, FGC, BGC);
					break;
			case 4:	//4 is active, 5 was active
					LCD_Print(STRING_QUENCH_CURRENT, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_MEAS_CURRENT, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 5:	//5 is active, 1 was active
					LCD_Print(STRING_MEAS_CURRENT, 2, 100, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_SPAN, 2, 20, 2, 1,1, FGC, BGC);
					break;
			
			default:
					break;
		}
	}
}
void paint_current_opt_page5(uint8_t option, uint8_t key)
{
	if(KEY_DOWN_S9 == key)
	{
		switch(option) 
		{
			case 1:	//1 is active, 5 was active
					LCD_Print(STRING_SHUTDOWN, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_TOTAL_VOL, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 1 was active
					LCD_Print(STRING_DIAG, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_SHUTDOWN, 2, 20, 2, 1,1, FGC, BGC);
					break;
			case 3:	//3 is active, 2 was active
					LCD_Print(STRING_ADCSPAN, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_DIAG, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 4:	//4 is active, 3 was active
					LCD_Print(STRING_ADCZERO, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_ADCSPAN, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 5:	//5 is active, 4 was active
					LCD_Print(STRING_TOTAL_VOL, 2, 100, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_ADCZERO, 2, 80, 2, 1,1, FGC, BGC);
					break;
			default:
					break;
		}
	} else {
		//switch between options showing the current active
		switch(option) 
		{
			case 1:	//1 is active, 2 was active
					LCD_Print(STRING_SHUTDOWN, 2, 20, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_DIAG, 2, 40, 2, 1,1, FGC, BGC);
					break;
			case 2:	//2 is active, 3 was active
					LCD_Print(STRING_DIAG, 2, 40, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_ADCSPAN, 2, 60, 2, 1,1, FGC, BGC);
					break;
			case 3:	//3 is active, 4 was active
					LCD_Print(STRING_ADCSPAN, 2, 60, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_ADCZERO, 2, 80, 2, 1,1, FGC, BGC);
					break;
			case 4:	//4 is active, 5 was active
					LCD_Print(STRING_ADCZERO, 2, 80, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_TOTAL_VOL, 2, 100, 2, 1,1, FGC, BGC);
					break;
			case 5:	//5 is active, 1 was active
					LCD_Print(STRING_TOTAL_VOL, 2, 100, 2, 1,1, ERR, BGC);
					LCD_Print(STRING_SHUTDOWN, 2, 20, 2, 1,1, FGC, BGC);
					break;
			default:
					break;	
		}
	}
}

///paints or updates measurement window
void paint_measure(uint16_t nr, char *position, double he_level, double total_volume, uint8_t batt, uint8_t critical_batt, _Bool update) 
{
	char temp[40];
	if (!update)
	{
		LCD_Cls(BGC);
		//device id
		LCD_Print("Vessel#", 1, 1, 2, 1,1, FGC, BGC);
		itoa(nr, temp, 10);
		LCD_Print(temp, 65, 2, 2, 1,1, ERR, BGC);
		
		//position
		LCD_Print(position, 100, 2, 2, 1,1, ERR, BGC);
		
		//HE-Level
		paint_he_level(he_level, total_volume);
	}
	else {	
		//HE-Level
		paint_he_level(he_level, total_volume);
	}
	paint_batt(batt, critical_batt);
}

///paints or updates filling position login window
void paint_start_filling(char *letter, uint8_t number, _Bool update)
{
	char temp[30];
	
	if (!update)
	{
		LCD_Cls(BGC);
		LCD_Print("Fill", 52, 2, 2, 1,1, ERR, BGC);
		LCD_Print("Enter Position", 5, 35, 2, 1, 1, FGC, BGC);
		
		LCD_Print(letter, 30, 70, 2, 2, 3, ERR, BGC);
		
		if(letter[1] == '\0')
		{
			itoa(number, temp, 10);
			LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
			//itoa(number2, temp, 10);
			//LCD_Print(temp, 90, 70, 2, 2, 3, FGC, BGC);
		}
		paint_buttons("canc"," ok", 3);
	}
	else {
		LCD_Print(letter, 30, 70, 2, 2, 3, ERR, BGC);
		
		//itoa(number, temp, 10);
		//LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
		draw_int(number, 60, 70, "", FGC);
		//itoa(number2, temp, 10);
		//LCD_Print(temp, 90, 70, 2, 2, 3, FGC, BGC);
	}
}

///paints or updates filling window
void paint_filling(uint16_t nr, char *position, double he_level, double total_volume, uint8_t batt, uint8_t critical_batt, _Bool update)
{
	char temp[40];
	if (!update)
	{
		LCD_Cls(BGC);
		//device id
		LCD_Print("Vessel#", 1, 1, 2, 1,1, FGC, BGC);
		itoa(nr, temp, 10);
		LCD_Print(temp, 65, 2, 2, 1,1, ERR, BGC);
		
		//position
		LCD_Print(position, 100, 2, 2, 1,1, ERR, BGC);
		
		//HE-Level
		paint_he_level(he_level, total_volume);
		
		//LCD_Print("stop",148,5,1,1,2,FGC, BGC);
		//LCD_Print("more",148,20,1,1,2,FGC, BGC);
	}
	else {	
		//HE-Level
		paint_he_level(he_level, total_volume);
		
	}
	paint_buttons("stop","more", 0xff);
	paint_batt(batt, critical_batt);
}



///updates digits on login window
void update_login_digits(uint8_t which, uint8_t new_val, uint8_t last_active_val, uint8_t direction)
{
	char temp[30];
	
	switch(direction)
	{
		case RIGHT:
						switch(which) 
						{
							case 1:	
									itoa(new_val,temp,10);
									LCD_Print(temp, 30, 70, 2, 2, 3, ERR, BGC);	//1.digit active, 3. was active
									itoa(last_active_val,temp,10);
									LCD_Print(temp, 90, 70, 2, 2, 3, FGC, BGC);
									break;
							case 2: 
									itoa(new_val,temp,10);
									LCD_Print(temp, 60, 70, 2, 2, 3, ERR, BGC);	//2.digit active, 1. was active	
									itoa(last_active_val,temp,10);
									LCD_Print(temp, 30, 70, 2, 2, 3, FGC, BGC);
									break;
							case 3: 
									itoa(new_val,temp,10);
									LCD_Print(temp, 90, 70, 2, 2, 3, ERR, BGC);	//3.digit active, 2. was active
									itoa(last_active_val,temp,10);
									LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
									break;
						}
						break;
		case LEFT:
						switch(which) 
						{
							case 1:	
									itoa(new_val,temp,10);
									LCD_Print(temp, 30, 70, 2, 2, 3, ERR, BGC);	 //1.digit active, 2. was active
									itoa(last_active_val,temp,10);
									LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
									break;
							case 2: 
									itoa(new_val,temp,10);
									LCD_Print(temp, 60, 70, 2, 2, 3, ERR, BGC);	//2.digit active, 3. was active	
									itoa(last_active_val,temp,10);
									LCD_Print(temp, 90, 70, 2, 2, 3, FGC, BGC);
									break;
							case 3: 
									itoa(new_val,temp,10);
									LCD_Print(temp, 90, 70, 2, 2, 3, ERR, BGC);	//3.digit active, 1. was active
									itoa(last_active_val,temp,10);
									LCD_Print(temp, 30, 70, 2, 2, 3, FGC, BGC);
									break;
						}
						break;
		default://update digit
						switch(which) 
						{
							case 1:	
									itoa(new_val,temp,10);
									LCD_Print(temp, 30, 70, 2, 2, 3, ERR, BGC);	 //1.digit active, 3. was active
									break;
							case 2: 
									itoa(new_val,temp,10);
									LCD_Print(temp, 60, 70, 2, 2, 3, ERR, BGC);	//2.digit active, 1. was active	
									break;
							case 3: 
									itoa(new_val,temp,10);
									LCD_Print(temp, 90, 70, 2, 2, 3, ERR, BGC);	//3.digit active, 2. was active
									break;
						}
						break;
	}
	_delay_ms(300);
}

// Update position letters/digits on filling login 
_Bool update_filling_pos(char (*pos_letters)[FILL_LETTERS_LENGTH], uint8_t (*pos_ranges)[FILL_RANGES_LENGTH], char *pos)
{
	char temp[50];
	
	//filling position login
	_Bool fill_pos_login = true;
	_Bool two_letters = false;
	uint8_t digit_on=1;
	char *letter1;
	uint8_t *number;
	int8_t active_letter = 1;
	int8_t active_range_number = 1;
	letter1 = &pos_letters[active_letter/*1*/][0];
	number = &pos_ranges[active_letter/*1*/][active_range_number];
	
	paint_start_filling(letter1,*number,0);
	if(!letter1[1] == '\0') two_letters = true;
	
	while(fill_pos_login) 
	{
		switch(keyhit())  
		{
			case KEY_DOWN_S9:	
				switch(digit_on)
				{
					case 1:	
						letter1 = &pos_letters[--active_letter][0];
						
						active_range_number = 1;
						if (*letter1 == '\r') 
						{
							letter1 = &pos_letters[1][0];
							active_letter = 1;
							break;
						}
						number = &pos_ranges[active_letter][1];
						
						if(!letter1[1] == '\0')
						{
							two_letters = true; 
							LCD_Print("     ", 30, 70, 2, 2, 3, ERR, BGC);
						}
						else {
							LCD_Print("   ", 60, 70, 2, 2, 3, ERR, BGC);
							if(two_letters)
							{
								LCD_Print("    ", 30, 70, 2, 2, 3, ERR, BGC);
								itoa(*number, temp, 10);
								LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
								//LCD_Print("0", 90, 70, 2, 2, 3, FGC, BGC);
								two_letters = false;
							}
							else {
								itoa(*number, temp, 10);
								LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
							}
						}
						LCD_Print(letter1, 30, 70, 2, 2, 3, ERR, BGC);	 //1.digit active, 3. was active
						break;
					case 2: 
						number = &pos_ranges[active_letter][--active_range_number];
						if (active_range_number < 0) active_range_number = 1;
						if (*number == END) 
						{
							number = &pos_ranges[active_letter][1];
							active_range_number = 1;
							break;
						}
						itoa(*number,temp,10);
						LCD_Print("   ", 60, 70, 2, 2, 3, ERR, BGC);
						LCD_Print(temp, 60, 70, 2, 2, 3, ERR, BGC);	//2.digit active, 1. was active
						break;
				}
				_delay_ms(200);
				break;
			
			case KEY_BOT_S10:	
								if(!letter1[1] == '\0') 
								{
									sprintf(temp,"Do you really want\nto save\nfilling position %s?",letter1);
								}
								else {
									sprintf(temp,"Do you really want\nto save\nfilling position %s%d ?",letter1,*number);
								}
								
								if(LCD_Dialog("Position", temp, D_FGC, D_BGC)) 
								{ 
									uint8_t index = 0;
									while(*letter1 != '\0') pos[index++] = *letter1++;
									if (!two_letters)
									{
										if(*number > 9)
										{	
											itoa(*number, temp, 10);
											uint8_t idx = 0;
											while(temp[idx] != '\0') pos[index++] = temp[idx++];
										}
										else {
											pos[index++] = *number + '0';
										}
									}
									pos[index] = '\0';
									fill_pos_login = false;
								}
								else {
									paint_start_filling(letter1,*number,0);
									digit_on=1;
								}
								_delay_ms(200);
								break;
								
			case KEY_LEFT_S7:		
								if (!two_letters)
								{
									digit_on--;
									if (!digit_on) digit_on = 2;
									
									switch(digit_on)
									{
										case 1:	
												LCD_Print(letter1, 30, 70, 2, 2, 3, ERR, BGC);	 //1.digit active, 2. was active
												itoa(*number,temp,10);
												LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
												break;
										case 2: 
												itoa(*number,temp,10);
												LCD_Print(temp, 60, 70, 2, 2, 3, ERR, BGC);	//2.digit active, 1. was active	
												LCD_Print(letter1, 30, 70, 2, 2, 3, FGC, BGC);	
												break;
									}
								}
								_delay_ms(200);
								break;
			case KEY_RIGHT_S8:				
								if (!two_letters)
								{
									digit_on++;
									if (digit_on > 2) digit_on = 1;
									
									switch(digit_on)
									{
										case 1:	
												LCD_Print(letter1, 30, 70, 2, 2, 3, ERR, BGC);	
												itoa(*number,temp,10);
												LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
												break;
										case 2: 
												
												itoa(*number,temp,10);
												LCD_Print(temp, 60, 70, 2, 2, 3, ERR, BGC);	
												LCD_Print(letter1, 30, 70, 2, 2, 3, FGC, BGC);
												break;
									}
								}
								_delay_ms(200);
								break;			
								
			case KEY_UP_S6:	
								switch(digit_on)
								{
									case 1:	
											letter1 = &pos_letters[++active_letter][0];
											//if (active_letter < 0) active_letter = 1;
											
											active_range_number = 1;
											if (*letter1 == '\r') 
											{
												letter1 = &pos_letters[--active_letter][0]; //nicht auf 1. springen
												//letter1 = &pos_letters[1][0];
												//active_letter = 1;
												break;
											}
											number = &pos_ranges[active_letter][1];
											
											if(!letter1[1] == '\0')
											{
												two_letters = true; 
												LCD_Print("     ", 30, 70, 2, 2, 3, ERR, BGC);
											}
											else {
												LCD_Print("   ", 60, 70, 2, 2, 3, ERR, BGC);
												if(two_letters)
												{
													LCD_Print("    ", 30, 70, 2, 2, 3, ERR, BGC);
													itoa(*number, temp, 10);
													LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
													
													two_letters = false;
												}
												else {
													itoa(*number, temp, 10);
													LCD_Print(temp, 60, 70, 2, 2, 3, FGC, BGC);
												}
											}
											LCD_Print(letter1, 30, 70, 2, 2, 3, ERR, BGC);	 
											break;
									case 2: 
											number = &pos_ranges[active_letter][++active_range_number];
											if (*number == END) 
											{
												number = &pos_ranges[active_letter][--active_range_number];
												//number = &pos_ranges[active_letter][1];
												//active_range_number = 1;
												break;
											}
											itoa(*number,temp,10);
											LCD_Print("   ", 60, 70, 2, 2, 3, ERR, BGC);
											LCD_Print(temp, 60, 70, 2, 2, 3, ERR, BGC);	
											break;
								}
								_delay_ms(200);
								break;
								
			case KEY_TOP_S5:	
								return false;
								break;
			default:			
								break;
			 
		}
	}
	return true;
}

// Get Vessel ID / device number
///status 0 - std. login / password okay
///status 1 - hidden function login
///status 2 - esc (only while entering password)
uint16_t get_number(uint8_t *status, _Bool device_id)
{
	(device_id)?
		paint_enter_number(0)
	:	paint_enter_number(1);	// 0 to get vessel ID - 1 to get password for options
	
	_Bool login = true;

	int8_t digit_on=3;
	int8_t digit1=0;
	int8_t digit2=0;
	int8_t digit3=0;

	while(login)
	{
		switch(keyhit())  
		{
		  case KEY_DOWN_S9:		// Decrease the number ID
							switch(digit_on)
							{
								case 1:	
										digit1--;
										if (digit1 < 0) digit1 = 9;
										update_login_digits(digit_on,digit1,0,UPDATE);
										break;
								case 2: 
										digit2--;
										if (digit2 < 0) digit2 = 9;
										update_login_digits(digit_on,digit2,0,UPDATE);
										break;
								case 3: 
										digit3--;
										if (digit3 < 0) digit3 = 9;
										update_login_digits(digit_on,digit3,0,UPDATE);
										break;
							}
							break;
						
		  case KEY_BOT_S10:		// OK pressed
							login = false;
							*status = 0;
							_delay_ms(100);
							break;
							
		  case KEY_LEFT_S7:		// Increase digit
							digit_on--;
							if (!digit_on) digit_on = 3;
							
							switch(digit_on)
							{
								case 1:	
										update_login_digits(digit_on,digit1,digit2,LEFT);
										break;
								case 2: 
										update_login_digits(digit_on,digit2,digit3,LEFT);
										break;
								case 3: 
										update_login_digits(digit_on,digit3,digit1,LEFT);
										break;
							}
							break;
							
		  case KEY_RIGHT_S8:		// Decrease digit
							digit_on++;
							if (digit_on > 3) digit_on = 1;
							
							switch(digit_on)
							{
								case 1:	
										update_login_digits(digit_on,digit1,digit3,RIGHT);
										break;
								case 2: 
										update_login_digits(digit_on,digit2,digit1,RIGHT);
										break;
								case 3: 
										update_login_digits(digit_on,digit3,digit2,RIGHT);
										break;
							}
							break;			
							
		  case KEY_UP_S6:		// Increase number ID
							switch(digit_on)
							{
								case 1:	
										digit1++;
										if (digit1 > 9) digit1 = 0;
										update_login_digits(digit_on,digit1,0,UPDATE);
										break;
								case 2: 
										digit2++;
										if (digit2 > 9) digit2 = 0;
										update_login_digits(digit_on,digit2,0,UPDATE);
										break;
								case 3: 
										digit3++;
										if (digit3 > 9) digit3 = 0;
										update_login_digits(digit_on,digit3,0,UPDATE);
										break;
							}
							break;
							
		  case KEY_TOP_S5:		// Cancel
							if(!device_id) 
							{
								*status = 2;
								return 0;
							}
							paint_enter_number(0);
							digit1 = 0;
							digit2 = 0;
							digit3 = 0;
							digit_on = 3;
							break;
		  case HIDDEN_FUNCTION:	
							if(!device_id) break;	//disable hidden function while getting password
							
							_delay_ms(1000);
							if(keyhit() == HIDDEN_FUNCTION)
							{
								/*login = false;
								login_okay = true;
								mode = MODE_OFFLINE;
								err_code = OFFLINE_errCode;*/
								*status = 1;
								return 0;
							}
							break;
					
		  default:			
							break;
		}//switch(keyhit())
	}//while(login)
	return ((uint16_t)((digit1*100) + (digit2*10) + digit3));
}

///paints given diagnostics page
void paint_diag(uint8_t page)
{ 
	LCD_Cls(BGC);
	switch(page)
	{
		case 1:		
				LCD_Print("Diag 1/2", 35, 2, 2, 1,1, ERR, BGC);
				
				LCD_Print("Res:", 2, 20, 2, 1,1, FGC, BGC);
				LCD_Print("HeL:", 2, 40, 2, 1,1, FGC, BGC);
				LCD_Print("Bat:", 2, 60, 2, 1,1, FGC, BGC);
				LCD_Print("Prs:", 2, 80, 2, 1,1, FGC, BGC);
				LCD_Print("Cur:", 2, 100, 2, 1,1, FGC, BGC);
				
				paint_buttons("esc", "puls", 2);
				LCD_Print("cal",148,31,1,1,2,FGC, BGC);
				LCD_Print("curr",148,82,1,1,2,FGC, BGC);
				break;
				
		case 2:	
				LCD_Print("Diag 2/2", 35, 2, 2, 1,1, ERR, BGC);
				
				LCD_Print("ADCV:", 2, 20, 2, 1,1, FGC, BGC);
				LCD_Print("ADCI:", 2, 40, 2, 1,1, FGC, BGC);
				LCD_Print("I:", 2, 60, 2, 1,1, FGC, BGC);
				LCD_Print("U:", 2, 80, 2, 1,1, FGC, BGC);
				LCD_Print("Us:", 2, 100, 2, 1,1, FGC, BGC);
				
				paint_buttons("esc", "puls", 2);
				//LCD_Print("curr",148,31,1,1,2,FGC, BGC);
				LCD_Print("cal",148,82,1,1,2,FGC, BGC);
				break;
		case 3:	
				LCD_Print("Pulse", 35, 2, 2, 1,1, ERR, BGC);
				
				LCD_Draw(1,20,1,71,1,FGC); // |
				LCD_Print("17.5",1,22+25,1,1,1,FGC,BGC);
				LCD_Draw(1,71,1+140,71,1,FGC); // _
				
				LCD_Draw(1,75,1,126,1,FGC); // |
				LCD_Print("50",1,75+25,1,1,1,FGC,BGC);
				LCD_Draw(1,126,1+140,126,1,FGC); // _
				
				paint_buttons("save", "esc", 2);
				break;
	}
}

///maps adc value to graph y-axis with range 0-50
///top point of the y-axis is given as y
///used for adc-graph
uint8_t toY(double value, uint8_t map_max, uint8_t y) 
{
	//return ((y+50)-ceil((adcVal)/(1023/50.0)));
	return ((y+50)-ceil((50.0/map_max)*value));
}

//=========================================================================
// Display backlight toggle
//=========================================================================
inline _Bool display_on(void)
{
	if (DISPLAY_ON) 
	{
		// Display is on, extend display_on time
		set_timeout(0, TIMER_2, RESET_TIMER);
		set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
		return true;
	}
	else 
	{
		// Turn display backlight on
		DISPLAY_TURN_ON
		set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
		#ifdef ALLOW_DEBUG
			LCD_Print("on ", 5, 20, 2, 1, 1, FGC, BGC); 
		#endif
		return false;
	}
}

