
#include "kanne.h"


/// USART recieve lock
volatile uint8_t cmd_line=0;
/// pointer to the next byte to send via USART		
volatile uint8_t *send_str_reader;
/// number of bytes to send	via USART
volatile uint8_t sending_cmd=0;
///status byte	
volatile uint8_t status_byte;
///pressed key, checked in PCI_int
volatile uint8_t pressed_key = 0;			//key
volatile uint8_t execute_pressed_key = 0;	//remember to execute

//EEPROM vars
#ifdef ALLOW_EEPROM_SAVING
	//control
	//calibration
	uint16_t 	ee_r_span EEMEM = R_SPAN_DEF*10;
	uint16_t 	ee_r_zero EEMEM = R_ZERO_DEF;

	//options
	uint16_t 	ee_transmit_slow EEMEM = 0;
	uint8_t 	ee_transmit_slow_min EEMEM = 0;
	uint16_t 	ee_transmit_fast EEMEM = 0;
	uint8_t 	ee_transmit_fast_sec EEMEM = 0;
	uint16_t 	ee_heat_time EEMEM = 0;
	uint8_t 	ee_meas_cycles EEMEM = 0;
	uint8_t 	ee_fill_timeout EEMEM = 0;
	uint8_t 	ee_he_min EEMEM = 10;
	uint16_t 	ee_res_min EEMEM = 0;
	uint16_t 	ee_res_max EEMEM = 0;
	uint16_t 	ee_span EEMEM = 0;
	uint16_t 	ee_zero EEMEM = 0;
	uint8_t		ee_enable_pressure = 1;
	uint16_t 	ee_batt_min EEMEM = 0;
	uint16_t 	ee_batt_max EEMEM = 0;
	uint8_t 	ee_critical_batt EEMEM = 0;
#endif


inline uint8_t pos_to_binary(char *pos, uint8_t offset, uint8_t *buffer)
{
	uint8_t index = 0;
	
	while (pos[index]!='\0')
	{
		buffer[index+offset] = pos[index];
		index++;
	}
	
	//fill spaces if pos is less then 3 letters
	for(uint8_t i=0; i<(3 - index); i++) buffer[i+index+offset] = 0x20;
	
	//return index, add spaces
	return (index + offset) + (3 - index);
}
//=========================================================================
///display backlight toggle
//=========================================================================
inline bool display_on(void)
{
	if (DISPLAY_ON) 
	{
		//display is on, extend display_on time
		set_timeout(0, TIMER_2, RESET_TIMER);
		set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
		return true;
	}
	else {
		//turn display backlight on
		DISPLAY_TURN_ON
		set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
		#ifdef ALLOW_DEBUG
			LCD_Print("on ", 5, 20, 2, 1, 1, FGC, BGC); 
		#endif
		return false;
	}
}
//=========================================================================
///check auto fill pin
//=========================================================================
inline bool auto_fill_pin_on(void)
{
	if(AUTO_FILL_TURNED_ON)
	{
		/*delay_ms(5);
		if(AUTO_FILL_TURNED_ON)*/ return 1;
		//else return 0;
	}
	else {
		/*delay_ms(5);
		if(!AUTO_FILL_TURNED_ON)*/ return 0;
		//else return 1;
	}
	
	return 0;
}

inline uint8_t get_at_frame_type(char *id, uint8_t len)
{
	//if(len < 2) return UNDEFINED;
	if((id[0] == 'D') && (id[1] == 'A')) return DA_MSG_TYPE;
	if((id[0] == 'D') && (id[1] == 'L')) return DL_MSG_TYPE;
	if((id[0] == 'D') && (id[1] == 'H')) return DH_MSG_TYPE;
	
	return 0;
}
//=========================================================================
///build a frame
//=========================================================================
inline void create_Frame(uint8_t *buffer, uint8_t length)
{
	if(length < 5) return;
	if(!chkChecksum(buffer)) return;
	
	BuffType newFrame;
	uint8_t data_counter = 0;
	
	newFrame.length = (buffer[1]<<8) + buffer[2];
	newFrame.api_id = buffer[3];
	newFrame.data_len = 0;	//init
	newFrame.type = 0;		//init
	
	switch(newFrame.api_id)
	{
		case AT_ID:
					newFrame.status = buffer[7];
					newFrame.type = get_at_frame_type((char*) &buffer[5], 2);
					
					//write data
					if (length < (8+1)) return;
					for (uint8_t i = 8; i < length; ++i)
						newFrame.data[data_counter++] = buffer[i];
					newFrame.data_len = --data_counter;
					break;
		case RX_ID:
					newFrame.type = buffer[14];
					newFrame.status = buffer[15];
					
					if (length < (16+1)) return;
					for (uint8_t i = 16; i < length; ++i)
						newFrame.data[data_counter++] = buffer[i];
					newFrame.data_len = --data_counter;
					break;
		case STA_ID:
					newFrame.status = (buffer[4] == 2)? 0 : 0xFF;
					newFrame.type = STATUS_MSG_TYPE;
					break;
		default:	
					newFrame.status = 0xFF;
	}
	BUFF_storeData(newFrame);
}
//=========================================================================
///USART data recieved interrupt routine 
//=========================================================================
ISR(USART0_RX_vect) 
{	
	static uint16_t length_counter = 0;
	static uint16_t summe = 0;
	static uint16_t frame_length = 0;
	static uint8_t buffer[SINGLE_FRAME_LENGTH];
	static uint8_t buffer_size = 0;
	
	uint8_t data = COM;

	if(buffer_size >= SINGLE_FRAME_LENGTH-1)	{create_Frame(buffer, buffer_size); buffer_size=0; frame_length=0;}
	if(cmd_line) return;
	
	if (length_counter > 1)
	{
		length_counter--;
		summe=+data;
		buffer[buffer_size++] = data;
	}
			
	if (frame_length > 1)
	{
		frame_length--;
		buffer[buffer_size++] = data;
	}
		
	if (frame_length == 1) 
	{
		frame_length--;
		create_Frame(buffer, buffer_size);
		buffer_size = 0;
		return;
	}
	
	if (length_counter == 1) 
	{
		frame_length = summe+2;
		length_counter--;
	}
	
	if (data == 0x7E)
	{						 							
		if(!frame_length)
		{
			length_counter = 3;
			buffer[buffer_size++] = data;
		}
	}
}

//=========================================================================
///USART data transmitted interrupt routine 
//=========================================================================
ISR(USART0_TX_vect) 
{
	if(sending_cmd) 
	{	
		COM=*send_str_reader++;
		sending_cmd--;
	}
}


inline void pin_change_init(void)
{
	PCICR |= (1<<PCIE3);		//enable int 24-31 (D-PORT)
	PCMSK3 |= (1<<PCINT31) | (1<<PCINT30) | (1<<PCINT29) | (1<<PCINT28) | (1<<PCINT27) | (1<<PCINT26); //enable only button pins
}



ISR(PCINT3_vect)
{
	//wakes cpu up + saves button
	if(execute_pressed_key || pressed_key) return;
	switch(keyhit())
	{		
		case KEY_LEFT_1:	pressed_key = KEY_LEFT_1;execute_pressed_key=1;
							break;
		case KEY_LEFT_2:	pressed_key = KEY_LEFT_2;execute_pressed_key=1;
							break;
		case KEY_DOWN:		pressed_key = KEY_DOWN;execute_pressed_key=1;
							break;
		case KEY_UP:		pressed_key = KEY_UP;execute_pressed_key=1;
							break;
		case KEY_RIGHT_1:	pressed_key = KEY_RIGHT_1;execute_pressed_key=1;
							break;
		case KEY_RIGHT_2:	pressed_key = KEY_RIGHT_2;execute_pressed_key=1;
							break;
		case KEY_FILL:		pressed_key = KEY_FILL;execute_pressed_key=1;
							break;
		default:			pressed_key = 0;execute_pressed_key=0;
							break;
	}
}

inline void xbee_sleep(void)
{
	#ifdef ALLOW_XBEE_SLEEP
		PORTC|=(1<<PC4);
		delay_ms(20);
	#endif
}
inline void xbee_wake_up(void)
{
	#ifdef ALLOW_XBEE_SLEEP
		PORTC&=~(1<<PC4);
		delay_ms(30);
	#endif
}


inline void diag_pulse(double *heat_time, double r_span, double r_zero)
{
	paint_diag(3);
	
	uint16_t diag_heat_timel_line = 0;
	uint8_t u_points[140];
	uint8_t i_points[140];
	uint16_t delay_between_points = ceil((*heat_time*1000)/50);
	
	uint16_t temp_ipoint = readChannel_calib(CURRENT, ADC_LOOPS, r_zero);
	u_points[0] = toY(map_to_volt( readChannel_calib(VOLTAGE, ADC_LOOPS, r_zero) ), 35, 20);
	i_points[0] = toY(map_to_current(temp_ipoint), 100, 75);
	
	for(uint8_t i = 1; i<10;i++)
	{
		temp_ipoint = readChannel_calib(CURRENT, ADC_LOOPS, r_zero);
		u_points[i] = toY(map_to_volt( readChannel_calib(VOLTAGE, ADC_LOOPS, r_zero) ), 35, 20);
		i_points[i] = toY(map_to_current(temp_ipoint), 100, 75);
		
		LCD_Draw(i+1, u_points[i-1],i+2, u_points[i],0,FGC);
		LCD_Draw(i+1, i_points[i-1],i+2, i_points[i],0,FGC);
		delay_ms(delay_between_points);
	}
	MEASURE_PIN_ON
	for(uint8_t i = 0; i<130;i++)
	{
		temp_ipoint = readChannel_calib(CURRENT, ADC_LOOPS, r_zero);
		u_points[i+10] = toY(map_to_volt( readChannel_calib(VOLTAGE, ADC_LOOPS, r_zero) ), 35, 20);
		i_points[i+10] = toY(map_to_current(temp_ipoint), 100, 75);
		
		LCD_Draw(i+10,u_points[i+9],i+11,u_points[i+10],0,FGC);
		LCD_Draw(i+10,i_points[i+9],i+11,i_points[i+10],0,FGC);
		delay_ms(delay_between_points);
	}
	MEASURE_PIN_OFF
	diag_heat_timel_line = 2+10+40;
	LCD_Draw(diag_heat_timel_line,20,diag_heat_timel_line,/*71*/126,0,ERR); // |
	draw_int(delay_between_points*(diag_heat_timel_line-2), 90, 2, "ms", ERR);
	
	bool back = true;
	while(back)
	{
		switch(keyhit())
		{
			case KEY_UP:
						if(--diag_heat_timel_line < 25) 
						{
							diag_heat_timel_line++;
							break;
						}
						LCD_Draw(diag_heat_timel_line+1,20,diag_heat_timel_line+1,126,0,BGC);
						
						//draw old u point
						LCD_Plot(diag_heat_timel_line+1,u_points[diag_heat_timel_line-1], 0, FGC);
						//draw old i point
						LCD_Plot(diag_heat_timel_line+1,i_points[diag_heat_timel_line-1], 0, FGC);
						
						LCD_Plot(diag_heat_timel_line+1,71, 1, FGC);
						LCD_Plot(diag_heat_timel_line+1,126, 1, FGC);
						
						LCD_Draw(diag_heat_timel_line,20,diag_heat_timel_line,126,0,ERR);
						
						if(1000 > (delay_between_points*(diag_heat_timel_line-3)))
							LCD_Print("      ", 90, 2, 2, 1, 1, FGC, BGC);
						else
							LCD_Print("     ", 90, 2, 2, 1, 1, FGC, BGC);
						draw_int(delay_between_points*(diag_heat_timel_line-3), 90, 2, "ms", ERR);
						delay_ms(100);
						break;
			case KEY_DOWN:
						if(++diag_heat_timel_line > 135) 
						{
							diag_heat_timel_line--;
							break;
						}
						LCD_Draw(diag_heat_timel_line-1,20,diag_heat_timel_line-1,126,0,BGC);
						
						//draw old point
						LCD_Plot(diag_heat_timel_line-1, u_points[diag_heat_timel_line-3], 0, FGC);
						//draw old i point
						LCD_Plot(diag_heat_timel_line-1, i_points[diag_heat_timel_line-3], 0, FGC);
						
						
						LCD_Plot(diag_heat_timel_line-1,71, 1, FGC);
						LCD_Plot(diag_heat_timel_line-1,126, 1, FGC);
						
						LCD_Draw(diag_heat_timel_line,20,diag_heat_timel_line,126,0,ERR);
						
						//nur zu anzeige, je nach stellenanzahl, korrektur nur bei 3 stellen
						if(1000 > (delay_between_points*(diag_heat_timel_line-3)))
							LCD_Print("      ", 90, 2, 2, 1, 1, FGC, BGC);
						else
							LCD_Print("     ", 90, 2, 2, 1, 1, FGC, BGC);
						draw_int(delay_between_points*(diag_heat_timel_line-3), 90, 2, "ms", ERR);
						delay_ms(100);
						break;
			case KEY_RIGHT_1:
						*heat_time = (delay_between_points*(diag_heat_timel_line-3))/1000.0;
						#ifdef ALLOW_EEPROM_SAVING
							eeprom_write_word(&ee_heat_time, (uint16_t) ((*heat_time)*1000));
						#endif
						back = false;
						delay_ms(100);
						break;
			case KEY_LEFT_1:
						back = false;
						delay_ms(100);
						break;
			default:
						break;
		}
	}
}

void diag_page1(double r_zero, double r_span, double batt_min, double batt_max, double res_min, double res_max, double zero, double span)
{
	char temp[10];
	uint16_t diag_current_adc  	= readChannel_calib(CURRENT, ADC_LOOPS, r_zero);
	uint16_t diag_voltage_adc 	= readChannel_calib(VOLTAGE, ADC_LOOPS, r_zero);
	uint16_t diag_pressure_adc 	= readChannel_calib(PRESSURE, ADC_LOOPS, r_zero);
	uint16_t diag_battery_adc 	= readChannel_calib(BATTERY, ADC_LOOPS, r_zero);
	
	if(diag_current_adc <= 0)	diag_current_adc = 1;
	
	double diag_res = 0;
	if( (diag_voltage_adc > 10) && (diag_current_adc > 10) ) 
	{
		diag_res = ((double) diag_voltage_adc/diag_current_adc)*r_span-10;
	}
	double diag_battery_map	= get_batt_level(batt_min, batt_max, r_zero);
	double diag_battery_volt 	= map_to_batt(diag_battery_adc);
	double diag_pressure_map	= map_to_pres(diag_pressure_adc, zero, span);
	
	double he_level = calc_he_level(diag_res, res_min, res_max);
	
	//Rx
	LCD_Print("         ", 40, 20, 2, 1, 1, FGC, BGC);
	draw_double(diag_res, 40, 20, 1,"ohm", FGC);
	
	//HeL
	LCD_Print("         ", 40, 40, 2, 1, 1, FGC, BGC);
	draw_double(he_level, 40, 40, 1,"%", FGC);
	
	//Bat
	LCD_Print("     ", 40, 60, 2, 1, 1, FGC, BGC);
	draw_int(diag_battery_map, 50, 60,"%", FGC);
	LCD_Print("     ", 90, 60, 2, 1, 1, FGC, BGC);
	//draw_double(diag_battery_volt, 90, 60, 1,"V", FGC);
	dtostrf(diag_battery_volt,4,1,temp);
	strcat(temp, "V");
	LCD_Print("      ", 90, 60, 2, 1,1, FGC, BGC);
	LCD_Print(temp, 90, 60, 2, 1,1, FGC, BGC);
	
	//Prs
	LCD_Print("         ", 40, 80, 2, 1, 1, FGC, BGC);
	draw_int(diag_pressure_map, 50, 80,"mbar", FGC);
	
	//Curr
	(CHECK_MEASURE_PIN)?
		LCD_Print("ON ", 40, 100, 2, 1,1, ERR, BGC)
	:	LCD_Print("OFF", 40, 100, 2, 1,1, ERR, BGC);
}

inline void diag_page2(double r_zero)
{
	uint16_t diag_current_adc  = readChannel(CURRENT, ADC_LOOPS);
	uint16_t diag_voltage_adc = readChannel(VOLTAGE, ADC_LOOPS);
	uint16_t diag_main_voltage_adc = readChannel(MAINVOLTAGE, ADC_LOOPS);
	
	double diag_voltage_map = map_to_volt(diag_voltage_adc);
	double diag_current_map = map_to_current(diag_current_adc);
	double diag_main_voltage_map = map_to_volt(diag_main_voltage_adc);
	
	//ADCV
	LCD_Print("     ", 50, 20, 2, 1, 1, FGC, BGC);
	draw_int(diag_voltage_adc, 50, 20,"", FGC);
	LCD_Print("     ", 90, 20, 2, 1, 1, FGC, BGC);
	draw_int(diag_voltage_adc+r_zero, 90, 20,"", FGC);
	
	//ADCI
	LCD_Print("     ", 50, 40, 2, 1, 1, FGC, BGC);
	draw_int(diag_current_adc, 50, 40, "", FGC);
	LCD_Print("     ", 90, 40, 2, 1, 1, FGC, BGC);
	draw_int(diag_current_adc+r_zero, 90, 40,"", FGC);
	
	//I
	LCD_Print("       ", 50, 60, 2, 1, 1, FGC, BGC);
	draw_double(diag_current_map, 50, 60, 1,"mA", FGC);
	
	//U
	LCD_Print("       ", 50, 80, 2, 1, 1, FGC, BGC);
	draw_double(diag_voltage_map, 50, 80, 1,"V", FGC);
	
	//Us
	LCD_Print("       ", 50, 100, 2, 1, 1, FGC, BGC);
	draw_double(diag_main_voltage_map, 50, 100, 1,"V", FGC);
}

/*void timer_test(uint8_t timer, uint16_t sec)
{
	LCD_Cls(BGC);
	switch(timer)
	{
		case TIMER_0:	
						LCD_Print("TIMER_0", 5, 5, 2, 1, 1, FGC, BGC);
						break;
		case TIMER_1:	
						LCD_Print("TIMER_1", 5, 5, 2, 1, 1, FGC, BGC);
						break;
		case TIMER_2:	
						LCD_Print("TIMER_2", 5, 5, 2, 1, 1, FGC, BGC);
						break;
		case TIMER_3:	
						LCD_Print("TIMER_3", 5, 5, 2, 1, 1, FGC, BGC);
						break;
		case TIMER_4:	
						LCD_Print("TIMER_4", 5, 5, 2, 1, 1, FGC, BGC);
						break;
		case TIMER_5:	
						LCD_Print("TIMER_5", 5, 5, 2, 1, 1, FGC, BGC);
						break;
	}
	//setzten
	set_timeout(sec, timer, USE_TIMER);
	//läuft
	delay_ms(20);
	//checken
	while(set_timeout(0, timer, USE_TIMER))
	{
		LCD_Print("run", 50, 50, 2, 1, 1, FGC, BGC);
		if(keyhit() > 0)  break;
	}
	
	LCD_Print("ende", 50, 100, 2, 1, 1, FGC, BGC);
	getkey();
}*/

//=========================================================================
///MAIN PROGRAM
//=========================================================================
int main(void)  
{
	//=========================================================================
	//PORTS
	//=========================================================================
	//Measurement PIN
	DDRC |= (1<<DDC0);			//Pin c0 as output
	MEASURE_PIN_OFF				
	
	//start_auto_fill_pin
	DDRC |= (1<<DDC2);			//Pin c2 as output
	
	//shutdown pin
	DDRC |= (1<<DDC5);			//Pin c5 as output
	PORTC |= (1<<PC5);
	
	//Display light control PIN
	DDRB |= (1<<DDB3);			//Pin B3 as output
	DISPLAY_TURN_ON
	//DISPLAY_TURN_OFF
	
	//auto fill pin
	DDRC &= ~(1<<DDC1);		//Pin c1 as input
	PORTC |= (1<<PC1);			//pull-up activated
	
	//measure button
	DDRC &= ~(1<<DDC3);		//Pin c3 as input
	PORTC |= (1<<PC3);			//pull-up activated
	
	
	//xbee
	DDRC |= (1<<DDC4);			//Pin c4 as output
	xbee_wake_up();
	
	//=========================================================================
	//PWM-Configuration to control measuring current
	//=========================================================================
	
	//DDRD |= (1 << PD7);
 //	//  DDRD |= 0b10000000;
	//  TCCR2A = (1 << COM2A1) | (1 << COM2A0) | (0 << COM2B1) | (0 << COM2B0) | (1 << WGM21) | (1 << WGM20);
	//  TCCR2B = (0 << FOC2A) | (0 << FOC2B) | (0 << WGM22) | (1 << CS22) | (1 << CS21) | (1 << CS20);
	//  ASSR = (0 << EXCLK) | (0 << AS2);
	//  
	//  OCR2A = 0;
	//  TIMSK2 = (1 << OCIE2A);
	//  TIFR2 = (1 << OCF2A);
//
	
	//=========================================================================
	//Initialization
	//=========================================================================
	LCD_Init();					//init LCD
	keyboard_init();			//init keyboard
	init_usart(39);				//init RS232 Communication, PRESCALE-Konstante, s. CPU-Manual oder usart.h 
	init_0_timer8();
	init_1_timer16();			//init Timer1 (16bit)
	timer16_1_start();			//always run
	pin_change_init();
	adc_init(BATTERY);			//init ADC and make 1. convertion on BATT PIN
	adc_init(MAINVOLTAGE);		//init ADC 
	adc_init(CURRENT);			//init ADC 
	adc_init(PRESSURE);			//init ADC 
	adc_init(VOLTAGE);			//init ADC 
	
	//=========================================================================
	//Variables
	//=========================================================================
	uint16_t 	device_id = 0;
	enum 		modi mode = MODE_MAIN_WINDOW;
	enum 		error err_code = NO_ERROR;
	
	double 		he_level = 0;
	double		last_he_level = 200;
	bool		no_he_level = false;
	uint8_t 	batt_level = 0;
	uint16_t 	pressure_level = 0;
	uint8_t 	meas_progress = 0;
	
	uint8_t		temp_mode = 0;
	bool		force_measurement = false;
	bool 		did_measurement = false;
	
	//default positions, example
	char 		fill_pos_letters[FILL_RANGES_COUNT][FILL_LETTERS_LENGTH] = {"\r","AB","C", "DEF","G", "H", "\r"};
	uint8_t 	fill_pos_ranges[FILL_RANGES_COUNT][FILL_RANGES_LENGTH] = {{END},{END,1,2,3,4,5,7,19,END}, {END,2,3,4,END}, {END,7,8,12,END}, {END,2,7,4,11,15,17,END}, {END,1,2,END}, {END}};
	char 		device_pos[5] = "POS";
	
	//diag pages
	uint8_t 	diag_to_show = 1;
	
	double 		r_span=R_SPAN_DEF;
	double 		r_zero=R_ZERO_DEF;
	double 		temp_r_span=0;
	double 		temp_r_zero=0; 
	
	//Options vars
	uint8_t		xbee_sleep_period = 1;
	bool 		xbee_sleeping = false;
	bool		xbee_busy = false;
	uint16_t	options_pw = OPTIONS_PASSWORD;
	uint16_t	entered_options_pw = 0;
	
	uint16_t 	transmit_slow = TRANSMIT_SLOW_DEF;
	bool 		transmit_slow_min = false;
	bool 		transmit_slow_changed = false;
	bool 		auto_fill_enabled = false;
	bool 		auto_fill_started = false;
	uint16_t 	transmit_fast = TRANSMIT_FAST_DEF;
	bool 		transmit_fast_sec = false;
	double 		heat_time = HEAT_TIME_DEF;
	uint8_t 	meas_cycles = MEASUREMENT_CYCLES_DEF;
	uint8_t 	fill_meas_counter = 0;
	uint8_t 	fill_timeout = FILLING_TIMEOUT_DEF;
	int8_t 		he_min = AUTO_FILL_HE_DEF;
	double 		res_min = RES_MIN_DEF;
	double 		res_max = RES_MAX_DEF;
	double 		span = SPAN_DEF;
	double 		zero = ZERO_DEF;
	bool		enable_pressure = true;
	double 		batt_min = BATT_MIN_DEF;
	double 		batt_max = BATT_MAX_DEF;
	uint8_t 	critical_batt = CRITICAL_BATT_DEF;
	
	
	#ifdef ALLOW_EEPROM_SAVING
		r_span 				= (double) (eeprom_read_word(&ee_r_span)/10.0);			if(r_span < 0) 		r_span = 350.0;
		r_zero 				= (double) (eeprom_read_word(&ee_r_zero)/10.0);			if(r_zero < 0) 		r_zero = 0;
		temp_r_span 		= r_span;
		temp_r_zero 		= r_zero;
		transmit_slow 		= eeprom_read_word(&ee_transmit_slow); 						//if(transmit_slow < 0) 		transmit_slow = TRANSMIT_SLOW_DEF;
		transmit_slow_min 	= eeprom_read_byte(&ee_transmit_slow_min);					//if(transmit_slow_min < 0) 	transmit_slow_min = false;
		transmit_fast 		= eeprom_read_word(&ee_transmit_fast);						//if(transmit_fast < 0) 		transmit_fast = TRANSMIT_FAST_DEF;
		transmit_fast_sec	= eeprom_read_byte(&ee_transmit_fast_sec);	
		heat_time 			= (double) (eeprom_read_word(&ee_heat_time)/1000.0);		if(heat_time < 0) 			heat_time = HEAT_TIME_DEF;
		meas_cycles 		= eeprom_read_byte(&ee_meas_cycles);						//if(meas_cycles < 0) 		meas_cycles = MEASUREMENT_CYCLES_DEF;
		fill_timeout 		= eeprom_read_byte(&ee_fill_timeout);						//if(fill_timeout < 0) 		fill_timeout = FILLING_TIMEOUT_DEF;
		he_min 				= eeprom_read_byte(&ee_he_min);								if(he_min > 100) 				he_min = AUTO_FILL_HE_DEF;
		res_min 			= (double) (eeprom_read_word(&ee_res_min)/10.0);			if(res_min < 0) 			res_min = RES_MIN_DEF;
		res_max 			= (double) (eeprom_read_word(&ee_res_max)/10.0);			if(res_max < 0) 			res_max = RES_MAX_DEF;
		span 				= (double) (eeprom_read_word(&ee_span)/10.0);				if(span < 0) 				span = SPAN_DEF;
		zero 				= (double) (eeprom_read_word(&ee_zero)/10.0);				if(zero < 0) 				zero = ZERO_DEF;
		enable_pressure 	= eeprom_read_byte(&ee_enable_pressure);					//if(enable_pressure < 0) 	enable_pressure = 1;
		batt_min 			= (double) (eeprom_read_word(&ee_batt_min)/10.0);			if(batt_min < 0) 			batt_min = BATT_MIN_DEF;
		batt_max 			= (double) (eeprom_read_word(&ee_batt_max)/10.0);			if(batt_max < 0) 			transmit_fast = BATT_MAX_DEF;
		critical_batt 		= eeprom_read_byte(&ee_critical_batt);						//if(critical_batt < 0) 		critical_batt = CRITICAL_BATT_DEF;
	#endif
	
	
	//Options flow control vars
	uint8_t 	active_option = 1;
	uint8_t 	active_value = 0;
	uint8_t 	active_page = 1;
	bool 		options_changed = false;
	//=========================================================================
	//conversion/temp variables
	//=========================================================================
	char 		temp[SINGLE_FRAME_LENGTH];
	uint8_t 	buffer[SINGLE_FRAME_LENGTH];
	//=========================================================================
	//XBEE dependend variables
	//=========================================================================
	uint32_t 	dest_low = 0;	//0x1234; 
	uint32_t 	dest_high = 0;	//0x5678;
	
	//=========================================================================
	//enable global interrupts
	//=========================================================================
	sei();
	
	//clear screen	
	LCD_Cls(BGC);
	
	//=========================================================================
	//Connection initialization
	//=========================================================================
	LCD_Print("connecting", 5, 40, 2, 1, 1, FGC, BGC);
	LCD_Print("to network ...", 5, 60, 2, 1, 1, FGC, BGC);
	
	//=========================================================================
	//Connecting to network 
	//=========================================================================
	#ifdef ALLOW_LOGIN
		while(1)//connecting to network
		{
			if(reset_connection())
			{
				if((dest_low = get_addr(DL_MSG_TYPE)) && (dest_high = get_addr(DH_MSG_TYPE))) 
				{
					break;
				}
			}
			LCD_Cls(BGC);
			LCD_Print("Connection", 5, 20, 2, 1, 1, FGC, BGC);
			LCD_Print("to network", 5, 40, 2, 1, 1, FGC, BGC);
			LCD_Print("failed!", 5, 75, 2, 1, 1, ERR, BGC);
			LCD_Print("press any key to try again", 2, 110, 1, 1, 1, FGC, BGC);
			
			getkey();
			set_timeout(1000, TIMER_0, USE_TIMER);
			while(keyhit() != HIDDEN_FUNCTION)	
			{
				if(!set_timeout(0, TIMER_0, USE_TIMER)) 
				{
					break;	
				}
			}
			if(set_timeout(0,TIMER_0, USE_TIMER)) 
			{
				delay_ms(500);
				if(keyhit() == HIDDEN_FUNCTION)
				{
					mode = MODE_OFFLINE;
					err_code = OFFLINE;
					break;
				}
			}
			LCD_Cls(BGC);
				LCD_Print("connecting", 5, 40, 2, 1, 1, FGC, BGC);
				LCD_Print("to network ...", 5, 60, 2, 1, 1, FGC, BGC);
		}
		LCD_Cls(BGC);
		LCD_Print("connection", 5, 40, 2, 1, 1, FGC, BGC);
		LCD_Print("to network", 5, 60, 2, 1, 1, FGC, BGC);
		LCD_Print("succeeded", 5, 80, 2, 1, 1, FGC, BGC);
		delay_ms(1200);
		
	//=========================================================================
	//Device Login 
	//=========================================================================
	if(mode != MODE_OFFLINE)
	{//enter device number
		bool login_okay = false;
		uint8_t status = 0;
		bool bad_id = false;
		
		while(!login_okay)
		{
			if(!bad_id)
			{
				device_id = get_number(&status, DEVICE_ID);
				
				if(status) //hidden function und weiter gehts
				{
					login_okay = true;
					mode = MODE_OFFLINE;
					err_code = OFFLINE;
					break;
				}
				
				if(!device_id) continue;   //bei Null nochmal
				sprintf(temp,"Do you really want\nto connect vessel %d\nto server?",device_id);
			}
			else sprintf(temp,"No vessel with ID\n%d found,\nstill login?",device_id);
			
			if (LCD_Dialog("Server", temp, D_FGC, D_BGC))
			{
				//bei ja senden
				status = (bad_id)?	
					send_login_msg(FORCE_LOGIN_MSG, device_id, buffer, dest_high, dest_low)
				:	send_login_msg(LOGIN_MSG, device_id, buffer, dest_high, dest_low);
				
				switch(status)
				{
					case 0:			//GOOD_OPTIONS:	
									transmit_slow = (buffer[0]<<8) + buffer[1];
									
									if(transmit_slow <= 0) transmit_slow = TRANSMIT_SLOW_DEF;
									if(transmit_slow > 60)
									{
										transmit_slow /= 60;
										transmit_slow_min = false;
									}
									else transmit_slow_min = true;
									
									transmit_fast = (buffer[2]<<8) + buffer[3];
									if(transmit_fast <= 0) transmit_fast = TRANSMIT_FAST_DEF;
									if(transmit_fast > 60)
									{
										transmit_fast /= 60;
										transmit_fast_sec = false;
									}
									else transmit_fast_sec = true;
									
									res_min = ((buffer[4]<<8) + buffer[5])/10.0;
									if(res_min <= 0) res_min = RES_MIN_DEF;
									
									res_max = ((buffer[6]<<8) + buffer[7])/10.0;
									if(res_max <= 0) res_max = RES_MAX_DEF;
									
									heat_time = (buffer[8]<<8) + buffer[9];
									heat_time = (heat_time <= 0)? HEAT_TIME_DEF : round_double(heat_time/1000.0, 1);
									
									meas_cycles = (!buffer[10])? MEASUREMENT_CYCLES_DEF : buffer[10];
									fill_timeout = (!buffer[11])? FILLING_TIMEOUT_DEF : buffer[11];
									
									//span & zero
									span = ((buffer[12]<<8) + buffer[13])/10.0;
									if(span <= 0) span = SPAN_DEF;
									
									zero = ((buffer[14]<<8) + buffer[15])/10.0;
									zero = ZERO_NULLPUNKT - zero;
									
									batt_min = ((buffer[16]<<8) + buffer[17])/10.0;
									if(batt_min <= 0) batt_min = BATT_MIN_DEF;
									
									batt_max = ((buffer[18]<<8) + buffer[19])/10.0;
									if(batt_max <= 0) batt_max = BATT_MAX_DEF;
									
									critical_batt = (!buffer[20])? CRITICAL_BATT_DEF : buffer[20];
									
									login_okay = true;
									break;
					case 1:			//BAD_OPTIONS:
									transmit_slow = TRANSMIT_SLOW_DEF;
									transmit_slow_min = false;
									transmit_fast = TRANSMIT_FAST_DEF;
									transmit_fast_sec = false;
									heat_time = HEAT_TIME_DEF;
									meas_cycles = MEASUREMENT_CYCLES_DEF;
									fill_timeout = FILLING_TIMEOUT_DEF;
									res_min = RES_MIN_DEF;
									res_max = RES_MAX_DEF;
									span = SPAN_DEF;
									zero = ZERO_DEF;
									batt_min = BATT_MIN_DEF;
									batt_max = BATT_MAX_DEF;
									critical_batt = CRITICAL_BATT_DEF;
									
									login_okay = true;
									break;
					case 2:			//BAD_DEVICE_ID:
									bad_id = true;
									break;
					case 3:			//TOP_SECRET: hidden function
									mode = MODE_OFFLINE;
									err_code = OFFLINE;
									login_okay = true;
									break;
					case 0xFF:		//FAILED_LOGIN:
									mode = MODE_ERROR;
									err_code = LOGIN_FAILED;
									login_okay = true;
									break;
					default:		
									break;
				}//switch status
			}else bad_id = false;  //bei Nein muss du eben noch mal eingeben
		}//while login_okay
	}//if(mode != MODE_OFFLINE)
	#else
		device_id = 1;
	#endif
	//=========================================================================
	//Initial setup 
	//=========================================================================
	switch(err_code)
	{
		case NO_ERROR:	
						#ifdef ALLOW_LOGIN
							//1. measurement
							MEASURE_PIN_ON
							he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, heat_time, 0);
							batt_level = get_batt_level(batt_min, batt_max,r_zero);
							pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
							MEASURE_PIN_OFF
							
							buffer[0] = device_id >> 8;
							buffer[1] = device_id;
							
							if(he_level < 0)  //dont send err HE (-1, -2, -3 ..), send some code instead
							{
								buffer[2] = ERROR_HE_LEVEL>>8;
								buffer[3] = (uint8_t) ERROR_HE_LEVEL;
							}
							else {
								buffer[2] = ((uint16_t)(he_level*10))>>8;
								buffer[3] = ((uint16_t)(he_level*10));
							}
							buffer[4] = batt_level;
							buffer[5] = pressure_level >> 8;
							buffer[6] = pressure_level;
							buffer[7] = status_byte;
							
							sending_cmd = pack_tx64_frame(STATUS_MSG, buffer, 8, dest_high, dest_low);
							basic_send(buffer);
						#endif
						//paint screen
						paint_main(device_id,device_pos,he_level,batt_level, critical_batt, PAINT_ALL);
						//set slow transmission; transmit_slow_min indicates whether its in min or sec
						(transmit_slow_min)? 
								set_timeout(ceil(transmit_slow*60), TIMER_1, USE_TIMER)
							:	set_timeout(transmit_slow*3600, TIMER_1, USE_TIMER);
							
						//set XBEE SLEEP and start waiting to wake it up
						xbee_sleep();
						xbee_sleeping = true;
						set_timeout(0, TIMER_5, RESET_TIMER);
						set_timeout(xbee_sleep_period*60, TIMER_5, USE_TIMER);
						break;
		case OFFLINE:	
						//hidden function mode
						paint_offline(device_id,device_pos,he_level,batt_level,critical_batt,0);
						xbee_sleep();
						break;
		default:		
						break;
	}	
	#ifdef ALLOW_DISPLAY_TIMER
		set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
	#endif
	
	//=========================================================================
	//Main loop 
	//=========================================================================
	while(1) 
	{	
		//=========================================================================
		//INTERVAL SLOW 
		//=========================================================================
		if((err_code != OFFLINE) && (mode == MODE_MAIN_WINDOW)) 	//das gleiche eig.
		{
			if(!set_timeout(0, TIMER_1, USE_TIMER)) 
			{
				paint_wait_screen();
				xbee_wake_up();
				
				MEASURE_PIN_ON
				he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, heat_time, 0);
				batt_level = get_batt_level(batt_min, batt_max,r_zero);
				pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
				MEASURE_PIN_OFF
				
				buffer[0] = device_id>>8;
				buffer[1] = device_id;
				
				if(he_level < 0)  //dont send err HE (-1, -2, -3 ..), send some code instead
				{
					buffer[2] = ERROR_HE_LEVEL>>8;
					buffer[3] = (uint8_t) ERROR_HE_LEVEL;
				}
				else {
					buffer[2] = ((uint16_t)(he_level*10))>>8;
					buffer[3] = ((uint16_t)(he_level*10));
				}
				
				buffer[4] = batt_level;
				buffer[5] = pressure_level >> 8;
				buffer[6] = pressure_level;
				buffer[7] = status_byte;
				
				#ifdef ALLOW_COM
					send_request(LONG_INTERVAL_MSG, buffer, 8, &dest_high, &dest_low);
				#else
					sending_cmd = pack_tx64_frame(LONG_INTERVAL_MSG, buffer, 8, dest_high, dest_low);
					basic_send(buffer);
				#endif
				//set timer for the next delay
				(transmit_slow_min)? 
					set_timeout(ceil(transmit_slow*60), TIMER_1, USE_TIMER)
				:	set_timeout(transmit_slow*3600, TIMER_1, USE_TIMER);
				
				xbee_sleep();
				paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
			}
		}
		
		//execute code according to mode 
		switch(mode)
		{
		case	MODE_MAIN_WINDOW:	
									switch(keyhit())
									{	
										//reconnect
										case KEY_LEFT_2:
														if (!display_on()) 
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														LCD_Cls(BGC);
														LCD_Print("connecting...", 5, 50, 2, 1, 1, FGC, BGC);
														xbee_wake_up();
														reconnect(&dest_low, &dest_high);
														xbee_sleep();
														(CHECK_NETWORK_ERROR)? 
															LCD_Dialog("Network", "Couldn't connect\nto network", D_FGC, D_BGC)
														:	LCD_Dialog("Network", "Connection\nsuccessfull", D_FGC, D_BGC);
														
														paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
														break;
										//options			
										case KEY_LEFT_1:
														{//scope needed for var
															if (!display_on())
															#ifndef ALLOW_DOUBLE_CLICK 
																break; 
															#else
																;
															#endif
															
															uint8_t status;	//pressed "cancel" or "okay"
															entered_options_pw = get_number(&status, PASSWORD);
															
															if(status || (options_pw != entered_options_pw))
															{
																paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
																break;	//if not 0 -> no options
															}
															
															mode = MODE_OPTIONS_WINDOW;
															//init options flow control
															active_option = 1;
															active_value = 0;
															active_page = 1;
															paint_options(active_page);
															//show current option values
															paint_opt_values_page1(transmit_slow_min, transmit_slow, transmit_fast, heat_time, meas_cycles, fill_timeout);
														}
														break;
														
										case KEY_UP:	
														{
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
													
																								
														//if( PINB & (1<<PB7) && PINB & (1<<PB6)) 
															{
															
															uint8_t status;	//pressed "cancel" or "okay"
															entered_options_pw = get_number(&status, PASSWORD);
															
															
																if(status || (options_pw != entered_options_pw))
																{
																	paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
																	break;	//if not 0 -> no options
																}
															
																if(LCD_Dialog(STRING_SHUTDOWN,"Do you really\nwant to shut down\nthe system?", D_FGC, D_BGC))
																				{
																					LCD_Cls(BGC);
																					mode = MODE_SHUTDOWN_ACTION;
																				}
																				else {
																					paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
																				}
																				break;
															
															}
														}
														break;
										
										
									
	
														
										case KEY_DOWN:	
														{
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
													
																								
														//if( PINB & (1<<PB7) && PINB & (1<<PB6)) 
															{
															
															uint8_t status;	//pressed "cancel" or "okay"
															entered_options_pw = get_number(&status, PASSWORD);
															
															
																if(status || (options_pw != entered_options_pw))
																{
																	paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
																	break;	//if not 0 -> no options
																}
															
																if(LCD_Dialog(STRING_SHUTDOWN,"Do you really\nwant to shut down\nthe system?", D_FGC, D_BGC))
																				{
																					LCD_Cls(BGC);
																					mode = MODE_SHUTDOWN_ACTION;
																				}
																				else {
																					paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
																				}
																				break;
															
															}
														}
														break;
														
										


				
										//fill procedure				
										case KEY_FILL:	
														{
														if (!display_on())
															#ifndef ALLOW_DOUBLE_CLICK 
																break; 
															#else
																;
															#endif
														if (!no_he_level)	
														{//filling only if actual level is higher then min. level in options
															if(!update_filling_pos(fill_pos_letters, fill_pos_ranges, temp))
															{
																paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
																break;
															}
															//copy position bytes from temp to device_pos
															//each digit is a byte
															strcpy(device_pos, temp);
															LCD_Cls(BGC);
															LCD_Print("sending data...", 5, 50, 2, 1, 1, FGC, BGC);
															
															//pack message...device id, pos
															uint8_t indx = 0;
															buffer[0] = device_id>>8;
															buffer[1] = device_id;
															indx = pos_to_binary(device_pos, 2, buffer);
															buffer[indx++] = status_byte;
															
															xbee_wake_up();
															xbee_busy = true;
															
															//message packed, now send indx bytes and get an answer!
															#ifdef ALLOW_COM
																send_request(FILLING_BEGIN_MSG, buffer, indx, &dest_high, &dest_low);
															#else
																sending_cmd = pack_tx64_frame(FILLING_BEGIN_MSG, buffer, indx, dest_high, dest_low);
																basic_send(buffer);
															#endif
															
															paint_filling(device_id, device_pos, he_level, batt_level, critical_batt,0);
															//number of measurements 
															fill_meas_counter = (transmit_fast_sec)?
																((((uint16_t) fill_timeout*60))/transmit_fast) +1
															:	(fill_timeout/transmit_fast) + 1;
															//draw_int(fill_meas_counter, 5, 115, "", blue);
															mode = MODE_FILL_ACTION;
														}
														else {//he lvl is too low, back to main
															LCD_Dialog("HE LEVEL", "Filling is disabled.\nHe Level is too low.", D_FGC, D_BGC);
															
															mode = MODE_MAIN_WINDOW;
															paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
														}
														//delay_ms(300);
														break;
														}
										//SINGLE MEASUREMENT				
										case KEY_RIGHT_1:	
														if (!display_on())
															#ifndef ALLOW_DOUBLE_CLICK 
																break; 
															#else
																;
															#endif
														paint_measure(device_id, device_pos, he_level, batt_level, critical_batt, 0);
														mode = MODE_MEASURE_ACTION;
														break;
										default:		
														//do stuff according to mode here
														break;
									}
									break;
		case	MODE_FILL_ACTION:
									switch(keyhit())
									{
										case KEY_LEFT_2:	
														if (!display_on())
															#ifndef ALLOW_DOUBLE_CLICK 
																break; 
															#else
																;
															#endif
														//delay_ms(300);
														break;
													
										case KEY_LEFT_1:	
														if (!display_on())
															#ifndef ALLOW_DOUBLE_CLICK 
																break; 
															#else
																;
															#endif
														fill_meas_counter++;		//more filling needed
														if(transmit_fast_sec) {
															if(fill_meas_counter > ((((uint16_t) fill_timeout*60))/transmit_fast))	 	fill_meas_counter = (((uint16_t) fill_timeout*60))/transmit_fast;
														} else {
															if(fill_meas_counter > (fill_timeout/transmit_fast)) 						fill_meas_counter = fill_timeout/transmit_fast;
														}
														sprintf(temp, FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
														LCD_Print(temp, 5, 105, 1, 1, 1, FGC, BGC);
														break;
														
										case KEY_UP:		
														if (!display_on())
															#ifndef ALLOW_DOUBLE_CLICK 
																break; 
															#else
																;
															#endif
														//delay_ms(300);
														break;
														
										case KEY_DOWN:	
														if (!display_on())
															#ifndef ALLOW_DOUBLE_CLICK 
																break; 
															#else
																;
															#endif
														//delay_ms(300);
														break;
														
										case KEY_RIGHT_2:	
														if (!display_on())
															#ifndef ALLOW_DOUBLE_CLICK 
																break; 
															#else
																;
															#endif
														//delay_ms(300);
														break;
										//stop filling				
										case KEY_RIGHT_1:	
														if (!display_on())
															#ifndef ALLOW_DOUBLE_CLICK 
																break; 
															#else
																;
															#endif
														if(LCD_Dialog("Filling","Do you really\nwant to stop\nfilling procedure?", D_FGC, D_BGC))
														{
															set_timeout(0, TIMER_4, RESET_TIMER); 		//clear timer!
															LCD_Cls(BGC);
															LCD_Print("sending data...", 5, 50, 2, 1, 1, FGC, BGC);		
															
															buffer[0] = device_id>>8;
															buffer[1] = device_id;
															
															if(he_level < 0)  //dont send err HE (-1, -2, -3 ..), send some code instead
															{
																//uint16_t error_he = ERROR_HE_LEVEL;
																buffer[2] = ERROR_HE_LEVEL>>8;
																buffer[3] = (uint8_t) ERROR_HE_LEVEL;
															}
															else {
																buffer[2] = ((uint16_t)(he_level*10))>>8;
																buffer[3] = ((uint16_t)(he_level*10));
															}
															buffer[4] = batt_level;
															buffer[5] = pressure_level >> 8;
															buffer[6] = pressure_level;
															buffer[7] = status_byte;
															
															//send 8 bytes
															#ifdef ALLOW_COM
																send_request(FILLING_END_MSG, buffer, 8, &dest_high, &dest_low);
															#else
																sending_cmd = pack_tx64_frame(FILLING_END_MSG, buffer, 8, dest_high, dest_low);
																basic_send(buffer);
															#endif
															//regardless communication errors continue anyway
															paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
															mode = MODE_MAIN_WINDOW;
															xbee_sleep();
															xbee_busy = false;
														}
														else {
																paint_filling(device_id, device_pos, he_level, batt_level,critical_batt, 0);
																LCD_Print("waiting  ", 2, 20, 2, 1, 1, FGC, BGC);
																sprintf(temp, FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
																LCD_Print(temp, 5, 100, 1, 1, 1, FGC, BGC);
															}
														break;
										default:				
														{
														bool filling_end = false;
														
														if(!(meas_progress = set_timeout(0, TIMER_4, USE_TIMER)))
														{//time between measurments is passed, new measurement
															
															LCD_Print("measuring     ", 2, 20, 2, 1, 1, FGC, BGC);
															(transmit_fast_sec)?
																set_timeout(transmit_fast, TIMER_4, USE_TIMER)
															:	set_timeout(transmit_fast*60, TIMER_4, USE_TIMER);
															
															MEASURE_PIN_ON
															he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, heat_time, 1);
															batt_level = get_batt_level(batt_min, batt_max,r_zero);
															pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
															MEASURE_PIN_OFF
															
															paint_filling(device_id,device_pos,he_level,batt_level,critical_batt,1);
															
															//send data	
															//uint8_t indx = 0;
															buffer[0] = device_id>>8;
															buffer[1] = device_id;
															//indx = pos_to_binary(device_pos, 2, buffer);
															
															if(he_level < 0)  //dont send err HE (-1, -2, -3 ..), send some code instead
															{
																buffer[2] = ERROR_HE_LEVEL>>8;
																buffer[3] = (uint8_t) ERROR_HE_LEVEL;
															}
															else {
																buffer[2] = ((uint16_t)(he_level*10))>>8;
																buffer[3] = ((uint16_t)(he_level*10));
															}
															
															buffer[4] = batt_level;
															buffer[5] = pressure_level >> 8;
															buffer[6] = pressure_level;
															buffer[7] = status_byte;
															
															//just send, no answer needed
															sending_cmd = pack_tx64_frame(FILLING_MSG, buffer, 8, dest_high, dest_low);
															basic_send(buffer);
															
															fill_meas_counter--;
															delay_ms(150);
															
															//if autofilling and no more signal - end
															if(!auto_fill_pin_on() && auto_fill_started)
															{
																//FE
																filling_end = true;
																//set auto fill pin low
																STOP_AUTO_FILL
																auto_fill_started = false;
															}
															//if normal filling and counter=0 - end
															if(!fill_meas_counter && !auto_fill_started)
															{
																//FE
																filling_end = true;
															}
															//he_level twice lower then min, end/disable filling
															if((he_level < he_min) && !auto_fill_started)
															{
																if(last_he_level < he_min) 
																{
																	STOP_AUTO_FILL
																	timed_dialog("HE LEVEL", "Filling is disabled.\nHe Level is too low.", 10, D_FGC, D_BGC); 
																	no_he_level = true;
																	
																	//FE
																	filling_end = true;
																	auto_fill_started = false;
																	auto_fill_enabled = false;		//disable autofill regardless any signals
																}
															}
															last_he_level = he_level;
															if(!filling_end)
															{
																//set timer till next measurement
																LCD_Print("waiting  ", 2, 20, 2, 1, 1, FGC, BGC);
																clear_progress_bar(5, 105);
																
																sprintf(temp, FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
																LCD_Print(temp, 5, 105, 1, 1, 1, FGC, BGC);
															}
														}//if(timer ready) 
														else if(!auto_fill_pin_on() && auto_fill_started)
															{
																//FE
																filling_end = true;
																//set auto fill pin low
																STOP_AUTO_FILL
																auto_fill_started = false;
															}
															else {
																(transmit_fast_sec)? 	draw_current_wait_time(70, 20, transmit_fast, meas_progress, FGC)
																:						draw_current_wait_time(70, 20, transmit_fast*60, meas_progress, FGC);
															}
															
														if(filling_end)
														{
															//send end ..
															buffer[0] = device_id>>8;
															buffer[1] = device_id;
															
															if(he_level < 0)  //dont send err HE (-1, -2, -3 ..), send some code instead
															{
																//uint16_t error_he = ERROR_HE_LEVEL;
																buffer[2] = ERROR_HE_LEVEL>>8;
																buffer[3] = (uint8_t) ERROR_HE_LEVEL;
															}
															else {
																buffer[2] = ((uint16_t)(he_level*10))>>8;
																buffer[3] = ((uint16_t)(he_level*10));
															}
															
															buffer[4] = batt_level;
															buffer[5] = pressure_level >> 8;
															buffer[6] = pressure_level;
															buffer[7] = status_byte;
															
															#ifdef ALLOW_COM
																send_request(FILLING_END_MSG, buffer, 8, &dest_high, &dest_low);
															#else
																sending_cmd = pack_tx64_frame(FILLING_END_MSG, buffer, 8, dest_high, dest_low);
																basic_send(buffer);
															#endif
															paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
															mode = MODE_MAIN_WINDOW;
															
															xbee_sleep();
															xbee_busy = false;
														}
														}//scope
														break;
									}
									break;
		case 	MODE_MEASURE_ACTION:
								


									
									
									MEASURE_PIN_ON
									he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, heat_time, 0);
									batt_level = get_batt_level(batt_min, batt_max,r_zero);
									pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
									MEASURE_PIN_OFF
									
									xbee_wake_up();
									
									buffer[0] = device_id>>8;
									buffer[1] = device_id;
									
									if(he_level < 0)  //dont send err HE (-1, -2, -3 ..), send some code instead
									{
										buffer[2] = ERROR_HE_LEVEL>>8;
										buffer[3] = (uint8_t) ERROR_HE_LEVEL;
									}
									else {
										buffer[2] = ((uint16_t)(he_level*10))>>8;
										buffer[3] = ((uint16_t)(he_level*10));
									}
									
									buffer[4] = batt_level;
									buffer[5] = pressure_level >> 8;
									buffer[6] = pressure_level;
									buffer[7] = status_byte;
									
									
									
									#ifdef ALLOW_COM
										send_request(LONG_INTERVAL_MSG, buffer, 8, &dest_high, &dest_low);
									#else
										sending_cmd = pack_tx64_frame(LONG_INTERVAL_MSG, buffer, 8, dest_high, dest_low);
										basic_send(buffer);
									#endif
									
									
									xbee_sleep();
									
									//paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
														
														
								
									
									//paint_measure(device_id, device_pos, he_level, batt_level, 1);
									
									if(force_measurement)
									{
										did_measurement = true;
									}
									else if(err_code == OFFLINE)
									{
										paint_offline(device_id,device_pos,he_level,batt_level,critical_batt,0);
										mode = MODE_OFFLINE;
									}
									else {
										paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
										mode = MODE_MAIN_WINDOW;
									}
									break;
		case	MODE_OPTIONS_WINDOW:
									switch(active_page)
									{
										case 1://general
												switch(keyhit())
												{
													case KEY_LEFT_2:
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 0:	
																				active_option++;
																				if(active_option == 6) {active_option = 1;}
																				paint_current_opt_page1(active_option, KEY_LEFT_2);
																				break;
																		case 1:	
																				//decrease transmission period
																				transmit_slow--;
																				if(!transmit_slow) 
																				{
																					if(!transmit_slow_min)
																					{
																						transmit_slow = 60;
																						transmit_slow_min = true;
																					}
																					else {
																						transmit_slow = 1;
																					}
																				}
																				(transmit_slow_min) ? 
																					draw_int(transmit_slow, 85, 20, "min", ERR) 
																				: 	draw_int(transmit_slow, 85, 20, "h", ERR);
																				
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					transmit_slow--;
																					if(!transmit_slow) 
																					{
																						if(!transmit_slow_min)
																						{
																							transmit_slow=60;
																							transmit_slow_min = true;
																						}
																						else {
																							transmit_slow = 1;
																						}
																					}
																					(transmit_slow_min) ? 
																						draw_int(transmit_slow, 85, 20, "min", ERR) 
																					: 	draw_int(transmit_slow, 85, 20, "h", ERR);
																					delay_ms(50);
																				}
																				
																				transmit_slow_changed = true;
																				break;
																		case 2: 
																				//decrease transmission period while filling
																				transmit_fast--;
																				if(!transmit_fast) 
																				{
																					if(!transmit_fast_sec)
																					{
																						transmit_fast = 60;
																						transmit_fast_sec = true;
																					}
																					else {
																						transmit_fast = 1;
																					}
																				}
																				if(transmit_fast < MIN_TRANSMIT_FAST) transmit_fast = MAX_TRANSMIT_FAST;
																				(transmit_fast_sec) ? 
																					draw_int(transmit_fast, 85, 40, "sec", ERR) 
																				: 	draw_int(transmit_fast, 85, 40, "min", ERR);
																				
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					transmit_fast--;
																					if(!transmit_fast) 
																					{
																						if(!transmit_fast_sec)
																						{
																							transmit_fast = 60;
																							transmit_fast_sec = true;
																						}
																						else {
																							transmit_fast = 1;
																						}
																					}
																					if(transmit_fast < MIN_TRANSMIT_FAST) transmit_fast = MAX_TRANSMIT_FAST;
																					(transmit_fast_sec) ? 
																						draw_int(transmit_fast, 85, 40, "sec", ERR) 
																					: 	draw_int(transmit_fast, 85, 40, "min", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 3:	
																				//decrease t o
																				heat_time-=0.1;
																				if(heat_time < MIN_HEAT_TIME) heat_time = MAX_HEAT_TIME;
																				draw_double(heat_time, 76, 60, 1,"s", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					heat_time-=0.1;
																					if(heat_time < MIN_HEAT_TIME) heat_time = MAX_HEAT_TIME;
																					draw_double(heat_time, 76, 60, 1, "s", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 4:	
																				//decrease t n
																				meas_cycles--;
																				if(meas_cycles < MIN_MEASUREMENT_CYCLES) meas_cycles = MAX_MEASUREMENT_CYCLES;
																				draw_int(meas_cycles, 85, 80, "c", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					meas_cycles--;
																					if(meas_cycles < MIN_MEASUREMENT_CYCLES) meas_cycles = MAX_MEASUREMENT_CYCLES;
																					draw_int(meas_cycles, 85, 80, "c", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 5:	
																				//decrease fill_timeout
																				fill_timeout--;
																				if(fill_timeout < MIN_FILLING_TIMEOUT) fill_timeout = MAX_FILLING_TIMEOUT;
																				draw_int(fill_timeout, 85, 100, "min", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					fill_timeout--;
																					if(fill_timeout < MIN_FILLING_TIMEOUT) fill_timeout = MAX_FILLING_TIMEOUT;
																					draw_int(fill_timeout, 85, 100, "min", ERR);
																					delay_ms(50);
																				}
																				break;
																	}
																	options_changed = true;
																	break;
																
													case KEY_LEFT_1:
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	active_page = 2;
																	paint_options(active_page);
																	active_option = 1;
																	active_value = 0;
																	paint_opt_values_page2(res_min, res_max, batt_min, batt_max, critical_batt);
																	break;
																	
													case KEY_UP:		
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 1:	
																				active_value=0;
																				LCD_Print(STRING_TRANSMIT_SLOW, 2, 20, 2, 1,1, ERR, BGC);
																				(transmit_slow_min) ? 
																					draw_int(transmit_slow, 85, 20, "min", FGC) 
																				: 	draw_int(transmit_slow, 85, 20, "h", FGC);
																				break;
																		case 2: 
																				active_value=0;
																				LCD_Print(STRING_TRANSMIT_FAST, 2, 40, 2, 1,1, ERR, BGC);
																				draw_int(transmit_fast, 85, 40, "min", FGC);
																				break;
																		case 3: 
																				active_value=0;
																				LCD_Print(STRING_HEAT_TIME, 2, 60, 2, 1,1, ERR, BGC);
																				draw_double(heat_time, 76, 60, 1, "s", FGC);
																				break;
																		case 4: 
																				active_value=0;
																				LCD_Print(STRING_MEASUREMENT_CYCLES, 2, 80, 2, 1,1, ERR, BGC);
																				draw_int(meas_cycles, 85, 80, "c", FGC);
																				break;
																		case 5: 
																				active_value=0;
																				LCD_Print(STRING_FILLING_TIMEOUT, 2, 100, 2, 1,1, ERR, BGC);
																				draw_int(fill_timeout, 85, 100, "min", FGC);
																				break;
																	}
																	break;
																	
													case KEY_DOWN:	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	if (!active_value) 
																	{
																		switch(active_option) 
																		{
																			case 1:	
																					active_value=1;
																					LCD_Print(STRING_TRANSMIT_SLOW, 2, 20, 2, 1,1, FGC, BGC);
																					(transmit_slow_min) ? 
																						draw_int(transmit_slow, 85, 20, "min", ERR) 
																					: 	draw_int(transmit_slow, 85, 20, "h", ERR);
																					break;
																			case 2: 
																					active_value=2;
																					LCD_Print(STRING_TRANSMIT_FAST, 2, 40, 2, 1,1, FGC, BGC);
																					draw_int(transmit_fast, 85, 40, "min", ERR);
																					break;
																			case 3: 
																					active_value=3;
																					LCD_Print(STRING_HEAT_TIME, 2, 60, 2, 1,1, FGC, BGC);
																					draw_double(heat_time, 76, 60, 1, "s", ERR);
																					break;
																			case 4: 
																					active_value=4;
																					LCD_Print(STRING_MEASUREMENT_CYCLES, 2, 80, 2, 1,1, FGC, BGC);
																					draw_int(meas_cycles, 85, 80, "c", ERR);
																					break;
																			case 5: 
																					active_value=5;
																					LCD_Print(STRING_FILLING_TIMEOUT, 2, 100, 2, 1,1, FGC, BGC);
																					draw_int(fill_timeout, 85, 100, "min", ERR);
																					break;
																		}
																	}
																	break;
																	
													case KEY_RIGHT_2:	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 0: 
																				active_option--;
																				if(!active_option) active_option = 5;	
																				
																				paint_current_opt_page1(active_option, KEY_RIGHT_2);
																				break;
																		case 1:	
																				//increase transmission period
																				transmit_slow++;
																				if((transmit_slow > MAX_TRANSMIT_SLOW) && (!transmit_slow_min)) transmit_slow = MIN_TRANSMIT_SLOW;
																				
																				if((transmit_slow > 60) && (transmit_slow_min))
																				{
																					transmit_slow=1;
																					transmit_slow_min = false;
																					LCD_Print("     ", 85, 20, 2, 1,1, ERR, BGC);
																				}
																				(transmit_slow_min) ? 
																					draw_int(transmit_slow, 85, 20, "min", ERR) 
																				: 	draw_int(transmit_slow, 85, 20, "h", ERR);
																				
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					transmit_slow++;
																					if((transmit_slow > MAX_TRANSMIT_SLOW) && (!transmit_slow_min)) transmit_slow = MIN_TRANSMIT_SLOW;
																					
																					if((transmit_slow > 60) && (transmit_slow_min))
																					{
																						transmit_slow=1;
																						transmit_slow_min = false;
																						LCD_Print("     ", 85, 20, 2, 1,1, ERR, BGC);
																					}
																					(transmit_slow_min) ? 
																						draw_int(transmit_slow, 85, 20, "min", ERR) 
																					: 	draw_int(transmit_slow, 85, 20, "h", ERR);
																					delay_ms(50);
																				}//while(keyhit()==KEY_RIGHT_2)
																				
																				transmit_slow_changed = true;
																				break;
																		case 2: 
																				//increase transmission period while filling
																				transmit_fast++;
																				if((transmit_fast > MAX_TRANSMIT_FAST) && (!transmit_fast_sec)) transmit_fast = MIN_TRANSMIT_FAST;
																				
																				if((transmit_fast>60) && (transmit_fast_sec)) 
																				{
																					transmit_fast=1;
																					transmit_fast_sec = false;
																					LCD_Print("     ", 85, 40, 2, 1,1, ERR, BGC);
																				}
																				(transmit_fast_sec) ? 
																					draw_int(transmit_fast, 85, 40, "sec", ERR) 
																				: 	draw_int(transmit_fast, 85, 40, "min", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					transmit_fast++;
																					if((transmit_fast > MAX_TRANSMIT_FAST) && (!transmit_fast_sec)) transmit_fast = MIN_TRANSMIT_FAST;
																					
																					if((transmit_fast>60) && (transmit_fast_sec)) 
																					{
																						transmit_fast=1;
																						transmit_fast_sec = false;
																						LCD_Print("     ", 85, 40, 2, 1,1, ERR, BGC);
																					}
																					(transmit_fast_sec) ? 
																						draw_int(transmit_fast, 85, 40, "sec", ERR) 
																					: 	draw_int(transmit_fast, 85, 40, "min", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 3:	
																				//increase t o
																				heat_time+=0.1;
																				if(heat_time > MAX_HEAT_TIME) heat_time = MIN_HEAT_TIME;
																				draw_double(heat_time, 76, 60, 1, "s", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					heat_time+=0.1;
																					if(heat_time > MAX_HEAT_TIME) heat_time = MIN_HEAT_TIME;
																					draw_double(heat_time, 76, 60, 1, "s", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 4:	
																				//increase t n
																				meas_cycles++;
																				if(meas_cycles > MAX_MEASUREMENT_CYCLES) meas_cycles = MIN_MEASUREMENT_CYCLES;
																				draw_int(meas_cycles, 85, 80, "c", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					meas_cycles++;
																					if(meas_cycles > MAX_MEASUREMENT_CYCLES) meas_cycles = MIN_MEASUREMENT_CYCLES;
																					draw_int(meas_cycles, 85, 80, "c", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 5:	
																				//increase fill_timeout
																				fill_timeout++;
																				if(fill_timeout > MAX_FILLING_TIMEOUT) fill_timeout = MIN_FILLING_TIMEOUT;
																				draw_int(fill_timeout, 85, 100, "min", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					fill_timeout++;
																					if(fill_timeout > MAX_FILLING_TIMEOUT) fill_timeout = MIN_FILLING_TIMEOUT;
																					draw_int(fill_timeout, 85, 100, "min", ERR);
																					delay_ms(50);
																				}
																				break;
																	}
																	options_changed = true;
																	break;
																	
													case KEY_RIGHT_1:	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	if(err_code != OFFLINE)
																	{
																		if(options_changed)
																		{
																			xbee_wake_up();
																			buffer[0] = device_id>>8;
																			buffer[1] = device_id;
																			
																			if(transmit_slow_min) 
																			{
																				buffer[2] = transmit_slow>>8;
																				buffer[3] = transmit_slow;
																			}
																			else {
																				buffer[2] = (transmit_slow*60)>>8;
																				buffer[3] = transmit_slow*60;
																			}
																			if(transmit_fast_sec) 
																			{
																				buffer[4] = transmit_fast>>8;
																				buffer[5] = transmit_fast;
																			}
																			else {
																				buffer[4] = (transmit_fast*60)>>8;
																				buffer[5] = transmit_fast*60;
																			}
																			buffer[6] = ((uint16_t)(res_min*10))>>8;
																			buffer[7] = res_min*10;
																			buffer[8] = ((uint16_t)(res_max*10))>>8;
																			buffer[9] = res_max*10;
																			buffer[10] = ((uint16_t)(heat_time*1000))>>8;
																			buffer[11] = (heat_time*1000);
																			buffer[12] = meas_cycles;
																			buffer[13] = fill_timeout;
																			buffer[14] = ((uint16_t)(span*10))>>8;
																			buffer[15] = span*10;
																			buffer[16] = ((uint16_t)((ZERO_NULLPUNKT+zero)*10))>>8;
																			buffer[17] = (ZERO_NULLPUNKT+zero)*10;
																			buffer[18] = ((uint16_t)(batt_min*10))>>8;
																			buffer[19] = batt_min*10;
																			buffer[20] = ((uint16_t)(batt_max*10))>>8;
																			buffer[21] = batt_max*10;
																			buffer[22] = critical_batt;
																			buffer[23] = status_byte;
																			
																			LCD_Cls(BGC);
																			LCD_Print("saving settings...", 5, 50, 2, 1, 1, FGC, BGC);
																			
																			#ifdef ALLOW_COM
																				send_request(OPTIONS_CHANGED_MSG, buffer, 24, &dest_high, &dest_low);
																			#else
																				sending_cmd = pack_tx64_frame(OPTIONS_CHANGED_MSG, buffer, 23, dest_high, dest_low);
																				basic_send(buffer);
																			#endif
																			xbee_sleep();
																			
																			#ifdef ALLOW_EEPROM_SAVING
																				eeprom_write_word(&ee_r_zero, (uint16_t)(r_zero*10));
																				eeprom_write_word(&ee_r_span, (uint16_t)(r_span*10));
																				
																				eeprom_write_word(&ee_transmit_slow, transmit_slow);
																				eeprom_write_byte(&ee_transmit_slow_min, transmit_slow_min);
																				eeprom_write_word(&ee_transmit_fast, transmit_fast);
																				eeprom_write_byte(&ee_transmit_fast_sec, transmit_fast_sec);
																				eeprom_write_word(&ee_heat_time, (uint16_t) (heat_time*1000));							
																				
																				eeprom_write_byte(&ee_meas_cycles, meas_cycles);
																				eeprom_write_byte(&ee_fill_timeout, fill_timeout);
																				eeprom_write_byte(&ee_he_min, he_min);
																				eeprom_write_word(&ee_res_min, (uint16_t) (res_min*10));
																				eeprom_write_word(&ee_res_max, (uint16_t) (res_max*10));
																				eeprom_write_word(&ee_span, (uint16_t) (span*10));
																				eeprom_write_word(&ee_zero, (uint16_t) (zero*10));
																				eeprom_write_byte(&ee_enable_pressure, enable_pressure);
																				eeprom_write_word(&ee_batt_min, (uint16_t) (batt_min*10));
																				eeprom_write_word(&ee_batt_max, (uint16_t) (batt_max*10));
																				eeprom_write_byte(&ee_critical_batt, critical_batt);
																			#endif
																		}
																		
																		paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
																		mode = MODE_MAIN_WINDOW;
																		options_changed = false;
																	}
																	else {
																		mode = MODE_OFFLINE;
																		paint_offline(device_id,device_pos,he_level,batt_level,critical_batt,0);
																	}
																	break;
													default:			
																	break;
												}
												break;
										case 2: //R
												switch(keyhit())
												{
													case KEY_LEFT_2:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 0: 
																				active_option++;
																				if(active_option == 6) {active_option = 1;}
																				paint_current_opt_page2(active_option, KEY_LEFT_2);
																				break;
																		case 1:	
																				//decrease r min
																				res_min-=0.1;
																				if(res_min < MIN_RES_MIN) res_min = MAX_RES_MIN;
																				draw_double(res_min, 76, 20, 1, "o", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					res_min-=0.1;
																					if(res_min < MIN_RES_MIN) res_min = MAX_RES_MIN;
																					draw_double(res_min, 76, 20, 1, "o", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 2:	
																				//decrease r max
																				res_max-=0.1;
																				if(res_max < MIN_RES_MAX) res_max = MAX_RES_MAX;
																				draw_double(res_max, 76, 40, 1, "o", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					res_max--;
																					if(res_max < MIN_RES_MAX) res_max = MAX_RES_MAX;
																					draw_double(res_max, 76, 40, 1, "o", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 3:	
																				//decrease batt min
																				batt_min-=0.1;
																				if(batt_min < MIN_BATTMIN) batt_min = MAX_BATTMIN;
																				draw_double(batt_min, 76, 60, 1, "V", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					batt_min-=0.1;
																					if(batt_min < MIN_BATTMIN) batt_min = MAX_BATTMIN;
																					draw_double(batt_min, 76, 60, 1, "V", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 4:	
																				//decrease batt max
																				batt_max-=0.1;
																				if(batt_max < MIN_BATTMAX) batt_max = MAX_BATTMAX;
																				draw_double(batt_max, 76, 80, 1, "V", ERR);
																				delay_ms(300); 
																				while(keyhit()==KEY_LEFT_2)
																				{
																					batt_max-=0.1;
																					if(batt_max < MIN_BATTMAX) batt_max = MAX_BATTMAX;
																					draw_double(batt_max, 76, 80, 1, "V", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 5:	
																				//decrease critical batt
																				critical_batt--;
																				if(critical_batt < 1) critical_batt = 100;
																				draw_int(critical_batt, 85, 100, "%", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					critical_batt--;
																					if(critical_batt < 1) critical_batt = 100;
																					draw_int(critical_batt, 85, 100, "%", ERR);
																					delay_ms(50);
																				}
																				break;
																		default:
																				break;
																	}
																	options_changed = true;
																	break;
																
													case KEY_LEFT_1:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	active_page = 3;
																	paint_options(active_page);
																	active_option = 1;
																	active_value = 0;
																	paint_opt_values_page3(device_pos, auto_fill_enabled, he_min, fill_timeout);
																	break;
																	
													case KEY_UP:		
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 1:	
																				active_value=0;
																				LCD_Print(STRING_RES_MIN, 2, 20, 2, 1,1, ERR, BGC);
																				draw_double(res_min, 76, 20, 1, "o", FGC);
																				break;
																		case 2: 
																				active_value=0;
																				LCD_Print(STRING_RES_MAX, 2, 40, 2, 1,1, ERR, BGC);
																				draw_double(res_max, 76, 40, 1, "o", FGC);
																				break;
																		case 3: 
																				active_value=0;
																				LCD_Print(STRING_BATTMIN, 2, 60, 2, 1,1, ERR, BGC);
																				draw_double(batt_min, 76, 60, 1, "V", FGC);
																				break;
																		case 4: 
																				active_value=0;
																				LCD_Print(STRING_BATTMAX, 2, 80, 2, 1,1, ERR, BGC);
																				draw_double(batt_max, 76, 80, 1, "V", FGC);
																				break;
																		case 5: 
																				active_value=0;
																				LCD_Print(STRING_CRITVOLT, 2, 100, 2, 1,1, ERR, BGC);
																				draw_int(critical_batt, 85, 100, "%", FGC);
																				break;
																	}
																	break;
																	
													case KEY_DOWN:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	if (!active_value) {
																		switch(active_option) 
																		{
																			case 1:	
																					active_value=1;
																					LCD_Print(STRING_RES_MIN, 2, 20, 2, 1,1, FGC, BGC);
																					draw_double(res_min, 76, 20, 1, "o", ERR);
																					break;
																			case 2: 
																					active_value=2;
																					LCD_Print(STRING_RES_MAX, 2, 40, 2, 1,1, FGC, BGC);
																					draw_double(res_max, 76, 40, 1, "o", ERR);
																					break;
																			case 3: 
																					active_value=3;
																					LCD_Print(STRING_BATTMIN, 2, 60, 2, 1,1, FGC, BGC);
																					draw_double(batt_min, 76, 60, 1, "V", ERR);
																					break;
																			case 4: 
																					active_value=4;
																					LCD_Print(STRING_BATTMAX, 2, 80, 2, 1,1, FGC, BGC);
																					draw_double(batt_max, 76, 80, 1, "V", ERR);
																					break;
																			case 5: 
																					active_value=5;
																					LCD_Print(STRING_CRITVOLT, 2, 100, 2, 1,1, FGC, BGC);
																					draw_int(critical_batt, 85, 100, "%", ERR);
																					break;
																		}
																	}
																	break;
																	
													case KEY_RIGHT_2:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 0: 
																				active_option--;
																				if(!active_option) active_option = 5;	
																				
																				paint_current_opt_page2(active_option, KEY_RIGHT_2);
																				break;
																		case 1:	
																				//increase r min
																				res_min+=0.1;
																				if(res_min > MAX_RES_MIN) res_min = MIN_RES_MIN;
																				draw_double(res_min, 76, 20, 1, "o", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					res_min+=0.1;
																					if(res_min > MAX_RES_MIN) res_min = MIN_RES_MIN;
																					draw_double(res_min, 76, 20, 1, "o", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 2:	
																				//increase r max
																				res_max+=0.1;
																				if(res_max > MAX_RES_MAX) res_max = MIN_RES_MAX;
																				draw_double(res_max, 76, 40, 1, "o", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					res_max++;
																					if(res_max > MAX_RES_MAX) res_max = MIN_RES_MAX;
																					draw_double(res_max, 76, 40, 1, "o", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 3:	
																				//increase batt min
																				batt_min+=0.1;
																				if(batt_min > MAX_BATTMIN) batt_min = MIN_BATTMIN;
																				draw_double(batt_min, 76, 60, 1, "V", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					batt_min+=0.1;
																					if(batt_min > MAX_BATTMIN) batt_min = MIN_BATTMIN;
																					draw_double(batt_min, 76, 60, 1, "V", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 4:	
																				//increase batt max
																				batt_max+=0.1;
																				if(batt_max > MAX_BATTMAX) batt_max = MIN_BATTMAX;
																				draw_double(batt_max, 76, 80, 1, "V", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					batt_max+=0.1;
																					if(batt_max > MAX_BATTMAX) batt_max = MIN_BATTMAX;
																					draw_double(batt_max, 76, 80, 1, "V", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 5:	
																				//increase critical batt
																				critical_batt++;
																				if(critical_batt > 99) critical_batt = 1;
																				draw_int(critical_batt, 85, 100, "%", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					critical_batt++;
																					if(critical_batt > 99) critical_batt = 1;
																					draw_int(critical_batt, 85, 100, "%", ERR);
																					delay_ms(50);
																				}
																				break;	
																		default:
																				break;
																	}
																	options_changed = true;
																	break;
																	
													case KEY_RIGHT_1:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	active_page = 1;
																	paint_options(active_page);
																	active_option = 1;
																	active_value = 0;
																	paint_opt_values_page1(transmit_slow_min, transmit_slow, transmit_fast, heat_time, meas_cycles, fill_timeout);
																	break;
													default:			
																	break;
												}
												break;												
										case 3://autofill
												switch(keyhit())
												{
													case KEY_LEFT_2:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value)
																	{
																		case 0:
																				active_option++;
																				if(active_option == 5) active_option = 1;
																				
																				paint_current_opt_page3(active_option, KEY_LEFT_2);
																				break;
																		case 1:
																				//pos verändern (wird sofort mit down (->) ausgeführt)
																				break;
																		case 2:
																				auto_fill_enabled = (auto_fill_enabled)?  false : true;
																				(auto_fill_enabled)?
																					LCD_Print("on ", 85, 40, 2, 1,1, ERR, BGC)
																				:	LCD_Print("off", 85, 40, 2, 1,1, ERR, BGC);
																				break;
																		case 3:
																				//decrease autofill value
																				he_min--;
																				if(he_min < MIN_AUTO_FILL_HE) he_min = MAX_AUTO_FILL_HE;
																				draw_int(he_min, 85, 60, "%", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					he_min--;
																					if(he_min < MIN_AUTO_FILL_HE) he_min = MAX_AUTO_FILL_HE;
																					draw_int(he_min, 85, 60, "%", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 4:
																				fill_timeout--;
																				if(fill_timeout < MIN_FILLING_TIMEOUT) fill_timeout = MAX_FILLING_TIMEOUT;
																				draw_int(fill_timeout, 85, 80, "min", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					fill_timeout--;
																					if(fill_timeout < MIN_FILLING_TIMEOUT) fill_timeout = MAX_FILLING_TIMEOUT;
																					draw_int(fill_timeout, 85, 80, "min", ERR);
																					delay_ms(50);
																				}
																				break;
																	}
																	break;
													case KEY_LEFT_1:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	active_page = 4;
																	paint_options(active_page);
																	active_option = 1;
																	active_value = 0;
																	
																	paint_opt_values_page4(span, zero, enable_pressure);
																	break;
																	
													case KEY_UP:		
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 1:	
																				active_value=0;
																				LCD_Print(STRING_POS, 2, 20, 2, 1,1, ERR, BGC);
																				LCD_Print(device_pos, 85, 20, 2, 1,1, FGC, BGC);
																				break;
																		case 2: 
																				active_value=0;
																				LCD_Print(STRING_AUTOFILL, 2, 40, 2, 1,1, ERR, BGC);
																				
																				(auto_fill_enabled)?
																					LCD_Print("on ", 85, 40, 2, 1,1, FGC, BGC)
																				:	LCD_Print("off", 85, 40, 2, 1,1, FGC, BGC);
																				break;
																		case 3:
																				active_value=0;
																				LCD_Print(STRING_HE_MIN_LVL, 2, 60, 2, 1,1, ERR, BGC);
																				draw_int(he_min, 85, 60, "%", FGC);
																				break;
																		case 4:
																				active_value=0;
																				LCD_Print(STRING_FILLING_TIMEOUT, 2, 80, 2, 1,1, ERR, BGC);
																				draw_int(fill_timeout, 85, 80, "min", FGC);
																				break;
																	}
																	break;
																	
													case KEY_DOWN:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	if(!active_value)
																	{
																		switch(active_option) 
																		{
																			case 1:	
																					//position bytes are in temp
																					if(update_filling_pos(fill_pos_letters, fill_pos_ranges, temp)) strcpy(device_pos, temp);
																					active_value=0;
																					
																					paint_options(active_page);
																					paint_opt_values_page3(device_pos, auto_fill_enabled, he_min, fill_timeout);
																					break;
																			case 2: 
																					active_value=2;
																					LCD_Print(STRING_AUTOFILL, 2, 40, 2, 1,1, FGC, BGC);
																					
																					(auto_fill_enabled)?
																						LCD_Print("on ", 85, 40, 2, 1,1, ERR, BGC)
																					:	LCD_Print("off", 85, 40, 2, 1,1, ERR, BGC);
																					break;
																			case 3:
																					active_value=3;
																					LCD_Print(STRING_HE_MIN_LVL, 2, 60, 2, 1,1, FGC, BGC);
																					draw_int(he_min, 85, 60, "%", ERR);
																					break;
																			case 4:
																					active_value=4;
																					LCD_Print(STRING_FILLING_TIMEOUT, 2, 80, 2, 1,1, FGC, BGC);
																					draw_int(fill_timeout, 85, 80, "min", ERR);
																					break;
																		}
																	}
																	break;
																	
													case KEY_RIGHT_2:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value)
																	{
																		case 0:
																				active_option--;
																				if(!active_option) active_option = 4;
																				paint_current_opt_page3(active_option, KEY_RIGHT_2);
																				break;
																		case 1:
																				//pos verändern
																				break;
																		case 2:
																				auto_fill_enabled = (auto_fill_enabled)?  false : true;
																				(auto_fill_enabled)?
																					LCD_Print("on ", 85, 40, 2, 1,1, ERR, BGC)
																				:	LCD_Print("off", 85, 40, 2, 1,1, ERR, BGC);
																				break;
																		case 3:
																				//increase autofill value
																				he_min++;
																				if(he_min > MAX_AUTO_FILL_HE) he_min = MIN_AUTO_FILL_HE;
																				draw_int(he_min, 85, 60, "%", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					he_min++;
																					if(he_min > MAX_AUTO_FILL_HE) he_min = MIN_AUTO_FILL_HE;
																					draw_int(he_min, 85, 60, "%", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 4:
																				fill_timeout++;
																				if(fill_timeout > MAX_FILLING_TIMEOUT) fill_timeout = MIN_FILLING_TIMEOUT;
																				draw_int(fill_timeout, 85, 80, "min", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					fill_timeout++;
																					if(fill_timeout > MAX_FILLING_TIMEOUT) fill_timeout = MIN_FILLING_TIMEOUT;
																					draw_int(fill_timeout, 85, 80, "min", ERR);
																					delay_ms(50);
																				}
																				break;
																	}
																	options_changed = true;
																	break;
																	
													case KEY_RIGHT_1:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	active_page = 2;
																	paint_options(active_page);
																	active_option = 1;
																	active_value = 0;
																	
																	paint_opt_values_page2(res_min, res_max, batt_min, batt_max, critical_batt);
																	break;	
												}
												break;
										case 4://pressure
												switch(keyhit())
												{
													case KEY_LEFT_2:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value)
																	{
																		case 0:
																				active_option++;
																				if(active_option == 4) active_option = 1;
																				paint_current_opt_page4(active_option, KEY_LEFT_2);
																				break;
																		case 1:	
																				//decrease span
																				span-=0.1;
																				if(span < MIN_SPAN) span = MIN_SPAN;
																				draw_double(span, 76, 20, 1, "", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					span-=1;
																					if(span < MIN_SPAN) span = MIN_SPAN;
																					draw_double(span, 76, 20, 1, "", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 2:	
																				//decrease zero
																				zero-=0.1;
																				if(zero < MIN_ZERO) zero = MIN_ZERO;
																				draw_double(zero, 76, 40, 1, "", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					zero-=1;
																					if(zero < MIN_ZERO) zero = MIN_ZERO;
																					draw_double(zero, 76, 40, 1, "", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 3:	
																				//decrease batt min
																				enable_pressure =(enable_pressure)? false : true;
																				(enable_pressure)?
																					LCD_Print("on ", 85, 60, 2, 1,1, ERR, BGC)
																				:	LCD_Print("off", 85, 60, 2, 1,1, ERR, BGC);
																				break;
																	}
																	break;
													case KEY_LEFT_1:
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	active_page = 5;
																	paint_options(active_page);
																	active_option = 1;
																	active_value = 0;
																	
																	paint_opt_values_page5(r_span, r_zero);
																	break;
													case KEY_UP:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 1:	
																				active_value=0;
																				LCD_Print(STRING_SPAN, 2, 20, 2, 1,1, ERR, BGC);
																				draw_double(span, 76, 20, 1, "", FGC);
																				break;
																		case 2: 
																				active_value=0;
																				LCD_Print(STRING_ZERO, 2, 40, 2, 1,1, ERR, BGC);
																				draw_double(zero, 76, 40, 1, "", FGC);
																				break;
																		case 3: 
																				active_value=0;
																				(enable_pressure)?
																					LCD_Print("on ", 85, 60, 2, 1,1, FGC, BGC)
																				:	LCD_Print("off", 85, 60, 2, 1,1, FGC, BGC);
																				break;
																	}
																	break;
													case KEY_DOWN:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_option) 
																	{
																		case 1:	
																				active_value=1;
																				LCD_Print(STRING_SPAN, 2, 20, 2, 1,1, FGC, BGC);
																				draw_double(span, 76, 20, 1, "", ERR);
																				break;
																		case 2: 
																				active_value=2;
																				LCD_Print(STRING_ZERO, 2, 40, 2, 1,1, FGC, BGC);
																				draw_double(zero, 76, 40, 1, "", ERR);
																				break;
																		case 3: 
																				active_value=3;
																				LCD_Print(STRING_ENABLE_PR, 2, 60, 2, 1,1, FGC, BGC);
																				(enable_pressure)?
																					LCD_Print("on ", 85, 60, 2, 1,1, ERR, BGC)
																				:	LCD_Print("off", 85, 60, 2, 1,1, ERR, BGC);
																				break;
																	}
																	break;
													case KEY_RIGHT_2:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value)
																	{
																		case 0:
																				active_option--;
																				if(!active_option) active_option = 3;
																				paint_current_opt_page4(active_option, KEY_RIGHT_2);
																				break;
																		case 1:	
																				//increase span
																				span+=0.1;
																				if(span > MAX_SPAN) span = MAX_SPAN;
																				draw_double(span, 76, 20, 1, "", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					span+=1;
																					if(span > MAX_SPAN) span = MAX_SPAN;
																					draw_double(span, 76, 20, 1, "", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 2:	
																				//increase zero
																				zero+=0.1;
																				if(zero > MAX_ZERO) zero = MAX_ZERO;
																				draw_double(zero, 76, 40, 1, "", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					zero+=1;
																					//if(zero > MAX_ZERO) zero = MAX_ZERO;
																					draw_double(zero, 76, 40, 1, "", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 3:	
																				enable_pressure =(enable_pressure)? false : true;
																				(enable_pressure)?
																					LCD_Print("on ", 85, 60, 2, 1,1, ERR, BGC)
																				:	LCD_Print("off", 85, 60, 2, 1,1, ERR, BGC);
																				break;
																	}
																	options_changed = true;
																	break;
																		
													case KEY_RIGHT_1:
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	active_page = 3;
																	paint_options(active_page);
																	active_option = 1;
																	active_value = 0;
																	
																	paint_opt_values_page3(device_pos, auto_fill_enabled, he_min, fill_timeout);
																	break;
													default: 		
																	break;
												}	
												break;
										case 5://diag/shutdown
												switch(keyhit())
												{
													case KEY_LEFT_2:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 0: 
																				active_option++;
																				if(active_option == 5) active_option = 1;	
																				paint_current_opt_page5(active_option, KEY_LEFT_2);
																				break;
																		case 1:	
																				if(LCD_Dialog(STRING_SHUTDOWN,"Do you really\nwant to shut down\nthe system?", D_FGC, D_BGC))
																				{
																					LCD_Cls(BGC);
																					mode = MODE_SHUTDOWN_ACTION;
																				}
																				else {
																					active_page = 5;
																					paint_options(active_page);
																					active_option = 1;
																					active_value = 0;
																					paint_opt_values_page5(r_span, r_zero);
																				}
																				break;
																		case 2: 
																				paint_diag(1);
																				execute_pressed_key=0;pressed_key=0;
																				mode = MODE_DIAG_ACTION;
																				break;
																		case 3: 
																				r_span-=0.1;
																				//if(r_span < 0) r_span = 2;
																				draw_double(r_span, 76, 60, 1, "", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					r_span-=0.1;
																					//if(r_span < 0) r_span = 2;
																					draw_double(r_span, 76, 60, 1, "", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 4: 
																				r_zero-=0.1;
																				//if(r_span < 0) r_span = 2;
																				draw_double(r_zero, 76, 80, 1, "", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_LEFT_2)
																				{
																					r_zero-=0.1;
																					//if(r_span < 0) r_span = 2;
																					draw_double(r_zero, 76, 80, 1, "", ERR);
																					delay_ms(50);
																				}
																				break;
																	}
																	break;
																
													case KEY_LEFT_1:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	if(err_code != OFFLINE)
																	{
																		if(options_changed)
																		{
																			xbee_wake_up();
																			buffer[0] = device_id>>8;
																			buffer[1] = device_id;
																			
																			if(transmit_slow_min) 
																			{
																				buffer[2] = transmit_slow>>8;
																				buffer[3] = transmit_slow;
																			}
																			else {
																				buffer[2] = (transmit_slow*60)>>8;
																				buffer[3] = transmit_slow*60;
																			}
																			if(transmit_fast_sec) 
																			{
																				buffer[4] = transmit_fast>>8;
																				buffer[5] = transmit_fast;
																			}
																			else {
																				buffer[4] = (transmit_fast*60)>>8;
																				buffer[5] = transmit_fast*60;
																			}
																			buffer[6] = ((uint16_t)(res_min*10))>>8;
																			buffer[7] = res_min*10;
																			buffer[8] = ((uint16_t)(res_max*10))>>8;
																			buffer[9] = res_max*10;
																			buffer[10] = ((uint16_t)(heat_time*1000))>>8;
																			buffer[11] = (heat_time*1000);
																			buffer[12] = meas_cycles;
																			buffer[13] = fill_timeout;
																			buffer[14] = ((uint16_t)(span*10))>>8;
																			buffer[15] = span*10;
																			buffer[16] = ((uint16_t)((ZERO_NULLPUNKT+zero)*10))>>8;
																			buffer[17] = (ZERO_NULLPUNKT+zero)*10;
																			buffer[18] = ((uint16_t)(batt_min*10))>>8;
																			buffer[19] = batt_min*10;
																			buffer[20] = ((uint16_t)(batt_max*10))>>8;
																			buffer[21] = batt_max*10;
																			buffer[22] = critical_batt;
																			buffer[23] = status_byte;
																			
																			LCD_Cls(BGC);
																			LCD_Print("saving settings...", 5, 50, 2, 1, 1, FGC, BGC);
																			#ifdef ALLOW_COM
																				send_request(OPTIONS_CHANGED_MSG, buffer, 24, &dest_high, &dest_low);
																			#else
																				sending_cmd = pack_tx64_frame(OPTIONS_CHANGED_MSG, buffer, 23, dest_high, dest_low);
																				basic_send(buffer);
																			#endif
																			xbee_sleep();
																			
																			#ifdef ALLOW_EEPROM_SAVING
																				eeprom_write_word(&ee_r_zero, (uint16_t)(r_zero*10));
																				eeprom_write_word(&ee_r_span, (uint16_t)(r_span*10));
																				
																				eeprom_write_word(&ee_transmit_slow, transmit_slow);
																				eeprom_write_byte(&ee_transmit_slow_min, transmit_slow_min);
																				eeprom_write_word(&ee_transmit_fast, transmit_fast);
																				eeprom_write_byte(&ee_transmit_fast_sec, transmit_fast_sec);
																				eeprom_write_word(&ee_heat_time, (uint16_t) (heat_time*1000));							
																				
																				eeprom_write_byte(&ee_meas_cycles, meas_cycles);
																				eeprom_write_byte(&ee_fill_timeout, fill_timeout);
																				eeprom_write_byte(&ee_he_min, he_min);
																				eeprom_write_word(&ee_res_min, (uint16_t) (res_min*10));
																				eeprom_write_word(&ee_res_max, (uint16_t) (res_max*10));
																				eeprom_write_word(&ee_span, (uint16_t) (span*10));
																				eeprom_write_word(&ee_zero, (uint16_t) (zero*10));
																				eeprom_write_byte(&ee_enable_pressure, enable_pressure);
																				eeprom_write_word(&ee_batt_min, (uint16_t) (batt_min*10));
																				eeprom_write_word(&ee_batt_max, (uint16_t) (batt_max*10));
																				eeprom_write_byte(&ee_critical_batt, critical_batt);
																			#endif
																		}
																		paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
																		mode = MODE_MAIN_WINDOW;
																		options_changed = false;
																	}
																	else {
																		mode = MODE_OFFLINE;
																		paint_offline(device_id,device_pos,he_level,batt_level,critical_batt,0);
																	}
																	break;
																	
													case KEY_UP:		
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 1:	
																				active_value=0;
																				LCD_Print(STRING_SHUTDOWN, 2, 20, 2, 1,1, ERR, BGC);
																				LCD_Print("off", 85, 20, 2, 1,1, FGC, BGC);
																				break;
																		case 2: 
																				active_value=0;
																				LCD_Print(STRING_DIAG, 2, 40, 2, 1,1, ERR, BGC);
																				LCD_Print("off", 85, 40, 2, 1,1, FGC, BGC);
																				break;
																		case 3: 
																				active_value=0;
																				LCD_Print(STRING_ADCSPAN, 2, 60, 2, 1,1, ERR, BGC);
																				draw_double(r_span, 76, 60, 1, "", FGC);
																				break;		
																		case 4: 
																				active_value=0;
																				LCD_Print(STRING_ADCZERO, 2, 80, 2, 1,1, ERR, BGC);
																				draw_double(r_zero, 76, 80, 1, "", FGC);
																				break;
																	}
																	break;
																	
													case KEY_DOWN:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	if (!active_value) {
																		switch(active_option) 
																		{
																			case 1:	
																					active_value=1;
																					LCD_Print(STRING_SHUTDOWN, 2, 20, 2, 1,1, FGC, BGC);
																					LCD_Print("off", 85, 20, 2, 1,1, ERR, BGC);
																					break;
																			case 2: 
																					active_value=2;
																					LCD_Print(STRING_DIAG, 2, 40, 2, 1,1, FGC, BGC);
																					LCD_Print("off", 85, 40, 2, 1,1, ERR, BGC);
																					break;
																			case 3: 
																					active_value=3;
																					LCD_Print(STRING_ADCSPAN, 2, 60, 2, 1,1, FGC, BGC);
																					draw_double(r_span, 76, 60, 1, "", ERR);
																					break;
																			case 4: 
																					active_value=4;
																					LCD_Print(STRING_ADCZERO, 2, 80, 2, 1,1, FGC, BGC);
																					draw_double(r_zero, 76, 80, 1, "", ERR);
																					break;
																			}
																	}
																	break;
																	
													case KEY_RIGHT_2:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	switch(active_value) 
																	{
																		case 0:
																				active_option--;
																				if(!active_option) active_option = 4;	
																				paint_current_opt_page5(active_option, KEY_RIGHT_2);
																				break;
																		case 1:	
																				if(LCD_Dialog(STRING_SHUTDOWN,"Do you really\nwant to shut down\n the system?", D_FGC, D_BGC))
																				{
																					LCD_Cls(BGC);
																					mode = MODE_SHUTDOWN_ACTION;
																				}
																				else {
																					active_page = 5;
																					paint_options(active_page);
																					active_option = 1;
																					active_value = 0;
																				}
																				break;
																		case 2: 
																				paint_diag(1);
																				execute_pressed_key=0;pressed_key=0;
																				mode = MODE_DIAG_ACTION;
																				break;
																		case 3: 
																				r_span+=0.1;
																				//if(r_span < 0) r_span = 2;
																				draw_double(r_span, 76, 60, 1, "", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					r_span+=0.1;
																					//if(r_span < 0) r_span = 2;
																					draw_double(r_span, 76, 60, 1, "", ERR);
																					delay_ms(50);
																				}
																				break;
																		case 4: 
																				r_zero+=0.1;
																				//if(r_span < 0) r_span = 2;
																				draw_double(r_zero, 76, 80, 1, "", ERR);
																				delay_ms(300);
																				while(keyhit()==KEY_RIGHT_2)
																				{
																					r_zero+=0.1;
																					//if(r_span < 0) r_span = 2;
																					draw_double(r_zero, 76, 80, 1, "", ERR);
																					delay_ms(50);
																				}
																				break;
																	}
																	options_changed = true;
																	break;
																	
													case KEY_RIGHT_1:	
																	
																	if (!display_on())
																	#ifndef ALLOW_DOUBLE_CLICK 
																		break; 
																	#else
																		;
																	#endif
																	active_page = 4;
																	paint_options(active_page);
																	active_option = 1;
																	active_value = 0;
																	
																	paint_opt_values_page4(span, zero, enable_pressure);
																	break;
													default:		
																	break;
												}
												break;
										default:
												break;
									}
									break;
		case	MODE_SHUTDOWN_ACTION:
									
									MEASURE_PIN_ON
									he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, heat_time, 0);
									batt_level = get_batt_level(batt_min, batt_max, r_zero);
									pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
									MEASURE_PIN_OFF
									
									xbee_wake_up();
									
									
									
									buffer[0] = device_id>>8;
									buffer[1] = device_id;
									
									if(he_level < 0)  //dont send err HE (-1, -2, -3 ..), send some code instead
									{
										buffer[2] = ERROR_HE_LEVEL>>8;
										buffer[3] = (uint8_t) ERROR_HE_LEVEL;
									}
									else {
										buffer[2] = ((uint16_t)(he_level*10))>>8;
										buffer[3] = ((uint16_t)(he_level*10));
									}
									
									buffer[4] = batt_level;
									buffer[5] = pressure_level >> 8;
									buffer[6] = pressure_level;
									buffer[7] = status_byte;
									
									#ifdef ALLOW_COM
										send_request(LOGOUT_MSG, buffer, 8, &dest_high, &dest_low);
									#else
										sending_cmd = pack_tx64_frame(LOGOUT_MSG, buffer, 8, dest_high, dest_low);
										basic_send(buffer);
									#endif
									//xbee_sleep();  all ends anyway
									SHUTDOWN //set shutdown pin
									//PROGRAM ENDS HERE
									//if pin doesn't work go to mysterious error
									mode=MODE_ERROR;
									err_code=42;
									break;
		case	MODE_DIAG_ACTION:
									switch(pressed_key)//switch(keyhit())
									{
													
										case KEY_LEFT_2:	
														execute_pressed_key = 0;
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														if(diag_to_show == 1)
														{
															if(CHECK_MEASURE_PIN)
															{
																MEASURE_PIN_OFF 
																LCD_Print("OFF", 40, 100, 2, 1,1, ERR, BGC);
															}
															else {
																MEASURE_PIN_ON
																LCD_Print("ON ", 40, 100, 2, 1,1, ERR, BGC);
															}
														}
														delay_ms(250);
														break;
													
										case KEY_LEFT_1:	
														execute_pressed_key = 0;
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														diag_pulse(&heat_time, r_span,r_zero);
														paint_diag(diag_to_show);
														options_changed = true;
														break;
														
										case KEY_UP:		
														execute_pressed_key = 0;
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														diag_to_show++;
														if(diag_to_show == 3) {diag_to_show = 1;}
														paint_diag(diag_to_show);
														//delay_ms(250);
														break;
														
										case KEY_DOWN:	
														execute_pressed_key = 0;
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														diag_to_show--;
														if(diag_to_show == 0) {diag_to_show = 2;}
														paint_diag(diag_to_show);
														//delay_ms(250);
														break;
														
														
										case KEY_RIGHT_2:	
														execute_pressed_key = 0;
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														mode = MODE_CALIBRATION;
														//delay_ms(300);
														break;
														
														
										case KEY_RIGHT_1:	
														execute_pressed_key = 0;
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														//if(LCD_Dialog("Diagnostic","Do you really\nwant to stop\ndiagnostic procedure?", D_FGC, D_BGC))
														//{
															if(CHECK_MEASURE_PIN) MEASURE_PIN_OFF
															mode = MODE_OPTIONS_WINDOW;
															active_page = 5;
															paint_options(active_page);
															active_option = 1;
															active_value = 0;
															diag_to_show = 1;
															paint_opt_values_page5(r_span, r_zero);
														//}
														//else paint_diag(diag_to_show);
														break;
										default:		
														switch(diag_to_show)
														{
															case 1:	
																	diag_page1(r_zero, r_span, batt_min, batt_max, res_min, res_max, zero, span);
																	//delay_ms(150);
																	break;
															case 2:
																	diag_page2(r_zero);
																	//delay_ms(150);
																	break;
															default:
																	break;
														}
														//delay_ms(100);
														break;
									}
									break;
		case	MODE_OFFLINE:
									switch(keyhit())
									{
										case KEY_LEFT_2:	
														
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														//delay_ms(300);
														break;
													
										case KEY_RIGHT_2:	
														
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														//delay_ms(300);
														break;
														
										case KEY_UP:		
														
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														//delay_ms(300);
														break;
										case KEY_DOWN:		
														
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														//delay_ms(300);
														break;				
										case KEY_RIGHT_1:	
														
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														paint_measure(device_id, device_pos, he_level, batt_level, critical_batt, 0);
														
														MEASURE_PIN_ON
														he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, heat_time, 1);
														batt_level = get_batt_level(batt_min, batt_max, r_zero);
														pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
														MEASURE_PIN_OFF
														
														//paint_measure(device_id, device_pos, he_level, batt_level, 1);
														if(force_measurement)
														{
															did_measurement = true;
														}
														else if(err_code == OFFLINE)
														{
															paint_offline(device_id,device_pos,he_level,batt_level,critical_batt,0);
															mode = MODE_OFFLINE;
														}
														else {
															paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
															mode = MODE_MAIN_WINDOW;
														}
														break;
										case KEY_LEFT_1:	
														
														if (!display_on())
														#ifndef ALLOW_DOUBLE_CLICK 
															break; 
														#else
															;
														#endif
														
														uint8_t status;
														entered_options_pw = get_number(&status, PASSWORD);
														
														if(status || (options_pw != entered_options_pw))
														{
															paint_offline(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
															break;	//if not 0 -> no options
														}
														
														mode = MODE_OPTIONS_WINDOW;
														active_option = 1;
														active_value = 0;
														active_page = 1;
														paint_options(active_page);
														//show current option values
														paint_opt_values_page1(transmit_slow_min, transmit_slow, transmit_fast, heat_time, meas_cycles, fill_timeout);
														break;
										default:
														break;
									}
									break;
		case	MODE_ERROR:			
									LCD_Cls(BGC);
									switch(err_code)
									{
										case LOGIN_FAILED:	
														LCD_Print("Connection", 5, 40, 2, 1, 1, FGC, BGC);
														LCD_Print("to server failed", 5, 60, 2, 1, 1, FGC, BGC);
														LCD_Print("press reset to try again", 2, 90, 1, 1, 1, FGC, BGC);
														while(1);
														//no login, no game
										default:		
														//LCD_Print("unknown error", 5, 50, 2, 1, 1, FGC, BGC);
														//LCD_Print("(probably hardware)", 5, 80, 1, 1, 1, FGC, BGC);

														LCD_Print("unknown error", 5, 40, 2, 1, 1, FGC, BGC);
														LCD_Print("(probably hardware)", 5, 60, 2, 1, 1, FGC, BGC);
														uint8_t bufferStr[20];
														sprintf(bufferStr, "code = %u", err_code);
														LCD_Print(bufferStr, 2, 90, 1, 1, 1, FGC, BGC);
														//mysterious error
														break;
									}
									break;									
		case	MODE_CALIBRATION:			
									{
										LCD_Cls(BGC);
										if(LCD_Dialog("Calibration", "Set current span/\nzero to default?", D_FGC, D_BGC))
										{
											//yes
											r_span = R_SPAN_DEF;
											r_zero = R_ZERO_DEF;
										}
										if(!CHECK_MEASURE_PIN) MEASURE_PIN_ON
										if(!LCD_Dialog("Calibration", "Set Res to\n0 ohm", D_FGC, D_BGC))
										{
											delay_ms(300);
											execute_pressed_key=0;pressed_key=0;	//sonst key sofort im diag aktiv
											MEASURE_PIN_OFF
											break;
										}
										
										temp_r_zero = ((10.0/r_span)*(r_zero + (readChannel(CURRENT, ADC_LOOPS)))) - readChannel(VOLTAGE, ADC_LOOPS);
										
										LCD_Dialog("Calibration", "Set Res to\n340 ohm", D_FGC, D_BGC);
										
										uint16_t adc_v = readChannel(VOLTAGE, ADC_LOOPS);
										uint16_t adc_i = readChannel(CURRENT, ADC_LOOPS);
										double temp_double1 = (double)(adc_v+temp_r_zero)/(adc_i+temp_r_zero);
										temp_r_span = (350)/(temp_double1);
										
										MEASURE_PIN_OFF
										
										sprintf(temp,"Span: %.5f\nZero: %.5f\nsave?", temp_r_span, temp_r_zero);
										if(LCD_Dialog("Calibration", temp, D_FGC, D_BGC))
										{
											r_span = temp_r_span;
											r_zero = temp_r_zero;
											options_changed = true;
										}
										paint_diag(diag_to_show);
										execute_pressed_key=0;pressed_key=0;	//sonst key sofort im diag aktiv
										mode = MODE_DIAG_ACTION;
									}
									break;
		default:					
									//should never happen
									LCD_Print("should never happen", 5, 50, 1, 1, 1, FGC, BGC);
									break;
		}
		//=========================================================================
		//entprellen..etwas 
		//=========================================================================
		//if(!execute_pressed_key)	pressed_key=0;
		if(mode != MODE_DIAG_ACTION)
		{//wait till user stops presssing a key	
			while(!(keyhit() == 0));
			delay_ms(10);
		}
		if(!execute_pressed_key)	pressed_key=0;	//reset saved key (PCI_int)
		//=========================================================================
		//xbee sleep cycle 
		//=========================================================================
		if(!xbee_busy && (err_code != OFFLINE))	//no sleeping while filling + not offline
		{
			if(xbee_sleeping)
			{
				if(!set_timeout(0, TIMER_5, USE_TIMER))		//timer5 not running and returns 0
				{
					xbee_wake_up();
					
					

					
					//notify server
					buffer[0] = device_id>>8;
					buffer[1] = device_id;
					buffer[2] = status_byte;
					
					//no ack, no blocking
					reconnect(&dest_low, &dest_high);
					sending_cmd = pack_tx64_frame(XBEE_ACTIVE_MSG, buffer, 3, dest_high, dest_low);
					basic_send(buffer);
					
					
					set_timeout(0, TIMER_5, RESET_TIMER);
					set_timeout(XBEE_AWAKE_TIME, TIMER_5, USE_TIMER);		//stay active for 30 sec
					xbee_sleeping = false;
					#ifdef ALLOW_DEBUG
						LCD_Print("awake", 5, 20, 2, 1, 1, FGC, BGC);
					#endif
				}
			}	
			else {//active
				if(!set_timeout(0, TIMER_5, USE_TIMER))		//timer5 not running and returns 0
				{	
					xbee_sleep();
					set_timeout(0, TIMER_5, RESET_TIMER);
					set_timeout(xbee_sleep_period*60, TIMER_5, USE_TIMER);
					xbee_sleeping = true;
					#ifdef ALLOW_DEBUG
						LCD_Print("sleep", 5, 20, 2, 1, 1, FGC, BGC);
					#endif
				}
			}
		}
		
		//=========================================================================
		//FORCED MEASUREMENT (BUTTON PRESSED)
		//=========================================================================

		if(DO_MEASUREMENT && (mode==MODE_MAIN_WINDOW || mode==MODE_FILL_ACTION))
		
		{
			DISPLAY_TURN_ON;
			force_measurement = true;
			temp_mode = mode;
			mode = MODE_MEASURE_ACTION;
			//paint_measure(device_id, device_pos, he_level, batt_level,critical_batt, 0);
			
				
		}
		else if (did_measurement)
			 {
				DISPLAY_TURN_ON;
				mode = temp_mode;
				//paint screen according to mode
				switch(mode)
				{
					case	MODE_MAIN_WINDOW:	
												paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
												break;
					case	MODE_FILL_ACTION:	
												paint_filling(device_id, device_pos, he_level, batt_level,critical_batt, 0);
												//sprintf(temp,"waiting %dmin", transmit_fast);
												LCD_Print("waiting  ", 2, 20, 2, 1, 1, FGC, BGC);
												sprintf(temp,FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
												LCD_Print(temp, 5, 105, 1, 1, 1, FGC, BGC);
												break;
					default:				
												break;
				}//switch
				did_measurement = false;
				force_measurement = false;
			 }//if (did_measurement)
			 
		
		
		//=========================================================================
		//DISPLAY TIMER
		//=========================================================================
		#ifdef ALLOW_DISPLAY_TIMER
			if(!set_timeout(0, TIMER_2, USE_TIMER)) 
			{
				if((mode==MODE_MAIN_WINDOW) || (mode==MODE_OFFLINE))	DISPLAY_TURN_OFF
				#ifdef ALLOW_DEBUG
					LCD_Print("off", 5, 20, 2, 1, 1, FGC, BGC);
				#endif	
			}
		#endif
		
		//=========================================================================
		//INTERVAL SLOW changed
		//=========================================================================
		if(transmit_slow_changed && !options_changed)
		{	//reset timer
			set_timeout(0, TIMER_1, RESET_TIMER);
			
			//set new timer value
			(transmit_slow_min)? 
				set_timeout(ceil(transmit_slow*60), TIMER_1, USE_TIMER)
			:	set_timeout(transmit_slow*3600, TIMER_1, USE_TIMER);
			
			transmit_slow_changed = false;
		}
		
		//=========================================================================
		//AUTO FILLING
		//=========================================================================
		if((mode == MODE_MAIN_WINDOW) && auto_fill_pin_on() && auto_fill_enabled)
		{//auto fill	
			//Kanne zu leer, back to main, no filling
			if(no_he_level)
			{
				LCD_Dialog("HE LEVEL", "Filling is disabled.\nHe Level is too low.", D_FGC, D_BGC);
				mode = MODE_MAIN_WINDOW;
				paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
			}
			else {
				delay_ms(100);
				//ext. Fuellen nicht mehr an?
				if(!auto_fill_pin_on())
				{
					mode = MODE_MAIN_WINDOW;
					paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
				}
				else {
					LCD_Cls(BGC);
					LCD_Print("auto fill init.", 5, 40, 2, 1, 1, FGC, BGC);
					LCD_Print("sending data...", 5, 60, 2, 1, 1, FGC, BGC);
					
					xbee_wake_up();
					xbee_busy = true;
					
					uint8_t indx = 0;
					buffer[0] = device_id>>8;
					buffer[1] = device_id;
					indx = pos_to_binary(device_pos, 2, buffer);
					buffer[indx++] = status_byte;
					
					//send
					#ifdef ALLOW_COM
						send_request(FILLING_BEGIN_MSG, buffer, indx, &dest_high, &dest_low);
					#else
						sending_cmd = pack_tx64_frame(FILLING_BEGIN_MSG, buffer, indx, dest_high, dest_low);
						basic_send(buffer);
					#endif
					
					paint_filling(device_id, device_pos, he_level, batt_level,critical_batt,0);
					fill_meas_counter = fill_timeout/transmit_fast;
					mode = MODE_FILL_ACTION;
					//set output fill pin
					START_AUTO_FILL
					auto_fill_started = true;
				}
			}
		}
		
		
		//=========================================================================
		//BATTERY CHECK
		//=========================================================================
		if(batt_min >= map_to_batt(readChannel_calib(BATTERY, ADC_LOOPS, r_zero)))
		//if(0)
		{	
			//last measurement
			
			/*MEASURE_PIN_ON
			he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, heat_time, 0);
			MEASURE_PIN_OFF
			sprintf(temp,"The battery is\ncritically low.\nSystem will shut down!\nLast He Level:\n%d%%",((uint8_t) he_level));
			timed_dialog("Shutdown", temp, 10, D_FGC, D_BGC); 
			
			#ifdef ALLOW_DEBUG
				LCD_Cls(BGC);
				LCD_Print("shutdown", 5, 50, 2, 1, 1, FGC, BGC);
				getkey(); 
			#endif
			
		
			SHUTDOWN
		
			mode=MODE_ERROR;
			err_code=42;
			*/
			
			
			//last measurement
			
			MEASURE_PIN_ON
			he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, heat_time, 0);
			batt_level = get_batt_level(batt_min, batt_max, r_zero);
			pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
			MEASURE_PIN_OFF
			
			sprintf(temp,"The battery is\ncritically low (%d%%).\nSystem will shut down!\nLast He Level:\n%d%%",((uint8_t) batt_level),((uint8_t) he_level));
			timed_dialog("Shutting down", temp, 10, D_FGC, D_BGC); 			
			
			xbee_wake_up();
			
			buffer[0] = device_id>>8;
			buffer[1] = device_id;
			
			if(he_level < 0)  //dont send err HE (-1, -2, -3 ..), send some code instead
			{
				buffer[2] = ERROR_HE_LEVEL>>8;
				buffer[3] = (uint8_t) ERROR_HE_LEVEL;
			}
			else {
				buffer[2] = ((uint16_t)(he_level*10))>>8;
				buffer[3] = ((uint16_t)(he_level*10));
			}
			
			buffer[4] = batt_level;
			buffer[5] = pressure_level >> 8;
			buffer[6] = pressure_level;
			buffer[7] = status_byte;
			
			#ifdef ALLOW_COM
				send_request(LOGOUT_MSG, buffer, 8, &dest_high, &dest_low);
			#else
				sending_cmd = pack_tx64_frame(LOGOUT_MSG, buffer, 8, dest_high, dest_low);
				basic_send(buffer);
			#endif
			//xbee_sleep();  all ends anyway
			SHUTDOWN //set shutdown pin
			//PROGRAM ENDS HERE
			//if pin doesn't work go to mysterious error
			mode=MODE_ERROR;
			err_code=42;
			
		}
		
		
		//=========================================================================
		//CMD PROCESSING
		//=========================================================================
		uint8_t reply_Id = hasReply(LAST_NON_CMD_MSG, GREATER_THEN);
		if(reply_Id != 0xFF) 						//always check for cmd in buffer and do 1 at a time
		{
			switch(frameBuffer[reply_Id].type) 
			{
				case STATUS_MSG:
								paint_wait_screen();
								
								buffer[0] = device_id>>8;
								buffer[1] = device_id;
								
								MEASURE_PIN_ON
								he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, heat_time, 0);
								batt_level = get_batt_level(batt_min, batt_max, r_zero);
								pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
								MEASURE_PIN_OFF
								
								if(he_level < 0)  //dont send err HE (-1, -2, -3 ..), send some code instead
								{
									//uint16_t error_he = ERROR_HE_LEVEL;
									buffer[2] = ((uint16_t)(ERROR_HE_LEVEL))>>8;
									buffer[3] = (uint8_t) ERROR_HE_LEVEL;
								}
								else {
									buffer[2] = ((uint16_t)(he_level*10))>>8;
									buffer[3] = ((uint16_t)(he_level*10));
								}
								buffer[4] = batt_level;
								buffer[5] = pressure_level >> 8;
								buffer[6] = pressure_level;
								buffer[7] = status_byte;
								
								sending_cmd = pack_tx64_frame(STATUS_MSG, buffer, 8, dest_high, dest_low);
								basic_send(buffer);
								
								switch(mode)
								{
									case	MODE_MAIN_WINDOW:	//ok	
																paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
																break;
									case	MODE_FILL_ACTION:	//ok
																paint_filling(device_id, device_pos, he_level, batt_level,critical_batt, 0);
																//sprintf(temp,"waiting %dmin", transmit_fast);
																LCD_Print("waiting  ", 2, 20, 2, 1, 1, FGC, BGC);
																sprintf(temp,FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
																LCD_Print(temp, 5, 105, 1, 1, 1, FGC, BGC);
																break;
									case	MODE_OPTIONS_WINDOW://keine werte
																paint_options(active_page);
																active_option = 1;
																active_value = 0;
																switch(active_page)
																{
																	case 1:
																			paint_opt_values_page1(transmit_slow_min, transmit_slow, transmit_fast, heat_time, meas_cycles, fill_timeout);
																			break;
																	case 2:
																			paint_opt_values_page2(res_min, res_max, batt_min, batt_max, critical_batt);
																			break;
																	case 3:
																			paint_opt_values_page3(device_pos, auto_fill_enabled, he_min, fill_timeout);
																			break;
																	case 4:
																			paint_opt_values_page4(span, zero, enable_pressure);
																			break;
																	case 5:		
																			paint_options(5);
																			break;
																	default:
																			LCD_Print("opt def", 5, 5, 2, 1, 1, ERR, BGC);
																			break;
																}
																break;
									case	MODE_DIAG_ACTION:	//ok
																paint_diag(diag_to_show);
																break;
									default:				
																break;
								}//switch
								break;
				case GET_OPTIONS_CMD:	//OP - options request	
								
								buffer[0] = device_id>>8;
								buffer[1] = device_id;
								
								if(transmit_slow_min) 
								{
									buffer[2] = transmit_slow>>8;
									buffer[3] = transmit_slow;
								}
								else {
									buffer[2] = (transmit_slow*60)>>8;
									buffer[3] = transmit_slow*60;
								}
								if(transmit_fast_sec) 
								{
									buffer[4] = transmit_fast>>8;
									buffer[5] = transmit_fast;
								}
								else {
									buffer[4] = (transmit_fast*60)>>8;
									buffer[5] = transmit_fast*60;
								}
								buffer[6] = ((uint16_t)(res_min*10))>>8;
								buffer[7] = res_min*10;
								buffer[8] = ((uint16_t)(res_max*10))>>8;
								buffer[9] = res_max*10;
								buffer[10] = ((uint16_t)(heat_time*1000))>>8;
								buffer[11] = (heat_time*1000);
								buffer[12] = meas_cycles;
								buffer[13] = fill_timeout;
								buffer[14] = ((uint16_t)(span*10))>>8;
								buffer[15] = span*10;
								buffer[16] = ((uint16_t)((ZERO_NULLPUNKT+zero)*10))>>8;
								buffer[17] = (ZERO_NULLPUNKT+zero)*10;
								buffer[18] = ((uint16_t)(batt_min*10))>>8;
								buffer[19] = batt_min*10;
								buffer[20] = ((uint16_t)(batt_max*10))>>8;
								buffer[21] = batt_max*10;
								buffer[22] = critical_batt;
								buffer[23] = status_byte;
								
								sending_cmd = pack_tx64_frame(GET_OPTIONS_CMD, buffer, 24, dest_high, dest_low);
								basic_send(buffer);
								break;
				case SET_OPTIONS_CMD:	
								
								if (NUMBER_OPTIONS_BYTES == frameBuffer[reply_Id].data_len)
								{
									uint16_t temp_transmit_slow = (frameBuffer[reply_Id].data[0]<<8) + frameBuffer[reply_Id].data[1];
									if(transmit_slow != temp_transmit_slow) { transmit_slow_changed = true; transmit_slow = temp_transmit_slow;}
									
									if(transmit_slow <= 0) transmit_slow = TRANSMIT_SLOW_DEF;
									if(transmit_slow > 60)
									{
										transmit_slow /= 60;
										transmit_slow_min = false;
									}
									else transmit_slow_min = true;
									
									transmit_fast = (frameBuffer[reply_Id].data[2]<<8) + frameBuffer[reply_Id].data[3];
									if(transmit_fast <= 0) transmit_fast = TRANSMIT_FAST_DEF;
									if(transmit_fast > 60)
									{
										transmit_fast /= 60;
										transmit_fast_sec = false;
									}
									else transmit_fast_sec = true;
									
									res_min = ((frameBuffer[reply_Id].data[4]<<8) + frameBuffer[reply_Id].data[5])/10.0;
									if(res_min <= 0) res_min = RES_MIN_DEF;
									
									res_max = ((frameBuffer[reply_Id].data[6]<<8) + frameBuffer[reply_Id].data[7])/10.0;
									if(res_max <= 0) res_max = RES_MAX_DEF;
									
									heat_time = (frameBuffer[reply_Id].data[8]<<8) + frameBuffer[reply_Id].data[9];
									heat_time = (heat_time <= 0)? HEAT_TIME_DEF : round_double(heat_time/1000.0, 1);
									
									meas_cycles = (!frameBuffer[reply_Id].data[10])? MEASUREMENT_CYCLES_DEF : frameBuffer[reply_Id].data[10];
									fill_timeout = (!frameBuffer[reply_Id].data[11])? FILLING_TIMEOUT_DEF : frameBuffer[reply_Id].data[11];
									
									span = ((frameBuffer[reply_Id].data[12]<<8) + frameBuffer[reply_Id].data[13])/10.0;
									if(span <= 0) span = SPAN_DEF;
									
									zero = ((frameBuffer[reply_Id].data[14]<<8) + frameBuffer[reply_Id].data[15])/10.0;
									zero = ZERO_NULLPUNKT - zero;
									
									batt_min = ((frameBuffer[reply_Id].data[16]<<8) + frameBuffer[reply_Id].data[17])/10.0;
									if(batt_min <= 0) batt_min = BATT_MIN_DEF;
									
									batt_max = ((frameBuffer[reply_Id].data[18]<<8) + frameBuffer[reply_Id].data[19])/10.0;
									if(batt_max <= 0) batt_max = BATT_MAX_DEF;
									
									critical_batt = (!frameBuffer[reply_Id].data[20])? CRITICAL_BATT_DEF : frameBuffer[reply_Id].data[20];
								
									#ifdef ALLOW_EEPROM_SAVING
										//eeprom_write_word(&ee_r_zero, (uint16_t)(r_zero*10));
										//eeprom_write_word(&ee_r_span, (uint16_t)(r_span*10));
										
										eeprom_write_word(&ee_transmit_slow, transmit_slow);
										eeprom_write_byte(&ee_transmit_slow_min, transmit_slow_min);
										eeprom_write_word(&ee_transmit_fast, transmit_fast);	
										eeprom_write_byte(&ee_transmit_fast_sec, transmit_fast_sec);
										eeprom_write_word(&ee_heat_time, (uint16_t) (heat_time*1000));							
										
										eeprom_write_byte(&ee_meas_cycles, meas_cycles);
										eeprom_write_byte(&ee_fill_timeout, fill_timeout);
										//eeprom_write_byte(&ee_he_min, he_min);
										eeprom_write_word(&ee_res_min, (uint16_t) (res_min*10));
										eeprom_write_word(&ee_res_max, (uint16_t) (res_max*10));
										eeprom_write_word(&ee_span, (uint16_t) (span*10));
										eeprom_write_word(&ee_zero, (uint16_t) (zero*10));
										//eeprom_write_byte(&ee_enable_pressure, enable_pressure);
										eeprom_write_word(&ee_batt_min, (uint16_t) (batt_min*10));
										eeprom_write_word(&ee_batt_max, (uint16_t) (batt_max*10));
										eeprom_write_byte(&ee_critical_batt, critical_batt);
									#endif
								}
								else {//defaults
									transmit_slow = TRANSMIT_SLOW_DEF;
									transmit_slow_min = false;
									transmit_fast = TRANSMIT_FAST_DEF;
									heat_time = HEAT_TIME_DEF;
									meas_cycles = MEASUREMENT_CYCLES_DEF;
									fill_timeout = FILLING_TIMEOUT_DEF;
									res_min = RES_MIN_DEF;
									res_max = RES_MAX_DEF;
									span = SPAN_DEF;
									zero = ZERO_DEF;
									batt_min = BATT_MIN_DEF;
									batt_max = BATT_MAX_DEF;
									critical_batt = CRITICAL_BATT_DEF;
								}
								break;
				case SET_LETTERS_CMD:	//LE - pos letters
								{//LCD_Print("!", 5, 60, 2, 1, 1, ERR, BGC);
									uint8_t byte_number = frameBuffer[reply_Id].data_len;	//get_data64(cmd_buffer, TX) - 3;
									uint8_t *ptr = frameBuffer[reply_Id].data;				//&cmd_buffer[3];
									
									//draw_int(byte_number, 5, 20, "", FGC);
									
									uint8_t i_letters = 1;
									uint8_t j_letters = 0;
									//uint8_t i_ranges = 1;
									uint8_t j_ranges = 1;
									
									fill_pos_letters[0][0] ='\r'; 
									fill_pos_letters[0][1] ='\0';
									
									fill_pos_ranges[0][0] = END; 
									
									while(byte_number)
									{
										fill_pos_letters[i_letters][j_letters] = ((char)(*ptr));
										j_letters++;
										ptr++;
										byte_number--;
										
										if(*ptr == ';') //ranges 
										{
											fill_pos_letters[i_letters][j_letters] = '\0';
											fill_pos_ranges[i_letters][0] = END;
											ptr++;
											byte_number--;
											
											while(*ptr != SEP)
											{
												fill_pos_ranges[i_letters][j_ranges] = *ptr;
												j_ranges++;
												ptr++;
												byte_number--;
												if(j_ranges == FILL_RANGES_LENGTH-1) break;
												//j darf net größer 20 werden und bei 20. muss immer END rein
											}
											fill_pos_ranges[i_letters][j_ranges] = END;
											
											ptr++;
											byte_number--;
											i_letters++;
											if(i_letters == FILL_RANGES_COUNT-1) break;
											//i darf net größer 14 werden und bei 14. muss immer '\r' rein 
											j_letters = 0;
											j_ranges = 1;
										}
									}
									fill_pos_letters[i_letters][0] ='\r';
									fill_pos_ranges[i_letters][0] = END; 
								}
								break;				
				case GET_LETTERS_CMD:	
								{//own scope
									//LCD_Print("?", 5, 50, 2, 1, 1, ERR, BGC);
									char *ptr;
									uint8_t index = 1;
									uint8_t i = 2;
									ptr = &fill_pos_letters[index][0];;
									
									buffer[0] = device_id>>8;
									buffer[1] = device_id;
									
									while(*ptr != '\r')
									{
										//ptr = fill_pos_letters[index][0];
										while(*ptr != '\0')
										{
											buffer[i++] = *ptr++;
										}
										buffer[i++] = ';';
										ptr = ((char*) &fill_pos_ranges[index][1]);
										
										while(*ptr != END)
										{
											buffer[i++] = ((char)(*ptr++));
										}
										buffer[i++] = SEP;
										
										index++;
										ptr = &fill_pos_letters[index][0];
									}
									buffer[i++] = status_byte;
									
									sending_cmd = pack_tx64_frame(GET_LETTERS_CMD, buffer, i, dest_high, dest_low);
									basic_send(buffer);
								}
								break;
				case GET_PASSWORD_CMD:
								buffer[0] = device_id>>8;
								buffer[1] = device_id;
								buffer[2] = options_pw >> 8;
								buffer[3] = options_pw;
								buffer[4] = status_byte;
								
								sending_cmd = pack_tx64_frame(GET_PASSWORD_CMD, buffer, 5, dest_high, dest_low);
								basic_send(buffer);
								break;
				case SET_PASSWORD_CMD:
								//LCD_Cls(white);
								//LCD_Print("!", 5, 5, 2, 1, 1, ERR, BGC);
								//draw_int(frameBuffer[reply_Id].data[0], 5, 30, "", blue);
								//draw_int(frameBuffer[reply_Id].data[1], 5, 40, "", blue);
								options_pw = (frameBuffer[reply_Id].data[0]<<8) + frameBuffer[reply_Id].data[1];
								//draw_int(options_pw, 5, 80, "", blue);
								break;
				case GET_XBEE_SLEEP_TIME_CMD:
								buffer[0] = device_id>>8;
								buffer[1] = device_id;
								buffer[2] = xbee_sleep_period;
								buffer[3] = status_byte;
								
								sending_cmd = pack_tx64_frame(GET_XBEE_SLEEP_TIME_CMD, buffer, 4, dest_high, dest_low);
								basic_send(buffer);
								break;
				case SET_XBEE_SLEEP_TIME_CMD:
								//LCD_Print("!", 5, 60, 2, 1, 1, ERR, BGC);
								xbee_sleep_period = frameBuffer[reply_Id].data[0];
								#ifdef ALLOW_DEBUG
									draw_int(xbee_sleep_period, 5,  190, "", blue);
								#endif
								break;
				default:	
								buffer[0] = status_byte;
								sending_cmd = pack_tx64_frame(ERROR_MSG, buffer, 1, dest_high, dest_low);
								basic_send(buffer);
								break;
			}//switch(frameBuffer[reply_Id].type)
			BUFF_removeData(reply_Id);	//mark cmd as done !
		}//if buffersize
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	}//while(1)
}
