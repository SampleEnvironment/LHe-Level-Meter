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

#ifdef ili9341
#include "disp/ili9341_driver.h"
#endif

#ifdef DISP_3000
#include "disp/DISP_3000_driver.h"
#endif

#include "disp/display_lib.h"
#include "display_utilities.h"
#include "keyboard.h"
#include "timer_utilities.h"
#include "DS3231M.h"
#include "xbee.h"
#include "xbee_AT_comm.h"
#include "adwandler.h"
#include "status.h"

#ifdef DISP_3000
#include "StringPixelCoordTable.h"
#endif
#ifdef ili9341
#include "StringPixelCoordTable_ili9341.h"
//#include "StringPixelCoordTable.h"
#endif


uint16_t InitScreenForeColor = ERR;
uint16_t InitScreenBackColor = BGC;
uint8_t InitScreenMaxNoOfLines = IAdd_Line_Max_Lines;
uint8_t InitScreenNextLine = 1;
uint8_t InitScreenLineFeed = IAdd_Line_LineFeed;
uint8_t InitScreenFontNr = 1;
uint8_t InitScreenXScale = 1;
uint8_t InitScreenYScale = 1;




// Variables for display



uint8_t XOffset = XOFFSET_32;                       // Offset in x when display is reversed


uint8_t xoff = 0;





///draws an unsigned integer32 at (x,y) with unit-string added
///tweak screen clearing for large numbers
void draw_int_without_erasing(int32_t number, uint16_t x, uint16_t y, char *unit, unsigned int color, uint8_t fontnr)
{
	char temp[20];
	
	//convert number to string and print it
	utoa(number,temp,10);
	int8_t str_len = strlen(temp);
	// old:	strcat(temp, unit);
	
	// print number
	LCD_Print(temp, x, y, fontnr, 1,1, color, BGC);
	// print unit with half space distance
	LCD_Print(unit, x+str_len*FontInfo[fontnr-1].CellWidth+3, y, fontnr, 1,1, color, BGC);

}


void draw_int(int32_t number, uint16_t x, uint16_t y, char *unit, unsigned int color, uint8_t fontnr)
{
	char temp[20];
	
	//convert number to string and print it
	utoa(number,temp,10);
	int8_t str_len = strlen(temp);
	// old:	strcat(temp, unit);
	
	//clear screen 5.3 chars...
	LCD_Print("     ", x, y, fontnr, 1,1, color, BGC);
	LCD_Print(" ", x+4*FontInfo[fontnr-1].CellWidth+4, y, fontnr, 1,1, color, BGC);
	// print number
	LCD_Print(temp, x, y, fontnr, 1,1, color, BGC);
	// print unit with half space distance
	LCD_Print(unit, x+str_len*FontInfo[fontnr-1].CellWidth+3, y, fontnr, 1,1, color, BGC);

}


///draws an double at (x,y) with unit-string added
///tweak screen clearing for large numbers

void draw_double_without_erasing(double number, uint16_t x, uint16_t y, uint8_t prec, char *unit, unsigned int color, uint8_t fontnr)
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
	int8_t str_len = strlen(str_temp);
	//	old: strcat(str_temp, unit);
	// print number
	LCD_Print(str_temp, x, y, fontnr, 1,1, color, BGC);
	// print unit with half space distance
	if (fontnr == 2)
	{
		LCD_Print(unit, x+str_len*CHAR_CELL_WIDTH_FONT_2+HALF_SPACE_WIDTH_FONT_2, y, fontnr, 1,1, color, BGC);
		}else{
		LCD_Print(unit, x+str_len*CHAR_CELL_WIDTH_FONT_1+HALF_SPACE_WIDTH_FONT_1, y, fontnr, 1,1, color, BGC);
	}
	

}


void paint_Notification( LCD_MessageType *Message){
	LCD_Cls(BGC);
	// first byte: 1: notice, 2: warning, 3: error
	switch (Message->type)
	{
		case 1: LCD_Print(STR_NOTICE, X_M_40, Y_M_20, 2, 1, 1, FGC, BGC); break;
		case 2: LCD_Print(STR_WARNING , X_M_40, Y_M_20, 2, 1, 1, FGC, BGC); break;
		case 3: LCD_Print(STR_ERROR, X_M_40, Y_M_20, 2, 1, 1, FGC, BGC); break;
		default: break;
	}
	// print the text starting from the second position in the data
	LCD_Print(Message->Str, X_M_5, Y_M_40_NOTICE, 2, 1, 1, white, BGC);
	LCD_Print(STR_PRESS_MEASURE , X_M_5, Y_M_57, FONTNR_M_1, 1, 1, FGC, BGC);
	LCD_Print(STR_TO_CONTINUE , X_M_5, Y_M_66, FONTNR_M_1, 1, 1, FGC, BGC);
	_delay_ms(1000);
	

}

void draw_double(double number, uint16_t x, uint16_t y, uint8_t prec, char *unit, unsigned int color, uint8_t fontnr)
{
	char str_temp[20];
	
	switch(prec)
	{
		case 0:		//convert number to string (3 digits total with 0 decimal place)
		dtostrf(number,3,0,str_temp);
		break;
		case 1:		//convert number to string (4 digits total with 1 decimal place)
		dtostrf(number,4,1,str_temp); //TODO numbers like 200,3 are to large for the Format ONLY 3 DIGITS TOTAL!!!!
		//the comma counts too. Proposal: leftshift by nine pix of the whole collumn
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
		case 5:
		draw_int(round(number), x, y, "mA", color, 2);
		return;
		default:
		dtostrf(number,4,1,str_temp);
	}
	int8_t str_len = strlen(str_temp);
	int8_t str_len_unit = strlen(unit);
	//	old: strcat(str_temp, unit);
	//clear screen, 7 chars...
	if (fontnr == 2)
	{
		LCD_Box(x,y,x+(CHAR_CELL_WIDTH_FONT_2*(str_len+str_len_unit)),y+FONT2_H,BGC);
		//LCD_Print("       ", x, y, 2, 1,1, color, BGC);
		// print number
		LCD_Print(str_temp, x, y, 2, 1,1, color, BGC);
		// print unit with half space distance
		LCD_Print(unit, x+str_len*CHAR_CELL_WIDTH_FONT_2+HALF_SPACE_WIDTH_FONT_2, y, 2, 1,1, color, BGC);
		}else{
		LCD_Box(x,y,x+(CHAR_CELL_WIDTH_FONT_1*(str_len+str_len_unit)),y+FONT1_H,BGC);
		//LCD_Print("       ", x, y, 2, 1,1, color, BGC);
		// print number
		LCD_Print(str_temp, x, y, 1, 1,1, color, BGC);
		// print unit with half space distance
		LCD_Print(unit, x+str_len*CHAR_CELL_WIDTH_FONT_1+HALF_SPACE_WIDTH_FONT_1, y, 1, 1,1, color, BGC);
		
	}
	
	
}

void paint_wait_screen(void)
{
	LCD_Cls(BGC);
	LCD_Print("Please wait...", 5, 50, 2, 1, 1, FGC, BGC);
}


void paint_info_line(char *line, _Bool update)
{
	if (!LVM.options->display_reversed)
	{
		if (!update) LCD_Print("                      ", X_PIL_2, Y_PIL_90, 1, 1, 1, FGC, BGC);  // clears line (not necessary if in update mode)
		LCD_Print(line, X_PIL_2, Y_PIL_90, 1, 1, 1, ERR, BGC);
	}
	else
	{
		if (!update) LCD_Print("                      ", XOffset + X_PIL_2, Y_PIL_90, 1, 1, 1, FGC, BGC);  // clears line (not necessary if in update mode)
		LCD_Print(line, XOffset + X_PIL_2, Y_PIL_90, 1, 1, 1, ERR, BGC);
	}
}


void paint_he_level(double he_level, double total_volume, _Bool print_Percentage)
{
	
	//	he_level in % / dewar_volume total volume in L
	//	uint8_t dewar_volume = 200; // volume in liters
	
	char temp[10];
	int16_t disp;
	
	#ifdef ili9341
	//LCD_Print(temp, xoff + X_PHL_2, Y_PHL_37, FONTNR_PHL_2, SCALE_PHL_4, SCALE_PHL_4, ERR, BGC);
	LCD_Print("   ",xoff+ (CENTER_LINE-((SCALE_PHL_4*3*FONT3_W)+FONT2_W*2)*0.5), Y_PHL_37, FONTNR_PHL_2, SCALE_PHL_4, SCALE_PHL_4, ERR, BGC);

	LCD_Print(" ", xoff + (CENTER_LINE-((SCALE_PHL_4*3*FONT3_W)+FONT2_W*2)*0.5)+(FONT3_W *SCALE_PHL_4+FONT3_W1 *SCALE_PHL_4*2), Y_PHL_61, 2, 2, 2, ERR, BGC);
	#endif

	#ifdef  DISP_3000
	//LCD_Print(temp, xoff + X_PHL_2, Y_PHL_37, FONTNR_PHL_2, SCALE_PHL_4, SCALE_PHL_4, ERR, BGC);
	LCD_Print("   ",xoff+ (CENTER_LINE-((SCALE_PHL_4*3*FONT3_W)+FONT2_W*2)*0.5), Y_PHL_37, FONTNR_PHL_2, SCALE_PHL_4, SCALE_PHL_4, ERR, BGC);

	LCD_Print(" ", xoff + (CENTER_LINE-((SCALE_PHL_4*3*FONT3_W)+FONT2_W*2)*0.5)+(3*FONT3_W*SCALE_PHL_4)-X_PHL_L_OFFSET, Y_PHL_61, 2, 2, 2, ERR, BGC);
	#endif


	
	
	if(he_level == errCode_TooHighRes){
		LCD_Print("      ", xoff + X_PHL_25, Y_PHL_37, 2, 2,2, ERR, BGC);
		LCD_Print("Cable?", xoff + X_PHL_25, Y_PHL_37, 2, 2,2, ERR, BGC);
	}
	else
	{
		LCD_Print("      ", xoff + X_PHL_25, Y_PHL_37, 2, 2,2, ERR, BGC);

		disp = round((he_level*total_volume) /100);
		if (he_level < 0) disp = 0;
		if (he_level > 100) disp = round(total_volume);

		sprintf(temp, "%d",disp);
		
		uint8_t digits = strlen(temp);
		
		
		//TODO this is terrible !!! redwork is in order  somehow font three has a different width for a single char (34), and for muutltiple chars (29)
		#ifdef ili9341
		//LCD_Print(temp, xoff + X_PHL_2, Y_PHL_37, FONTNR_PHL_2, SCALE_PHL_4, SCALE_PHL_4, ERR, BGC);
		LCD_Print(temp,xoff+ (CENTER_LINE-((SCALE_PHL_4*digits*FONT3_W)+FONT2_W*2)*0.5), Y_PHL_37, FONTNR_PHL_2, SCALE_PHL_4, SCALE_PHL_4, ERR, BGC);

		LCD_Print("l", xoff + (CENTER_LINE-((SCALE_PHL_4*digits*FONT3_W)+FONT2_W*2)*0.5)+(FONT3_W *SCALE_PHL_4+FONT3_W1 *SCALE_PHL_4*(digits-1)), Y_PHL_61, 2, 2, 2, ERR, BGC);
		#endif

		#ifdef  DISP_3000
		//LCD_Print(temp, xoff + X_PHL_2, Y_PHL_37, FONTNR_PHL_2, SCALE_PHL_4, SCALE_PHL_4, ERR, BGC);
		LCD_Print(temp,xoff+ (CENTER_LINE-((SCALE_PHL_4*digits*FONT3_W)+FONT2_W*2)*0.5), Y_PHL_37, FONTNR_PHL_2, SCALE_PHL_4, SCALE_PHL_4, ERR, BGC);

		LCD_Print("l", xoff + (CENTER_LINE-((SCALE_PHL_4*digits*FONT3_W)+FONT2_W*2)*0.5)+(digits*FONT3_W*SCALE_PHL_4)-X_PHL_L_OFFSET, Y_PHL_61, 2, 2, 2, ERR, BGC);
		#endif


		if (print_Percentage)
		{
			
			LCD_Print("             ", xoff + X_PHL_1, Y_PHL_100, 2, 1, 1, ERR, BGC);
			draw_double(he_level,xoff + X_PHL_2,Y_PHL_100,1,"% of",ERR, 2);
			if (he_level < 100) draw_double(total_volume,xoff + X_PHL_85,Y_PHL_100,0,"l",ERR, 2);
			else draw_double(total_volume,xoff + X_PHL_93,Y_PHL_100,0,"l",ERR, 2);
		}
	}
	
	// deletes voltage, current and resistance displayed in debug mode
	#ifdef ALLOW_DEBUG
	LCD_Print("               ", xoff + X_PHL_5, Y_PHL_90, 1, 1, 1, FGC, black);
	#endif
	

}

// paints time and pressure
void paint_time_pressure(struct tm ltime, double lpress, _Bool update)
{


	char temp[15];

	if (!update) LCD_Print("               ", xoff + X_PTP_2, Y_PTP_20, 2, 1, 1, FGC, BGC);  // clears line (not necessary if in update mode)
	
	if ((ltime.tm_min< 10)) sprintf(temp,"%i:0%i  ", ltime.tm_hour, ltime.tm_min);
	if (!(ltime.tm_min < 10)) sprintf(temp,"%i:%i  ", ltime.tm_hour, ltime.tm_min);
	LCD_Print(temp, xoff + X_PTP_2, Y_PTP_20, 2, 1, 1, ERR, BGC);



	if (xbee.CoordIdChanged)
	{
		LCD_Print("              ",X_PTP_COORDINATOR +xoff-14*FONT1_W,Y_PTP_COORDINATOR,1,1,1,FGC,BGC);
	}
		
	uint16_t color = FGC;
	
	if (CHECK_ERROR(NETWORK_ERROR))
	{
		color = red;
	}

	//draw_double_without_erasing(1013.3, xoff + X_PTP_60, Y_PTP_20, 0, "mbar ", ERR, 2);
	if (lpress > 0)
	{
		draw_double_without_erasing(lpress, xoff + X_PTP_60, Y_PTP_20, 0, "mbar ", ERR, 2);
		
		/*		dtostrf(lpress,4,0,temp);
		strcat(temp,"mbar");
		LCD_Print(temp, 60, 20, 2, 1, 1, ERR, BGC);
		*/
	}
	

	
	if (global_mode.netstat == online)
	{
		
		switch (xbee.netstat)
		{
			case ONLINE:
			LCD_conn_Stregth(0,xbee.RSSI,X_SIGSTRENGTH_INDICATOR+xoff,Y_SIGSTRENGTH_INDICATOR,green);
			break;
			
			case NO_NETWORK:
			LCD_conn_Stregth(1,xbee.RSSI,X_SIGSTRENGTH_INDICATOR+xoff,Y_SIGSTRENGTH_INDICATOR,red);
			break;
			
			case NO_SERVER:
			LCD_conn_Stregth(0,xbee.RSSI,X_SIGSTRENGTH_INDICATOR+xoff,Y_SIGSTRENGTH_INDICATOR,orange);
			break;
		}
	}
	
		LCD_Print(xbee_get_coordID(),X_PTP_COORDINATOR +xoff-strlen(xbee_get_coordID())*FONT1_W,Y_PTP_COORDINATOR,1,1,1,color,BGC);
	

		if (!update)
		{
		
		//autofill indicator
		uint8_t boxheight = AUTOFILL_BOX_WIDTH;
		uint16_t x0 = X_AUTOFFILL_INDICATOR+xoff;
		uint16_t y0 = Y_AUTOFFILL_INDICATOR;
		if (LVM.vars->auto_fill_enabled)
		{
			LCD_Box(x0,y0,x0 +boxheight,y0+boxheight,green);
		}
		else
		{
			LCD_Box(x0,y0,x0+boxheight,y0+boxheight,green);
			LCD_Box(x0+1,y0+1,x0+boxheight-1,y0+boxheight-1,BGC);
		}
		}
		

	
	
	/*	  draw_double(lpress, 55, 20, 0, " mbar ", ERR);
	else draw_double(lpress, 55, 20, 0, " mbar", ERR);
	*/
}


///paints static battery indicator with range 0-100%
void paint_batt(uint8_t batt, uint8_t critical_batt)
{
	if(batt > 100) batt = 100;
	
	
	#ifdef ili9341
	float bar_scale = X_PBATT_140-(X_PBATT_38+2);
	float batt_fl   = (float) batt;
	uint8_t barlen = (uint8_t)((batt_fl / 100) * bar_scale);
	
	#endif
	
	#ifdef DISP_3000
	uint8_t barlen = batt;
	#endif
	
	//draw_int(barlen,0,0,"",red);
	//draw_int(batt,0,20,"",red);
	//clear screen
	LCD_Box(xoff+X_PBATT_38,Y_PBATT_119,xoff+X_PBATT_140,Y_PBATT_131,BGC);
	
	//draw borders
	LCD_Rect(xoff+X_PBATT_38,Y_PBATT_120,xoff+X_PBATT_140,Y_PBATT_130,0, FGC);
	
	//if lower then critBatt draw in ERROR-color
	(batt < critical_batt)?
	LCD_Box(xoff+X_PBATT_39,Y_PBATT_121,barlen+xoff+X_PBATT_39,Y_PBATT_129,red)
	:	LCD_Box(xoff+X_PBATT_39,Y_PBATT_121,barlen+xoff+X_PBATT_39,Y_PBATT_129,green);
	
	LCD_Print(STR_BATT,xoff+X_PBATT_1,Y_PBATT_118,2,1,1,FGC, BGC);
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
	LCD_Rect(x-X_PPB_OFFS_2,y-Y_PPB_OFFS_2,x+X_PPB_OFFS_100,y+Y_PPB_OFFS_7, 0, FGC);	//draw borders
	
	for(uint8_t i=0;i<progress;i++)		//draw progress bar
	{
		LCD_Box(x+i*X_PPB_OFFS_10,y,x+i*X_PPB_OFFS_10+X_PPB_OFFS_8,y+Y_PPB_OFFS_5,FGC);
	}
	
	//draw xx%
	utoa((progress)*10,temp,10);
	strcat(temp, "%");
	
	//clear screen 4 chars... 100%
	LCD_Print("    ", x+X_PPB_OFFS_103, y-Y_PPB_OFFS_1, FONT_PPB_1, 1,1, FGC, BGC);
	LCD_Print(temp, x+X_PPB_OFFS_103, y-Y_PPB_OFFS_1, FONT_PPB_1, 1,1, FGC, BGC);
}

void clear_progress_bar(uint8_t x, uint8_t y)
{
	//LCD_Box(x-3,y-3,x+101,y+8, BGC);  nur balken
	
	LCD_Box(x-3,y-3,x+5+X_PPB_OFFS_100+(4*Percentage_Font_Cellwidth),y+3+Y_PPB_OFFS_5,BGC);
	//LCD_Box(x-3,y-3,x+141,y+8, BGC); old
}


void draw_current_wait_time(uint8_t x, uint8_t y, uint16_t sec_must, uint16_t secs_is, unsigned int color)
{
	static uint16_t last_value = 0;
	if (last_value == secs_is) return;
	
	last_value = secs_is;
	
	char temp[10];
	uint8_t mins = (sec_must - secs_is)/60;
	uint8_t rest_secs = (sec_must - secs_is) % 60;
	
	if (rest_secs <10) sprintf(temp,"%i:0%i", mins, rest_secs);
	else sprintf(temp,"%i:%i", mins, rest_secs);

	LCD_Print(temp, x, y, 1, 1,1, color, BGC);
}

///paints buttons on the left of the screen
void paint_buttons(char *top, char *bottom, uint8_t type)
{
	
	if (!LVM.options->display_reversed)
	{
		//clear screen
		
		
		LCD_Box(X_PB_144,Y_PB_Box_0,X_PB_176,Y_PB_131,BGC);
		
		//draw captions according to type
		switch(type)
		{
			case 1:
			LCD_Print("+",X_PB_150,Y_PB_25,2,2,2,FGC, BGC);
			LCD_Print("-",X_PB_151,Y_PB_77,2,2,2,FGC, BGC);
			break;
			case 2:
			LCD_Print("<-",X_PB_145,Y_PB_61,1,1,1,FGC, BGC);
			LCD_Print("->",X_PB_161,Y_PB_61,1,1,1,FGC, BGC);
			break;
			case 3:
			LCD_Print("+",X_PB_150,Y_PB_25,2,2,2,FGC, BGC);
			LCD_Print("<-",X_PB_145,Y_PB_61,1,1,1,FGC, BGC);
			LCD_Print("->",X_PB_161,Y_PB_61,1,1,1,FGC, BGC);
			LCD_Print("-",X_PB_151,Y_PB_77,2,2,2,FGC, BGC);
			break;
			default://no captions other then oben unten
			break;
		}
		

		LCD_Print(top,X_PB_159-(strlen(top)*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
		LCD_Print(bottom,X_PB_159-(strlen(bottom)*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);


		//box
		LCD_vline(X_PB_144,Y_PB_1,LEN_PB_129,FGC); // |
		LCD_vline(X_PB_174,Y_PB_1,LEN_PB_129,FGC); // |
		LCD_hline(X_PB_144,Y_PB_1,LEN_PB_30,FGC);  // -
		LCD_hline(X_PB_144,Y_PB_130,LEN_PB_30,FGC);// -
		
		//separators
		LCD_hline(X_PB_144,Y_PB_26,LEN_PB_30,FGC); //-
		LCD_hline(X_PB_144,Y_PB_52,LEN_PB_30,FGC); //-
		LCD_hline(X_PB_144,Y_PB_78,LEN_PB_30,FGC); //-
		LCD_hline(X_PB_144,Y_PB_104,LEN_PB_30,FGC);//-
		LCD_vline(X_PB_159a,Y_PB_52,LEN_PB_26,FGC); //|
		
		
	}
	else
	{
		//clear screen
		LCD_Box(X_PB_Box_0,Y_PB_Box_0,X_PB_32,Y_PB_131,BGC);
		
		//draw captions according to type
		switch(type)
		{
			case 1:
			LCD_Print("+",X_PB_6,Y_PB_25,2,2,2,FGC, BGC);
			LCD_Print("-",X_PB_7,Y_PB_77,2,2,2,FGC, BGC);
			break;
			case 2:
			LCD_Print("<-",X_PB_1,Y_PB_61,1,1,1,FGC, BGC);
			LCD_Print("->",X_PB_17,Y_PB_61,1,1,1,FGC, BGC);
			break;
			case 3:
			LCD_Print("+",X_PB_6,Y_PB_25,2,2,2,FGC, BGC);
			LCD_Print("<-",X_PB_1,Y_PB_61,1,1,1,FGC, BGC);
			LCD_Print("->",X_PB_17,Y_PB_61,1,1,1,FGC, BGC);
			LCD_Print("-",X_PB_7,Y_PB_77,2,2,2,FGC, BGC);
			break;
			default://no captions other then oben unten
			break;
		}



		LCD_Print(top,X_PB_15-(strlen(top)*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
		LCD_Print(bottom,X_PB_15-(strlen(bottom)*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);




		
		
		//box
		LCD_vline(X_PB_0,Y_PB_1,LEN_PB_129,FGC); // |
		LCD_vline(X_PB_30,Y_PB_1,LEN_PB_129,FGC);// |
		LCD_hline(X_PB_0,Y_PB_1,LEN_PB_30,FGC);  // -
		LCD_hline(X_PB_0,Y_PB_130,LEN_PB_30,FGC);// -
		
		//separators
		LCD_hline(X_PB_0,Y_PB_26,LEN_PB_30,FGC); // -
		LCD_hline(X_PB_0,Y_PB_52,LEN_PB_30,FGC); // -
		LCD_hline(X_PB_0,Y_PB_78,LEN_PB_30,FGC); // -
		LCD_hline(X_PB_0,Y_PB_104,LEN_PB_30,FGC);// -
		LCD_vline(X_PB_15a,Y_PB_52,LEN_PB_26,FGC);// |
		
	}
}


///paints or updates main window depending on update parameter
void paint_main(struct tm ltime, _Bool netstat, _Bool update)
{
	//void paint_main(  TimeBuff ltime, _Bool offline, _Bool update)
	
	
	if (!update)
	{
		LCD_Cls(BGC);
		//device id
		LCD_Print(STR_VESSEL_NR, xoff+X_PM_1, Y_PM_2, 2, 1,1, FGC, BGC);
		
		
		//position
		LCD_Print(LVM.vars->device_pos, xoff+X_PM_100, Y_PM_2, 2, 1,1, ERR, BGC);
		

		
		

		//HE-Level
		paint_he_level(LVM.vars->he_level, LVM.options->total_volume, 1);
		
		
		//time and pressure
		paint_time_pressure(ltime, LVM.vars->pressure_level, update);

		paint_buttons(" ","opts",0);
		//		LCD_Print("Fill",148,31,1,1,2,FGC, BGC);
		if (netstat) {
			LCD_Print(LVM.vars->Device_ID_Str, xoff+X_PM_65, Y_PM_2, 2, 1,1, ERR, BGC);
			(!LVM.options->display_reversed)? LCD_Print(STR_Conn,X_PM_148,Y_PM_83,FONTNR_PM_1,1,2,FGC, BGC) : LCD_Print(STR_Conn,X_PM_4,Y_PM_83,FONTNR_PM_1,1,2,FGC, BGC);
			}else{
			LCD_Print(STR_OFF, xoff+X_PM_65, Y_PM_2, 2, 1,1, ERR, BGC);
		}
	}
	else
	{
		paint_info_line("",0);
		clear_progress_bar(xoff+X_M_5, Y_M_105);


		//HE-Level
		//LCD_Print("   ", xoff+X_PM_25, Y_PM_37, 2, 4,4, ERR, BGC);
		paint_he_level(LVM.vars->he_level, LVM.options->total_volume, 1);
		
		//time and pressure
		paint_time_pressure(ltime, LVM.vars->pressure_level, update);

		
	}
	paint_batt(LVM.vars->batt_level, LVM.options->critical_batt);
	

}




///paints or updates filling position login window
void paint_start_filling(posType * PosModel, _Bool update)
{
	if (!update)
	{
		LCD_Cls(BGC);
		LCD_Print(STR_FILL, xoff+X_PSF_52, Y_PSF_2, 2, 1,1, ERR, BGC);
		LCD_Print(STR_ENTER_POS, xoff+X_PSF_5, Y_PSF_35, 2, 1, 1, FGC, BGC);
		
		
		paint_buttons("canc","ok", 3);

	}
	char* current_Pos_Str = PosModel->Strings[PosModel->active_Pos_Str];
	uint8_t current_Str_len = PosModel->StrLen[PosModel->active_Pos_Str];

	LCD_Box(X_PSF_zero+xoff-(FONT2_W*3*2),Y_PSF_70,X_PSF_zero+10+xoff+FONT2_W*3*2,Y_PSF_70+FONT2_H*2,BGC);
	
	
	if (PosModel->letters_and_numbers) // Letters and Number divided by half space
	{
		char NumberBuff[10];
		uint8_t current_Range_Number = PosModel->RangeNums[PosModel->active_Pos_Str][PosModel->active_range_Number];
		itoa(current_Range_Number, NumberBuff, 10);
		if (PosModel->digit_on) //digit Highlighted
		{
			LCD_Print(current_Pos_Str,xoff+X_PSF_zero-(current_Str_len*FONT2_W*2),Y_PSF_70,2,2,2,FGC,BGC);
			LCD_Print(NumberBuff,xoff+X_PSF_zero+10,Y_PSF_70,2,2,2,ERR,BGC);
		}
		else // Letters highlighted
		{
			LCD_Print(current_Pos_Str,xoff+X_PSF_zero-(current_Str_len*FONT2_W*2),Y_PSF_70,2,2,2,ERR,BGC);
			LCD_Print(NumberBuff,xoff+X_PSF_zero+10,Y_PSF_70,2,2,2,FGC,BGC);
		}
		
		
	}
	else
	{ //Just centered string
		LCD_Print(current_Pos_Str,xoff+((LCD_WIDTH-XOFFSET_32)/2)-(FONT2_W*strlen(current_Pos_Str)),Y_PSF_70,2,2,2,FGC,BGC);
	}

}

///paints or updates filling window
void paint_filling(struct tm ltime, _Bool netstat, _Bool update, _Bool draw_wait)
{
	if (!update)
	{
		LCD_Cls(BGC);
		//device id
		
		LCD_Print(STR_VESSEL_NR, xoff+X_PM_1, Y_PM_2, 2, 1,1, FGC, BGC);
		
		
		if (netstat)
		{
			LCD_Print(LVM.vars->Device_ID_Str, xoff+X_PM_65, Y_PM_2, 2, 1,1, ERR, BGC);
		}else
		{
			LCD_Print(STR_OFF, xoff+X_PM_65, Y_PM_2, 2, 1,1, ERR, BGC);
		}

		
		//position
		LCD_Print(LVM.vars->device_pos, xoff+X_PM_100, Y_PM_2, 2, 1,1, ERR, BGC);
		


		//HE-Level
		paint_he_level(LVM.vars->he_level, LVM.options->total_volume, 1);
		
		//time and pressure
		paint_time_pressure(ltime, LVM.vars->pressure_level, update);
		
		paint_buttons("stop","more", 0xff);
		paint_batt(LVM.vars->batt_level, LVM.options->critical_batt);

		//LCD_Print("stop",148,5,1,1,2,FGC, BGC);
		//LCD_Print("more",148,20,1,1,2,FGC, BGC);
	}
	else {


		//HE-Level
		paint_he_level(LVM.vars->he_level, LVM.options->total_volume, 0);
		
		//time and pressure
		paint_time_pressure(ltime, LVM.vars->pressure_level, update);

		clear_progress_bar(xoff+X_M_5, Y_M_105);
		
	}
	if (draw_wait)
	{
		
		uint16_t helpvariable;
		
		if (LVM.vars->auto_fill_started)
		{
			LCD_Print("Autofill", xoff+X_M_2, Y_M_105, 1, 1, 1, green, BGC);
		}
		else
		{
			helpvariable = round((double) LVM.vars->fill_meas_counter * LVM.options->transmit_fast / 60);
			if (LVM.options->transmit_fast_sec) sprintf(LVM.temp->string, FILL_WAITING_LABEL, helpvariable);
			else sprintf(LVM.temp->string, FILL_WAITING_LABEL, LVM.vars->fill_meas_counter*LVM.options->transmit_fast);

			LCD_Print(LVM.temp->string, xoff+X_M_2, Y_M_105, 1, 1, 1, FGC, BGC);
		}
	}

}




char* itoc(char * letterstr,uint8_t letter_num){
	if (letter_num < 10)
	{
		letter_num += '0';
	}
	else
	{
		#ifdef ili9341
		letter_num += 54;
		#endif // ili9341

		#ifdef DISP_3000
		letter_num += 55;
		#endif
	}
	
	sprintf(letterstr,"%c",letter_num);
	return letterstr;
}





void resetPosModel(posType *PosModel){
	PosModel->active_Pos_Str = 1;
	PosModel->active_range_Number = 1;

	PosModel->letters_and_numbers = false;
	PosModel->digit_on = false;

	return;
}


// Update position letters/digits on filling login
_Bool update_filling_pos(posType *PosModel,char *pos )
{
	char Dialog_String[100];

	_Bool fill_pos_login = true;
	resetPosModel(PosModel);
	PosModel->letters_and_numbers = (PosModel->StrLen[PosModel->active_Pos_Str] != 0)? true : false;
	
	paint_start_filling(PosModel,0);
	
	while(fill_pos_login)
	{
		switch(keyhit_cont())
		{
			case KEY_TOP_S5: //CANCEL
			display_on();
			return false;
			break;
			
			
			case KEY_UP_S6: // ++RANGE/POS
			if (PosModel->digit_on){
				//Increase active Range Number
				PosModel->active_range_Number++;
				
				if(PosModel->RangeNums[PosModel->active_Pos_Str][PosModel->active_range_Number] == END){
					PosModel->active_range_Number--;
					break;
				}
				
			}
			else
			{
				//Increase active Pos String Index
				PosModel->active_Pos_Str++;
				if (PosModel->Strings[PosModel->active_Pos_Str][0] == '\r')	{
					PosModel->active_Pos_Str--;
					break;
				}

				//Reset active rangenumber
				PosModel->active_range_Number = 1;
				// StringLen is zero if there are no range entrys
				PosModel->letters_and_numbers = (PosModel->StrLen[PosModel->active_Pos_Str] != 0)? true : false;
			}
			
			paint_start_filling(PosModel,1);
			break;
			
			
			case KEY_LEFT_S7: // POS <--> RANGES
			if (PosModel->letters_and_numbers)
			{
				PosModel->digit_on = (PosModel->digit_on == false)?true : false;
			}
			
			paint_start_filling(PosModel,1);
			break;
			
			
			case KEY_RIGHT_S8: // POS <--> RANGES
			if (PosModel->letters_and_numbers)
			{
				PosModel->digit_on = (PosModel->digit_on == false)?true : false;
			}
			paint_start_filling(PosModel,1);
			break;
			
			
			case KEY_DOWN_S9: // --RANGE/POS
			if (PosModel->digit_on){
				//Decrease active Range Number
				PosModel->active_range_Number--;
				
				if(PosModel->RangeNums[PosModel->active_Pos_Str][PosModel->active_range_Number] == END){
					PosModel->active_range_Number++;
					break;
				}
				
			}
			else
			{
				//Decrease active Pos String Index
				PosModel->active_Pos_Str--;
				
				if (PosModel->Strings[PosModel->active_Pos_Str][0] == '\r')	{
					PosModel->active_Pos_Str++;
					break;
				}
				
				//Reset active rangenumber
				PosModel->active_range_Number = 1;
				// StringLen is zero if there are no range entrys
				PosModel->letters_and_numbers = (PosModel->StrLen[PosModel->active_Pos_Str] != 0)? true : false;
				
			}
			paint_start_filling(PosModel,1);
			break;
			
			
			case KEY_BOT_S10: ;//OK
			char  NumberBuff[10] ;
			char *current_Pos_String = PosModel->Strings[PosModel->active_Pos_Str];
			
			
			
			if(!PosModel->letters_and_numbers)
			{
				sprintf(Dialog_String,"Do you really want\nto save the\nfilling position %s?",current_Pos_String);
			}
			else
			{
				
				uint8_t current_Range_Number = PosModel->RangeNums[PosModel->active_Pos_Str][PosModel->active_range_Number];
				itoa(current_Range_Number, NumberBuff, 10);
				
				sprintf(Dialog_String,"Do you really want\nto save the\nfilling position %s%d ?",current_Pos_String,current_Range_Number);
			}
			
			if(LCD_Dialog("Position", Dialog_String, D_FGC, D_BGC, POS_TIMEOUT_TIME))
			{
				strcpy(pos,current_Pos_String);
				if (PosModel->letters_and_numbers)
				{
					strcat(pos,NumberBuff);
				}
				fill_pos_login = false;

			}
			else
			{
				paint_start_filling(PosModel,0);
			}
			display_on();
			_delay_ms(200);
			
			
			break;
			
			default:
			break;
		}
		
	}

	return true;
}



void paint_Dev_ID(void){
	LCD_Cls(BGC);
	LCD_Print(STR_CONNECTION, xoff+ X_PE_4 , Y_PE_2, 2, 1,1, ERR, BGC);
	LCD_Print(STR_TO_SERVER, xoff+X_PE_4, Y_PE_15, 2, 1,1, ERR, BGC);
	LCD_Print(STR_ENTER_VESSEL_ID, xoff+X_PE_4, Y_PE_40, 2, 1, 1, FGC, BGC);
	paint_buttons(STR_EXIT,STR_OK, 3);
}

void paint_get_password(void){
	LCD_Cls(BGC);
	LCD_Print("Options", xoff+X_PE_42, Y_PE_2, 2, 1,1, ERR, BGC);
	LCD_Print("Enter Password:", xoff+X_PE_5, Y_PE_35, 2, 1, 1, FGC, BGC);
	paint_buttons(STR_CANCEL,STR_OK, 3);
}





///paints given diagnostics page
void paint_diag(uint8_t page)
{
	LCD_Cls(BGC);
	switch(page)
	{
		case 1:
		LCD_Print(STR_DIAG_1of2, xoff+X_PD_35, Y_PD_2, 2, 1,1, ERR, BGC);
		
		LCD_Print(STR_RESISATANCE, xoff+X_PD_2, Y_PD_20, 2, 1,1, FGC, BGC);
		LCD_Print(STR_HELIUM, xoff+X_PD_2, Y_PD_40, 2, 1,1, FGC, BGC);
		LCD_Print(STR_BATTERY, xoff+X_PD_2, Y_PD_60, 2, 1,1, FGC, BGC);
		LCD_Print(STR_PRESSURE, xoff+X_PD_2, Y_PD_80, 2, 1,1, FGC, BGC);
		LCD_Print(STR_CURRENT, xoff+X_PD_2, Y_PD_100, 2, 1,1, FGC, BGC);
		
		paint_buttons("esc", "puls", 2);
		(!LVM.options->display_reversed)? LCD_Print("cal",X_PD_148a,Y_PD_31,FONTNR_PD_1,1,2,FGC, BGC) : LCD_Print("cal",X_PD_4a,Y_PD_31,FONTNR_PD_1,1,2,FGC, BGC);
		(!LVM.options->display_reversed)? LCD_Print("curr",X_PD_148b,Y_PD_82,FONTNR_PD_1,1,2,FGC, BGC) : LCD_Print("curr",X_PD_4b,Y_PD_82,FONTNR_PD_1,1,2,FGC, BGC);
		break;
		
		case 2:
		LCD_Print(STR_DIAG_2of2, xoff+X_PD_35, Y_PD_2, 2, 1,1, ERR, BGC);
		
		LCD_Print(STR_ADCV, xoff+X_PD_2, Y_PD_20, 2, 1,1, FGC, BGC);
		LCD_Print(STR_ADCI, xoff+X_PD_2, Y_PD_40, 2, 1,1, FGC, BGC);
		LCD_Print(STR_I , xoff+X_PD_2, Y_PD_60, 2, 1,1, FGC, BGC);
		LCD_Print(STR_U, xoff+X_PD_2, Y_PD_80, 2, 1,1, FGC, BGC);
		LCD_Print(STR_US , xoff+X_PD_2, Y_PD_100, 2, 1,1, FGC, BGC);
		
		paint_buttons("esc", "puls", 2);
		//LCD_Print("cal",148,31,1,1,2,FGC, BGC);
		//LCD_Print("curr",148,82,1,1,2,FGC, BGC);
		break;

		
		
		
		
	}
}

///maps adc value to graph y-axis with range 0-points
// zero line is given as yzero_line
///top point of the y-axis is given as yzeroline-points
// max_value is mapped to maximum==points
///used for adc-graph
uint8_t toY(double value, uint16_t max_value, uint16_t points, uint8_t yzero_line)
{
	//return ((y+50)-ceil((adcVal)/(1023/50.0)));
	
	if ((uint16_t)value > max_value)
	{
		value = max_value;
	}
	
	if(value < 0){
		value = 0 ;
	}
	
	
	
	
	uint8_t result = ((yzero_line)-ceil(points*value/max_value));
	

	return result;
}

//=========================================================================
// Display backlight toggle
//=========================================================================
_Bool display_on(void)
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
		LCD_Print("on ", xoff+5, 20, 2, 1, 1, ERR, BGC);
		
		#endif
		return false;
	}
}

//=========================================================================
// Init screen with messages appearing one under the other
//=========================================================================

// Print line of InitScreen
// if FirstLine is 1 clear page and start with first line
void InitScreen_AddLine(const char* Text, const char FirstLine)
{
	if (FirstLine == 1)
	{
		InitScreenNextLine = 1;
	}
	if (InitScreenNextLine == 1) LCD_Cls(BGC);  // Clear Screen, before first line is written

	if (FirstLine == 2)
	{
		--InitScreenNextLine;
		LCD_Print("                     ", X_IA_2, InitScreenNextLine * InitScreenLineFeed, InitScreenFontNr, InitScreenXScale, InitScreenYScale, ERR, BGC);
	}

	
	LCD_Print(Text, X_IA_2, InitScreenNextLine * InitScreenLineFeed, InitScreenFontNr, InitScreenXScale, InitScreenYScale, ERR, BGC);
	++InitScreenNextLine;
	if (InitScreenNextLine > InitScreenMaxNoOfLines)
	{
		InitScreenNextLine = 1;
		_delay_ms(1000);		  // wait until next page is displayed
	}
	_delay_ms(500);

}

//yes no dialog for Landscape
//output a message with title and text
//at last function waits for keypress
_Bool LCD_Dialog(char *title, char *text, unsigned int BackColor, unsigned int ForeColor,uint8_t timeout)  {
	uint8_t x0,x,y,i,len;
	LCD_Cls(BackColor);

	char temp[2];
	temp[1] = '\0';

	switch (Orientation)
	{
		case Landscape:
		LCD_Box(X_LD_10,Y_LD_25,X_LD_160,Y_LD_120,ForeColor);
		LCD_Print(title,LD_MID_LANDSCAPE-(strlen(title)*LD_HALF_CHAR_WIDTH),Y_LD_1,2,1,1,ForeColor,BackColor);
		x0 = X_LD_13;
		break;
		case Landscape180:
		LCD_Box(X_LD_17,Y_LD_25,X_LD_166,Y_LD_120,ForeColor);
		LCD_Print(title,LD_MID_LANDSCAPE180-(strlen(title)*LD_HALF_CHAR_WIDTH),Y_LD_1,2,1,1,ForeColor,BackColor);
		x0 = X_LD_20;
		break;
		default:
		LCD_Box(X_LD_10,Y_LD_25,X_LD_120,Y_LD_165,ForeColor);
		LCD_Print(title,65-(strlen(title)*LD_HALF_CHAR_WIDTH),Y_LD_1,2,1,1,ForeColor,BackColor);
		x0 = X_LD_13;
		break;
	}

	//for Landscape only
	x = x0;
	y = Y_LD_28;


	len = strlen(text);
	for(i=0;i<len;i++)  {
		if(text[i] != '\n')
		{
			temp[0] = text[i];
			LCD_Print(temp,x,y,1,1,1,BackColor,ForeColor);
			x += CHAR_CELL_WIDTH_FONT_1;
			if(x >= LD_Line_Length)
			{
				if(y>=LD_MAX_Height)
				break;
				y += LD_Line_Height;
				x = x0;
				continue;
			}
		}
		else  {
			if(y >= LD_MAX_Height)
			break;
			y += LD_Line_Height;
			x = x0;
			continue;
		}
	}




	if(!LVM.options->display_reversed)
	{
		LCD_Print("Y",X_LD_165,Y_LD_1,2,1,1,ForeColor,BackColor);
		LCD_Print("e",X_LD_165,Y_LD_17,2,1,1,ForeColor,BackColor);
		LCD_Print("s",X_LD_165,Y_LD_33,2,1,1,ForeColor,BackColor);

		LCD_Print("N",X_LD_165,Y_LD_99,2,1,1,ForeColor,BackColor);
		LCD_Print("o",X_LD_165,Y_LD_115,2,1,1,ForeColor,BackColor);
		}else{
		LCD_Print("Y",X_LD_3,Y_LD_1,2,1,1,ForeColor,BackColor);
		LCD_Print("e",X_LD_3,Y_LD_17,2,1,1,ForeColor,BackColor);
		LCD_Print("s",X_LD_3,Y_LD_33,2,1,1,ForeColor,BackColor);

		LCD_Print("N",X_LD_3,Y_LD_99,2,1,1,ForeColor,BackColor);
		LCD_Print("o",X_LD_3,Y_LD_115,2,1,1,ForeColor,BackColor);
	}





	while (!(keyhit_block() == 0));  // wait until no key is pressed

	// accept a new key to be pressed
	ready_for_new_key();



	// dialouge is only timed if timeout > 0
	if(timeout){
		set_timeout(0, TIMER_3, RESET_TIMER);
		set_timeout(timeout, TIMER_3, USE_TIMER);
	}


	while(1)
	{	//check for pressed Key
		if (timeout)
		{
			if(!set_timeout(0,TIMER_3, USE_TIMER) )  //return false after shutdown_timout if no key was pressed
			{
				return 0;
			}
		}

		switch(keyhit_block())
		{
			case KEY_TOP_S5:
			set_timeout(0, TIMER_3, RESET_TIMER);
			return 1;
			case KEY_UP_S6:
			set_timeout(0, TIMER_3, RESET_TIMER);
			return 1;
			case KEY_DOWN_S9:
			set_timeout(0, TIMER_3, RESET_TIMER);
			return 0;
			case KEY_BOT_S10:
			set_timeout(0, TIMER_3, RESET_TIMER);
			return 0;
			default:   ready_for_new_key();
		}
	}
}
