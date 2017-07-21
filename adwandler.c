
#include "adwandler.h"


void adc_init(uint8_t channel)
{
	uint16_t result;

	//Frequenzvorteiler setzen auf 128 und ADC aktivieren
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); 

	ADMUX = channel;					// Kanal waehlen
	ADMUX |= (0<<REFS1) | (1<<REFS0); 	// interne Referenz auf AVcc
 
	//Readout um den ADC warmlaufen zu lassen (Init) 
	ADCSRA |= (1<<ADSC);	// 1 Wandlung
  
	while ( ADCSRA & (1<<ADSC) );

	//Strom sparen?
	//ADCSRA &= ~(1<<ADEN); // ADC deaktivieren
	
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
	if(abs(res_x) > 5000)	return 101;
	if(res_x < res_min) 	return 100;	//return -2.0;			//übervoll
	if(res_x > res_max) 	return 0;	//return -3.0; 			//überleer
	
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

uint16_t readChannel_calib(uint8_t channel, uint8_t avg, double adczero)
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
  
  if(((result/avg) + adczero) < 0)
  {
	return 0;
  }
  
  return (((double)result/avg) + adczero);
}


uint8_t get_batt_level(double batt_min, double batt_max, double r_zero)
{	
	double batt_v = map_to_batt(readChannel_calib(BATTERY, ADC_LOOPS, r_zero));
	
	if(batt_v < batt_min)  return 0;
	else	return ((batt_v-batt_min)*100.0)/(batt_max-batt_min);
}

///measure he-level, show progress bar if needed
double get_he_level(double res_min, double res_max, double r_span, double r_zero, uint8_t count, double heat_time, uint8_t show_progress)
{
	uint16_t u_val = 0;
	uint16_t i_val = 0;
	uint16_t u_val_temp = 0;
	uint16_t i_val_temp = 0;
	
	//4000 = 225 / 9 = 25
	uint16_t heat_time_ms 	= heat_time*1000;				//2100
	uint8_t steps_sum 		= floor(((double) (heat_time_ms/OVERFLOW_IN_MS_8_BIT)));	//118.x floor: 118
	uint8_t	step 			= floor(steps_sum/9.0);			//13
	uint16_t current_count = 0;
	
	set_timeout(heat_time_ms, TIMER_0, USE_TIMER);
	while( current_count = set_timeout(0, TIMER_0, USE_TIMER) )
	{
		if(show_progress)
		{
			paint_progress_bar(5, 105, current_count/step);
		}
	}
	
	for(uint8_t i=0;i < count; ++i)
	{		
		for(uint8_t j=0;j < ADC_LOOPS; ++j)
		{
			u_val_temp  += readChannel_calib(VOLTAGE, ADC_LOOPS, r_zero);
			i_val_temp  += readChannel_calib(CURRENT, ADC_LOOPS, r_zero);		
		}
		
		u_val+=(u_val_temp/ADC_LOOPS);
		i_val+=(i_val_temp/ADC_LOOPS);
		
		u_val_temp = 0;
		i_val_temp = 0;
	}
	if(show_progress)	paint_progress_bar(5, 105, 10);

	u_val = u_val/count;
	i_val = i_val/count;
	
	if(i_val <= 0) i_val=1;
	double r_val	= ((double) u_val/i_val)*r_span-10;
	
	if(i_val < 50) r_val=9999;	//damit cable? anzeige herauskommt
	
	return calc_he_level(r_val, res_min, res_max);
}


