// Adwandler.c - Copyright 2016, HZB, ILL/SANE & ISIS
#define RELEASE_ADWAND 1.00

// HISTORY -------------------------------------------------------------
// 1.00 - First release on March 2016

#include <avr/io.h>
#include <math.h>
#include <util/delay.h>

#include "display.h" // Added by JG for debugging
#include "display_utilities.h"
#include "timer_utilities.h"

#include "main.h"

#include "adwandler.h"

// Initialize analog to digital converter on selected channel
void adc_init(uint8_t channel)
{
	uint16_t result;

	// ADCSRA means ADC Control and Status Register A
	// Set ADEN to true enables the ADC
	// Set the division factor to 128 between the XTAL frequency and the input clock to the ADC
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

	ADMUX = channel;					// Select channel
	ADMUX |= (0<<REFS1) | (1<<REFS0); 	// Intern reference on AVCC with external capacitor at AREF pin
	
	// First conversion performs initialization of ADC
	ADCSRA |= (1<<ADSC);
	
	while (ADCSRA & (1<<ADSC));

	result = ADCW;
}

///rounds double number
double round_double(double number, int digits)
{	int temp;

	int mul = pow(10,digits);
	number*=mul;
	if(number>0)
	number+=0.5;
	else
	number-=0.5;
	//cut off
	temp=number;
	number=temp;
	
	number/=mul;
	return number;
}

double map_to_current(uint16_t adcVal)
{
	return ((adcVal)/((1023.0*10.0)/(100.0*11.0)));
}

double map_to_volt(uint16_t adcVal)
{
	return ((adcVal)/((1023.0*10.0)/(35.0*11.0)));
}

double map_to_default(uint16_t adcVal)
{
	return ((adcVal)/((1023.0)/1.0));
}

double map_to_test(uint16_t adcVal)
{
	return ((adcVal)/((1023.0)/50.0));
}

double map_to_batt(uint16_t adcVal)
{
	return ((adcVal)/((1023.0*10.0)/(15.0*11.0)));
}

int16_t map_to_pres(uint16_t adcVal, double zero, double span)
{
	return (((adcVal)/((1023.0)/10.0))*span) - zero;
}


double calc_he_level(double res_x, double res_min, double res_max)
{
	// The measured resistance is quite high, probe is probably not connected
	if((fabs(res_x) > 5000)||(fabs(res_x) < 50))
	{
		return errCode_TooHighRes;	// Return an error code
	}
	
	// The measured resistance is lower than the minimum value, the dewar is full
	if(res_x < res_min)
	{
		return 100;			// Return 100 %
	}
	
	// The measured resistance is higher than the maximum value, the dewar is empty	
	if(res_x > res_max)
	{
		return 0;			// Return 0 %
	}
	
	return ((res_max - res_x)/(res_max - res_min))*100;
}

///reads the given channel and returns converted ADc-value (average of "avr" measurements)
uint16_t readChannel(uint8_t channel, uint8_t avg)
{
	uint16_t result = 0;
	
	ADMUX = channel;						// Kanal waehlen
	ADMUX |= (0<<REFS1) | (1<<REFS0); 	// interne Referenz auf AVcc
	
	//Messung - Mittelwert aus "avg" aufeinanderfolgenden Wandlungen
	for( uint8_t i=0; i<avg; i++ )
	{
		ADCSRA |= (1<<ADSC);            	// eine Wandlung "single conversion"
		while ( ADCSRA & (1<<ADSC) );		// auf Abschluss der Konvertierung warten
		
		result += ADCW;		    			// Wandlungsergebnisse aufaddieren
	}

	return result/avg;
}

uint16_t readChannel_calib(uint8_t channel, uint8_t nb_readings, double adczero)
{
	uint16_t result = 0;
	
	ADMUX = channel;						// Select channel
	ADMUX |= (0<<REFS1) | (1<<REFS0); 		// Internal reference on AVcc
	
	// Measurement - mean value of "nb_readings" consecutive actions
	for(uint8_t i=0; i<nb_readings; i++)
	{
		ADCSRA |= (1<<ADSC);            	// single conversion
		while ( ADCSRA & (1<<ADSC) );		// wait for the conversion is complete
		
		result += ADCW;		    			// add results conversion
	}
	
	if(((result/nb_readings) + adczero) < 0)
	{
		return 0;
	}
	
	return (((double)result/nb_readings) + adczero);
}


uint8_t get_batt_level(double batt_min, double batt_max, double r_zero)
{
	double batt_v = map_to_batt(readChannel_calib(BATTERY, ADC_LOOPS, r_zero));
	
	if(batt_v < batt_min) 
		return 0;
	else
		return ((batt_v-batt_min)*100.0)/(batt_max-batt_min);
}

// Measure liquid He level
double get_he_level(double res_min, double res_max, double r_span, double r_zero, uint8_t count, double quench_time, double quench_current, double wait_time, double meas_current, uint8_t show_progress)
{
	uint16_t u_val = 0, i_val = 0;				// Initialize voltage and current variables
	uint16_t u_val_temp = 0, i_val_temp = 0;	// Initialize voltage and current buffer variables
	
	uint16_t quench_time_ms = quench_time*1000;				// Converts seconds to ms
	uint16_t wait_time_ms	= wait_time*1000;				// Converts seconds to ms
//	uint16_t heat_time_ms 	= 10000;				// Converts seconds to ms
	uint8_t steps_sum 		= floor(((double) (quench_time_ms/OVERFLOW_IN_MS_8_BIT)));	//118.x floor: 118
//	uint8_t	step 			= floor(steps_sum/9.0);			//13
//	uint16_t current_count = 0;
	
	//=========================================================================
	//PWM-Configuration to control measuring current
	//=========================================================================
		
	// Start PWM
	TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (0 << WGM22) | (1 << CS22) | (0 << CS21) | (1 << CS20);
	DDRD |= (1 << PD7);			// Set PORTD.7 as output

	// Switch on current supply to 150 mA to quench the He probe (153.67 mA measured)
	// OCR2A value should be in 0 to 255 range
	// 0 gives 0 mA / 255 gives 238 mA
//	OCR2A = 160;
	OCR2A = (uint8_t)round(quench_current*(double)(255/238));
	//init_0_timer8();	// Enable Timer interrupt
	set_timeout(quench_time_ms, TIMER_0, USE_TIMER);
	while(set_timeout(0, TIMER_0, USE_TIMER))
	{
	}
	
	// Change current output to 80 mA (81.36 mA measured)
//	OCR2A = 94; 
	OCR2A = (uint8_t)round(meas_current*(double)(255/238));
	set_timeout(wait_time_ms, TIMER_0, USE_TIMER);
	while(set_timeout(0, TIMER_0, USE_TIMER))
	{
		//if(show_progress)
		//{
		//paint_progress_bar(5, 105, current_count/step);
		//}
	}

	for(uint8_t i=0;i < count; ++i)
	{
		for(uint8_t j=0;j<ADC_LOOPS;j++)
		{
			u_val_temp  += readChannel_calib(VOLT_PROBE_MEAS, ADC_LOOPS, r_zero); // VOLT_PROBE_MEAS =2 / ADC_LOOPS=10 / 
			i_val_temp  += readChannel_calib(CURRENT_PROBE_MEAS, ADC_LOOPS, r_zero); // CURRENT_PROBE_MEAS = 1
		}
		
		u_val+=(u_val_temp/ADC_LOOPS);
		i_val+=(i_val_temp/ADC_LOOPS);
	}
	// Stop PWM
	DDRD &= (0 << PD7);			// Set PORTD.7 as input

	if(show_progress)	paint_progress_bar(5, 105, 10);

	u_val = u_val/count;		// Calculate the averaged voltage
	i_val = i_val/count;		// Calculate the averaged current
	
	if(i_val <= 0) i_val=1;
	
	//double r_val = (double)(( u_val/i_val)*r_span-10);
	
	
	// Voltage reference 3.34 V measured on the PCB
	// On PA1, R18 & R19 divide the voltage probe (82500+3740)/3740=23.06
	// On PA2, we measured 952 mV for 81,36 mA => 81.36/952 = 0.08546 mA/mV
	double r_val = ((u_val*3.34/1024)*23.06)/((i_val*3.34/1024)*0.08546);
	
	// Added by JG for debugging
/*
	char str_temp[20];
	sprintf(str_temp,"i: %d /u: %d /res: %d\n", (int)i_val, (int)u_val, (int)r_val);
	LCD_Print(str_temp, 5, 60, 1, 1, 1, yellow, black);
	delay_ms(6000);
*/
	
	if(i_val < 50) r_val=9999;	// Any cable connected
	
	return calc_he_level(r_val, res_min, res_max);
}


