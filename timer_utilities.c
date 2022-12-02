// Timer_Utilities.c - Copyright 2016, HZB, ILL/SANE & ISIS
#define RELEASE_TIMERUTILS 1.00

// HISTORY -------------------------------------------------------------
// 1.00 - First release on March 2016
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "disp/display_lib.h"

#include "keyboard.h"
#include "timer_utilities.h"
#ifdef DISP_3000
#include "StringPixelCoordTable.h"
#endif
#ifdef ili9341
#include "StringPixelCoordTable_ili9341.h"
//#include "StringPixelCoordTable.h"
#endif

volatile uint32_t t8_0_overflow	= 0;
volatile uint16_t t16_1_overflow = 65500;
volatile uint16_t t8_2_overflow	= 0;


//volatile _Bool t_timeout=false;

void init_0_timer8(void)
{
	TIMSK0 |= (1<<TOIE0);		//enable Timer0 interrupts
}

void timer8_0_start(void)
{
	//enable module
	//PRR0 |= (0<<PRTIM2);
	
	//Frequency: F_CPU / 1024
	TCCR0B |= (1<<CS00)|(0<<CS01)|(1<<CS02);
}

void timer8_0_stop(void)
{
	//stops the counter and resets counter value
	TCCR0B &= ~((1<<CS00)|(1<<CS01)|(1<<CS02));
	TCNT0 = 0;
	t8_0_overflow=0;
	
	//disable module
	//PRR0 |= (1<<PRTIM2);
}

//REF TIMER
void init_1_timer16(void)
{
	//CTCmode
	OCR1A = SEC; 			//1sec
	TIMSK1 |= (1<<OCIE1A);
	TCCR1B = (1<<WGM12);
	
	timer16_1_start();
	//normal mode
	//TIMSK1 |= (1<<TOIE1);		//enable Timer1 interrupts
}

void timer16_1_start(void)
{
	//enable module
	//PRR0 |= (0<<PRTIM1);
	
	//Frequency: F_CPU / 1024
	TCCR1B |= (1<<CS10)|(1<<CS12);
}

void timer16_1_stop(void)
{
	//stops the counter and resets counter value
	TCCR1B &= ~((1<<CS10)|(1<<CS11)|(1<<CS12));
	TCNT1 = 0;
	t16_1_overflow=0;
	
	//disable module
	//PRR0 |= (1<<PRTIM1);
}

void init_2_timer8(void)
{
	TIMSK2 |= (1<<TOIE2);		//enable Timer2 interrupts
}

void timer8_2_start(void)
{
	//enable module
	//PRR0 |= (0<<PRTIM2);
	
	//Frequency: F_CPU / 1024
	TCCR2B |= (1<<CS20)|(0<<CS21)|(1<<CS22);
}

void timer8_2_stop(void)
{
	//stops the counter and resets counter value
	TCCR2B &= ~((1<<CS20)|(1<<CS21)|(1<<CS22));
	TCNT2 = 0;
	t8_2_overflow=0;
	
	//disable module
	//PRR0 |= (1<<PRTIM2);
}

ISR(TIMER0_OVF_vect)
{
	t8_0_overflow ++;
}

ISR(TIMER1_COMPA_vect)
{
	t16_1_overflow ++;
	count_t_elapsed ++;
	system_tick();
}

ISR(TIMER2_OVF_vect)
{
	t8_2_overflow ++;
}


///sets, checks and resets given Timer
///use only 16bit_number-1 as max waittime or tweak implementation
///returns 1 when running, 0 when stopped
uint32_t set_timeout(uint16_t sec, uint8_t timer, uint8_t reset)
{
	static uint16_t timer0_waittime = 0;
	static uint16_t timer1_waittime = 0;
	static uint16_t timer2_waittime = 0;
	static uint16_t timer3_waittime = 0;
	static uint16_t timer4_waittime = 0;
	static uint16_t timer5_waittime = 0;
	static uint16_t timer6_waittime = 0;

	
	static _Bool ovf_flag1 = false;
	static _Bool ovf_flag2 = false;
	static _Bool ovf_flag3 = false;
	static _Bool ovf_flag4 = false;
	static _Bool ovf_flag5 = false;
	//static _Bool ovf_flag6 = false;
	
	
	static uint16_t start_time = 0;		//nur fuer timer 4/ filling
	uint16_t current_time = t16_1_overflow;
	//draw_int(current_time, 90, 80, "c", ERR);
	
	switch(timer)
	{
		case TIMER_0:	//measurement/ sec wert in ms!
		if(reset)
		{
			timer8_0_stop();
			timer0_waittime = 0;
		}
		else {
			if(!timer0_waittime && sec)				// Init. timer
			{
				timer0_waittime = sec;				//setzen
				timer8_0_start();					//starten
			}
			
			if(t8_0_overflow >= ((timer0_waittime)/OVERFLOW_IN_MS_8_BIT))
			{
				timer8_0_stop();
				timer0_waittime = 0;
				return 0;
			}
			else if(t8_0_overflow) return t8_0_overflow; else return 1;
			//else return 1;
		}
		break;
		case TIMER_1:	// Time interval between regular network connections (in seconds)
		if(reset)
		{
			timer1_waittime = 0;
			ovf_flag1=false;
		}
		else {
			if(!timer1_waittime && sec)				//0 -> Timer l�uft nicht
			{
				uint16_t diff_time = UINT16_MAX - current_time;
				
				if (sec > diff_time) 	{timer1_waittime = sec - diff_time; ovf_flag1 = true;}
				else					timer1_waittime = current_time + sec;
			}
			if(ovf_flag1 && (current_time <= timer1_waittime)) ovf_flag1 = false;
			if((current_time >= timer1_waittime) && (!ovf_flag1))
			{
				timer1_waittime = 0;
				return 0;
			}
			else return 1;
		}
		break;
		case TIMER_2:	// Display timer (in seconds) to manage the standby
		if(reset)
		{
			timer2_waittime = 0;
			ovf_flag2=false;
		}
		else {
			if(!timer2_waittime && sec)				//0 -> Timer l�uft nicht
			{
				uint16_t diff_time = UINT16_MAX - current_time;
				
				if (sec > diff_time) 	{timer2_waittime = sec - diff_time; ovf_flag2 = true;}
				else					timer2_waittime = current_time + sec;
				
				//draw_int(timer2_waittime, 90, 40, "w", ERR);
			}
			if(ovf_flag2 && (current_time <= timer2_waittime)) ovf_flag2 = false;
			if((current_time >= timer2_waittime) && (!ovf_flag2))
			{
				timer2_waittime = 0;
				return 0;
			}
			else return 1;
		}
		break;
		case TIMER_3:	//main loop com delays
		if(reset)
		{
			timer3_waittime = 0;
			ovf_flag3=false;
		}
		else {
			if(!timer3_waittime && sec)				//0 -> Timer l�uft nicht
			{
				uint16_t diff_time = UINT16_MAX - current_time;
				
				if (sec > diff_time) 	{timer3_waittime = sec - diff_time; ovf_flag3 = true;}
				else					timer3_waittime = current_time + sec;
				
				//draw_int(timer3_waittime, 90, 40, "w", ERR);
			}
			if(ovf_flag3 && (current_time <= timer3_waittime)) ovf_flag3 = false;
			if((current_time >= timer3_waittime) && (!ovf_flag3))
			{
				timer3_waittime = 0;
				return 0;
			}
			else return 1;
		}
		break;
		case TIMER_4:	//between measurements (filling)/ in s
		if(reset)
		{
			timer4_waittime = 0;
			ovf_flag4=false;
		}
		else {
			if(!timer4_waittime && sec)// timer l�uft nicht -> setzen
			{
				uint16_t diff_time = UINT16_MAX - current_time;
				
				if (sec > diff_time) 	{timer4_waittime = sec - diff_time; ovf_flag4 = true;}
				else					timer4_waittime = current_time + sec;
				
				start_time = current_time;
				//draw_int(timer4_waittime, 90, 40, "w", ERR);
				//draw_int(start_time, 90, 60, "s", ERR);
			}
			if(ovf_flag4 && (current_time <= timer4_waittime)) ovf_flag4 = false;
			if((current_time >= timer4_waittime) && (!ovf_flag4))
			{
				timer4_waittime = 0;
				return 0;					//timer abgelaufen
			}
			else {	//vergangene secs
				if ((timer4_waittime >= start_time) || ovf_flag4)		{return (current_time - start_time)+1;}
				else													{return ((UINT16_MAX - start_time) + current_time)+1;}
			}
		}
		break;
		case TIMER_5:	//xbee active time/ in s
		if(reset)
		{
			timer5_waittime = 0;
			ovf_flag5=false;
		}
		else {
			if(!timer5_waittime && sec)				//0 -> Timer l�uft nicht
			{
				uint16_t diff_time = UINT16_MAX - current_time;
				
				if (sec > diff_time) 	{timer5_waittime = sec - diff_time; ovf_flag5 = true;}
				else					timer5_waittime = current_time + sec;
				
				//draw_int(timer5_waittime, 90, 40, "w", ERR);
			}
			if(ovf_flag5 && (current_time <= timer5_waittime)) ovf_flag5 = false;
			if((current_time >= timer5_waittime) && (!ovf_flag5))
			{
				timer5_waittime = 0;
				return 0;
			}
			else return 1;
		}
		break;
		case TIMER_6:	//difference between two valid pressed keys / in ms
		if(reset)
		{
			timer8_2_stop();
			timer6_waittime = 0;
		}
		else {
			if(!timer6_waittime && sec)				// Init. timer
			{
				timer6_waittime = sec;				//setzen
				timer8_2_start();					//starten
			}
			
			if(t8_2_overflow >= ((timer6_waittime)/OVERFLOW_IN_MS_8_BIT))
			{
				timer8_2_stop();
				timer6_waittime = 0;
				return 0;
			}
			else if(t8_2_overflow) return t8_2_overflow; else return 1;
			//else return 1;
		}
		break;
		case TIMER_7:	//measurement/ sec wert in ms!
		if(reset)
		{
			timer8_0_stop();
			timer0_waittime = 0;
		}
		else {
			if( sec)				// Init. timer
			{
				timer8_0_start();					//starten
			}
			if(t8_0_overflow)
			{
				return t8_0_overflow;
			}
			else
			{
				return 1;
			}
			
		}
		break;

		default:
		break;
	}
	return 0;
}

//dialog timer is the slow transmission timer! used only on shut down (slow transmit doesn't matter there)
void timed_dialog(char *title, char *text, uint8_t timeout, unsigned int ForeColor, unsigned int BackColor)
{
	uint8_t x,y,i,len,x0;
	LCD_Cls(BackColor);
	
	char temp[2];
	temp[1] = '\0';
	switch (Orientation)
	{
		case Landscape:
		LCD_Box(X_LD_10,Y_LD_25,X_LD_160,Y_LD_120,ForeColor);
		LCD_Print(title,LD_MID_LANDSCAPE-((strlen(title)*FONT2_W)/2),Y_LD_1,2,1,1,ForeColor,BackColor);
		x0 = X_LD_13;
		break;
		case Landscape180:
		LCD_Box(X_LD_17,Y_LD_25,X_LD_166,Y_LD_120,ForeColor);
		LCD_Print(title,LD_MID_LANDSCAPE-((strlen(title)*FONT2_W)/2),Y_LD_1,2,1,1,ForeColor,BackColor);
		x0 = X_LD_20;
		break;
		default:
		LCD_Box(X_LD_10,Y_LD_25,X_LD_120,Y_LD_165,ForeColor);
		LCD_Print(title,LD_MID_LANDSCAPE-((strlen(title)*FONT2_W)/2),Y_LD_1,2,1,1,ForeColor,BackColor);
		x0 = X_LD_13;
		break;
	}
	
	//for Landscape only
	x = x0;
	y = Y_LD_28 ;//Y_TD_28;
	len = strlen(text);
	for(i=0;i<len;i++)
	{
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
	
	ready_for_new_key();
	
	while(1)
	{	//check for pressed Key or wait timeout
		
		set_timeout(timeout, TIMER_3, USE_TIMER);
		while(!(keyhit_cont() == 0));                  // wait until button is released
		while(set_timeout(0, TIMER_3, USE_TIMER))
		{
			//delay_ms(200);
			if(keyhit_cont() > 0)  {set_timeout(0, TIMER_1, RESET_TIMER);break;}
		}
		return;
	}
}


