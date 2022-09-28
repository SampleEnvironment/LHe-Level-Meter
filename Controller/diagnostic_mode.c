/*
* diagnostic_mode.c
*
* Created: 01.03.2021 11:35:53
*  Author: Weges
*/

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "base_controller.h"
#include "diagnostic_mode.h"
#include "option_mode.h"
#include "../display_utilities.h"
#include "../HoneywellSSC.h"
#include "../main.h"
#include "../avr-util-library/xbee.h"
#include "../avr-util-library/xbee_utilities.h"
#include "../timer_utilities.h"
#include "../avr-util-library/adwandler.h"
#include "../diag_pulse.h"
#include "pulse_select_mode.h"


struct Controller_Model_Vtable diag_Vtable ={
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&diag_pressedTOP,
	&diag_pressedUP,
	&diag_pressedLEFT,
	&diag_pressedRIGHT,
	&diag_pressedDOWN,
	&diag_pressedBOT,
	&diag_pressedNONE,
	
	&base_display_always_on,
	&base_display_DONOTHING,
	
	&base_pressedDONOTHING,
	&base_pressedDONOTHING
};

Controller_Model_diagnostic diag_model ={
	.super.mode = &global_mode,
	.super.vtable = &diag_Vtable,
	.super.batt_check = true,
	.page_Nr = 1,
	.temp_r_span = 0,
	.temp_r_zero = 0,
	.temp_rmin_calc = 0,
	.temp_rmax_calc = 0
	
};

Controller_Model_diagnostic* get_diag_model(){
	return &diag_model;
}

//return to options
void diag_pressedTOP(Controller_Model *Model){
	if(CHECK_MEASURE_PIN) MEASURE_PIN_OFF
	
	Model->mode->next = ex_options;
	set_timeout(0, TIMER_3, RESET_TIMER);
	set_timeout(OPT_TIMEOUT_TIME, TIMER_3, USE_TIMER);

	set_OptionModel(2,1,0);
	diag_model.page_Nr = 1;
	
	opt_drawPage();
	
	base_display_extend_onTime();
}


// do calibration if on Page 1
void diag_pressedUP(Controller_Model *Model){
	if (diag_model.page_Nr == 1)
	{
		calibration(Model);
	}
}

//cycle page left
void diag_pressedLEFT(Controller_Model *Model){
	diag_model.page_Nr++;
	if(diag_model.page_Nr == 3) {diag_model.page_Nr = 1;}
	paint_diag(diag_model.page_Nr);
}

//cycle page right
void diag_pressedRIGHT(Controller_Model *Model){
	diag_model.page_Nr--;
	if(diag_model.page_Nr == 0) {diag_model.page_Nr = 2;}
	paint_diag(diag_model.page_Nr);
}


// Switch ON/OFF current source (only on page 1)
void diag_pressedDOWN(Controller_Model *Model){
	if(diag_model.page_Nr == 1)
	{
		if(CHECK_MEASURE_PIN)
		{
			// Stop PWM
			DDRD &= (0 << PD7);			// Set PORTD.7 as input

			MEASURE_PIN_OFF

			LCD_Print(STR_OFF, xoff+X_PD_40,Y_PD_100, 2, 1,1, ERR, BGC);
		}
		else
		{
			// Start PWM
			TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << WGM21) | (1 << WGM20);
			TCCR2B = (0 << WGM22) | (1 << CS22) | (0 << CS21) | (1 << CS20);
			DDRD |= (1 << PD7);			// Set PORTD.7 as output

			// Switch on current supply (meas_current)
			// OCR2A value has to be in 0 to 255 range
			// 0 gives 0 mA / 255 gives 238 mA
			// OCR2A = (uint8_t)round(meas_current*(double)(255/238) * 1.09); //???
			// Calculation of the corrected set value for the current with the constants given in main.h
			OCR2A = (uint8_t)round(LVM.options->meas_current*(double)(SET_CURRENT_FACTOR)+(double)(SET_CURRENT_OFFSET));

			MEASURE_PIN_ON

			LCD_Print(STR_ON, xoff+X_PD_40, Y_PD_100, 2, 1,1, ERR, BGC);
		}
	}
	_delay_ms(250);
}

//--> diag Pulse
void diag_pressedBOT(Controller_Model *Model){
	if(CHECK_MEASURE_PIN) MEASURE_PIN_OFF
	
	
	diag_model.page_Nr = 1;
	pulse_select_Init();
	pulse_select_set_Model(1, 1, 1);
	pulse_select_drawPage();
	
	set_timeout(0, TIMER_3, RESET_TIMER);
	set_timeout(OPT_TIMEOUT_TIME, TIMER_3, USE_TIMER);
	
	Model->mode->next = ex_pulse_select;
	
}

//continuously update current diag page
void diag_pressedNONE(Controller_Model *Model){
	switch(diag_model.page_Nr)
	{
		case 1:
		diag_page1(LVM.options->r_zero, LVM.options->r_span, LVM.options->batt_min, LVM.options->batt_max, LVM.options->res_min, LVM.options->res_max, LVM.options->zero, LVM.options->span);
		break;
		case 2:
		diag_page2(LVM.options->r_zero);
		break;
		default:
		break;
	}

}





void calibration(Controller_Model *Model){
	LCD_Cls(BGC);
	if(LCD_Dialog(STR_CALIBRATION, STR_SET_RESISTANCE_OR_NZERO_TO_DEFAULT , D_FGC, D_BGC,CALIBRATE_TIMEOUT_TIME))
	{
		// User answered "Yes"
		LVM.options->r_zero = R_ZERO_DEF;
		LVM.options->r_span = R_SPAN_DEF;

		paint_diag(diag_model.page_Nr);


	}
	else
	{
		// User answered "No, I want to calibrate the levelmeter"
		if(!CHECK_MEASURE_PIN)
		{
			// Start PWM
			TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << WGM21) | (1 << WGM20);
			TCCR2B = (0 << WGM22) | (1 << CS22) | (0 << CS21) | (1 << CS20);
			DDRD |= (1 << PD7);			// Set PORTD.7 as output

			// Set current output to measurement value (user defined)
			// OCR2A = (uint8_t)round(meas_current*(double)(255/238)) * 1.09; //???  Faktor 255/238???
			// Calculation of the corrected set value for the current with the constants given in main.h
			OCR2A = (uint8_t)round(LVM.options->meas_current*(double)(SET_CURRENT_FACTOR)+(double)(SET_CURRENT_OFFSET));

			// Close relay
			MEASURE_PIN_ON
		}

		// First measure at 10% of the expected range (allowed values: minimum 10 Ohm, 50 Ohm, 100 Ohm, 150 Ohm, 250 Ohm etc.)
		uint16_t r_calib_min = round(LVM.options->res_min / 50) * 50;
		if (r_calib_min < 10) r_calib_min = 10;

		sprintf(LVM.temp->string,STR_SET_RESISTANCE_TO_N_OHM, r_calib_min);
		if(!LCD_Dialog(STR_CALIBRATION, LVM.temp->string, D_FGC, D_BGC,CALIBRATE_TIMEOUT_TIME))
		{
			// User answered "No"
			_delay_ms(300);

			// Open relay
			MEASURE_PIN_OFF

			// Set current output to measurement value (user defined)
			OCR2A = 0;

			// Stop PWM
			DDRD &= (0 << PD7);			// Set PORTD.7 as input

			paint_diag(diag_model.page_Nr);

			
			return;
		}
		else
		{
			// User answered "Yes"

			// measure current and voltage and calculate the resistance

			// wait some time for the current source
			LCD_Cls(BGC);
			LCD_Print(STR_CALIBRATION, X_M_5, Y_M_20, 2, 1,1, ERR, BGC);
			_delay_ms(2000);

			double temp_u = 0;
			double temp_i = 0;
			for(uint8_t i=LVM.options->meas_cycles;i > 0; i--)
			{
				draw_int(i,50,50,"",red,2);
				temp_u += readChannel(VOLT_PROBE_MEAS, CALIB_LOOPS*ADC_LOOPS); // VOLT_PROBE_MEAS =2 / ADC_LOOPS=10 /
				temp_i += readChannel(CURRENT_PROBE_MEAS, CALIB_LOOPS*ADC_LOOPS); // CURRENT_PROBE_MEAS = 1
				
			}
			temp_u = (double) temp_u/LVM.options->meas_cycles;		// Calculate the averaged voltage
			temp_i = (double) temp_i/LVM.options->meas_cycles;		// Calculate the averaged current

			temp_u = map_to_volt(temp_u);
			temp_i = map_to_current(temp_i);
			if (temp_i != 0)
			diag_model.temp_rmin_calc = temp_u * 1000 /temp_i;  // temp_i is in mA
			else diag_model.temp_rmin_calc = r_calib_min;

			LCD_Print(STR_R_CALIB, X_M_2, Y_M_40, 2, 1,1, ERR, BGC);
			LCD_Print(STR_VOLT , X_M_2, Y_M_60, 2, 1,1, ERR, BGC);
			LCD_Print(STR_CURR, X_M_2, Y_M_80, 2, 1,1, ERR, BGC);
			LCD_Print(STR_RMEAS , X_M_2,Y_M_100, 2, 1,1, ERR, BGC);

			draw_double(r_calib_min,X_M_100,Y_M_40,1,"o",FGC, 2);
			draw_double(temp_u,X_M_100,Y_M_60,1,"V",FGC, 2);
			draw_double(temp_i,X_M_100,Y_M_80,1,"mA",FGC, 2);
			draw_double(diag_model.temp_rmin_calc,X_M_100,Y_M_100,1,"o",FGC, 2);
			_delay_ms(5000);


			//					temp_r_zero = (double)((((res_min)/r_span)*(r_zero + (readChannel(CURRENT_PROBE_MEAS, ADC_LOOPS)))) - readChannel(VOLT_PROBE_MEAS, ADC_LOOPS));
			//					temp_r_zero = (double)((((res_min)/r_span)*(r_zero + (readChannel(CURRENT_PROBE_MEAS, ADC_LOOPS)))) - readChannel(VOLT_PROBE_MEAS, ADC_LOOPS));

			// Calibration, set resistance to the top end of the resistance range
			uint16_t r_calib_max = round(LVM.options->res_max / 50) * 50;  // in steps of 50paint_diag
			if (round(r_calib_min) == 10) if ((r_calib_max - r_calib_min) < 40)  r_calib_max = 50; // minimal difference: 50 Ohm
			if (round(r_calib_min) > 10) if ((r_calib_max - r_calib_min) < 50)  r_calib_max = r_calib_min + 50;


			sprintf(LVM.temp->string,STR_SET_RESISTANCE_TO_N_OHM, r_calib_max);
			if(!LCD_Dialog(STR_CALIBRATION, LVM.temp->string, D_FGC, D_BGC,CALIBRATE_TIMEOUT_TIME))
			{
				// User answered "No"
				_delay_ms(300);

				// Open relay
				MEASURE_PIN_OFF

				// Set current output to measurement value (user defined)
				OCR2A = 0;

				// Stop PWM
				DDRD &= (0 << PD7);			// Set PORTD.7 as input

				paint_diag(diag_model.page_Nr);

				


				return;
			}
			else
			{
				// User answered "Yes"

				// measure current and voltage and calculate the resistance

				LCD_Cls(BGC);
				LCD_Print(STR_CALIBRATION, X_M_5, Y_M_20, 2, 1,1, ERR, BGC);
				// wait some time for the current source
				_delay_ms(2000);

				temp_u = 0;
				temp_i = 0;

				for(uint8_t i=LVM.options->meas_cycles;i > 0; i--)
				{
					draw_int(i,50,50,"",red,2);
					temp_u += readChannel(VOLT_PROBE_MEAS, CALIB_LOOPS*ADC_LOOPS); // VOLT_PROBE_MEAS =2 / ADC_LOOPS=10 /
					temp_i += readChannel(CURRENT_PROBE_MEAS, CALIB_LOOPS*ADC_LOOPS); // CURRENT_PROBE_MEAS = 1
				}
				temp_u = (double) temp_u/LVM.options->meas_cycles;		// Calculate the averaged voltage
				temp_i = (double) temp_i/LVM.options->meas_cycles;		// Calculate the averaged current

				temp_u = map_to_volt(temp_u);
				temp_i = map_to_current(temp_i);

				// evaluate the measurement
				if (temp_i != 0)
				diag_model.temp_rmax_calc = temp_u * 1000 /temp_i;  // temp_i is in mA
				else diag_model.temp_rmax_calc = r_calib_max;


				LCD_Cls(BGC);
				LCD_Print(STR_RESULTS, X_M_50, Y_M_20, 2, 1,1, ERR, BGC);
				LCD_Print(STR_R_CALIB, X_M_2, Y_M_40, 2, 1,1, ERR, BGC);
				LCD_Print(STR_VOLT, X_M_2, Y_M_60, 2, 1,1, ERR, BGC);
				LCD_Print(STR_CURR, X_M_2, Y_M_80, 2, 1,1, ERR, BGC);
				LCD_Print(STR_RMEAS, X_M_2, Y_M_100, 2, 1,1, ERR, BGC);

				draw_double(r_calib_max,X_M_100,Y_M_40,1,"o",FGC, 2);
				draw_double(temp_u,X_M_100,Y_M_60,1,"V",FGC, 2);
				draw_double(temp_i,X_M_100,Y_M_80,1,"mA",FGC, 2);
				draw_double(diag_model.temp_rmax_calc,X_M_100,Y_M_100,1,"o",FGC, 2);
				_delay_ms(4000);

				//					double adc_v = readChannel(VOLT_PROBE_MEAS, ADC_LOOPS);
				//					double adc_i = readChannel(CURRENT_PROBE_MEAS, ADC_LOOPS);
				//					double temp_double1 = (double)(adc_v + temp_r_zero)/(adc_i + temp_r_zero);
				//					temp_r_span = (double)((350)/(temp_double1));
				//					temp_r_span = (double)((res_max + 10.0)/(temp_double1)); //???
				//					temp_r_span = (double)((res_max)/(temp_double1));

				// Open relay
				MEASURE_PIN_OFF

				// Set current output to measurement value (user defined)
				OCR2A = 0;

				// Stop PWM
				DDRD &= (0 << PD7);			// Set PORTD.7 as input




				// calc span
				diag_model.temp_r_span = 1;
				if ((diag_model.temp_rmax_calc-diag_model.temp_rmin_calc) != 0)
				diag_model.temp_r_span = (double)(r_calib_max-r_calib_min) / (diag_model.temp_rmax_calc-diag_model.temp_rmin_calc);

				// calc zero
				diag_model.temp_r_zero = (double) (r_calib_max - (diag_model.temp_r_span * diag_model.temp_rmax_calc));

				LCD_Cls(BGC);
				LCD_Print(STR_RESULTS, X_M_50, Y_M_20, 2, 1,1, ERR, BGC);
				LCD_Print(STR_R_SPAN, X_M_2, Y_M_60, 2, 1,1, ERR, BGC);
				LCD_Print(STR_R_ZERO, X_M_2, Y_M_80, 2, 1,1, ERR, BGC);
				draw_double(diag_model.temp_r_span,X_M_100,Y_M_60,3,"",FGC, 2);
				draw_double(diag_model.temp_r_zero,X_M_100,Y_M_80,1,"o",FGC, 2);
				_delay_ms(4000);

				sprintf(LVM.temp->string,STR_WOULD_YOU_LIKE_TO_SAVE);
				//					sprintf(temp," Span value: %6.3f \n\n Zero value: %6.3f \n\n Would you like to save?", temp_r_span, temp_r_zero);
				//					sprintf(temp," Span value: %d \n\n Zero value: %d \n\n Would you like to save?", (int)temp_r_span, (int)temp_r_zero);
				if(LCD_Dialog(STR_CALIBRATION, LVM.temp->string, D_FGC, D_BGC,CALIBRATE_TIMEOUT_TIME))
				{
					LVM.options->r_zero = diag_model.temp_r_zero;
					LVM.options->r_span = diag_model.temp_r_span;
				}
				paint_diag(diag_model.page_Nr);

			}
		}
	}

	
	
}



void diag_page1(double r_zero, double r_span, double batt_min, double batt_max, double res_min, double res_max, double zero, double span)
{

	char temp[10];
	double diag_current_adc  = readChannel(CURRENT_PROBE_MEAS, 20* ADC_LOOPS);
	double diag_voltage_adc = readChannel(VOLT_PROBE_MEAS, 20* ADC_LOOPS);
	//	double diag_pressure_adc 	= readChannel(PRESSURE, ADC_LOOPS);
	double diag_battery_adc 	= readChannel(BATTERY, 20* ADC_LOOPS);

	if(diag_current_adc <= 0)	diag_current_adc = 1;

	double diag_res = 0;
	// Calculate resistance with correction
	if (map_to_current(diag_current_adc) > 10) // > current should be > 10 mA, else broken cable?
	diag_res = (r_span * map_to_volt(diag_voltage_adc) * 1000 / map_to_current(diag_current_adc)) + r_zero;  // factor 1000:  mA to A
	else
	diag_res = 6000;  // Any cable connected
	
	double diag_battery_map	=   get_batt_level(batt_min, batt_max);
	double diag_battery_volt 	= map_to_batt(diag_battery_adc);
	//	double diag_pressure_map	= map_to_pres(pressure_adc, zero, span);

	double he_level = calc_he_level(diag_res, res_min, res_max);

	// Resistance
	LCD_Print("         ", xoff+X_PD_40, Y_PD_20, 2, 1, 1, FGC, BGC);
	//	if(he_level >= errCode_TooHighRes)  //???
	//		LCD_Print("Res: disable", 2, 20, 2, 1, 1, FGC, BGC);
	//	else
	if (diag_res >= errCode_TooHighRes)
	{
		if(CHECK_MEASURE_PIN){
			LCD_Print("Cable?", xoff+X_PD_40, Y_PD_20, 2, 1, 1, FGC, BGC);
		}
		else{
			LCD_Print("-", xoff+X_PD_40, Y_PD_20, 2, 1, 1, FGC, BGC);
		}
	}
	else{
		draw_double(diag_res, xoff+X_PD_40, Y_PD_20, 1,"ohm", FGC, 2);
	}


	// He Liquid
	LCD_Print("         ", xoff+X_PD_40, Y_PD_40, 2, 1, 1, FGC, BGC);
	if(he_level >= errCode_TooHighRes){
		if(CHECK_MEASURE_PIN){
			LCD_Print("Cable?", xoff+X_PD_40, Y_PD_40, 2, 1, 1, FGC, BGC);
		}
		else{
			LCD_Print("-", xoff+X_PD_40, Y_PD_40, 2, 1, 1, FGC, BGC);
		}
	}
	else{
		draw_double(he_level, xoff+X_PD_40, Y_PD_40, 1,"%", FGC, 2);
	}
	// Battery
	LCD_Print("     ", xoff+X_PD_40, Y_PD_60, 2, 1, 1, FGC, BGC);
	draw_int(diag_battery_map, xoff+X_PD_50, Y_PD_60,"%", FGC, 2);
	LCD_Print("     ", xoff+X_PD_90, Y_PD_60, 2, 1, 1, FGC, BGC);
	//draw_double(diag_battery_volt, 90, 60, 1,"V", FGC);
	dtostrf(diag_battery_volt,4,1,temp);
	strcat(temp, "V");
	LCD_Print("      ", xoff+X_PD_90, Y_PD_60, 2, 1,1, FGC, BGC);
	LCD_Print(temp, xoff+X_PD_90, Y_PD_60, 2, 1,1, FGC, BGC);

	// Pressure
	LCD_Print("         ", xoff+X_PD_40, Y_PD_80, 2, 1, 1, FGC, BGC);
	// enter pressure
	double pressure_level = 0;
	if (HoneywellSSC_status.connected)
	{
		HoneywellSSC_read_pressure();
		if (HoneywellSSC_status.status < 4) pressure_level = HoneywellSSC_Pressure;
	}
	draw_int(pressure_level, xoff+X_PD_50, Y_PD_80,"mbar", FGC, 2);

	// Current
	(CHECK_MEASURE_PIN)?
	LCD_Print("ON ", xoff+X_PD_40, Y_PD_100, 2, 1,1, ERR, BGC)
	:	LCD_Print("OFF", xoff+X_PD_40, Y_PD_100, 2, 1,1, ERR, BGC);
}

void diag_page2(double r_zero)
{
	double diag_current_adc  = readChannel(CURRENT_PROBE_MEAS, 20*ADC_LOOPS);
	double diag_voltage_adc = readChannel(VOLT_PROBE_MEAS, 20*ADC_LOOPS);
	double diag_main_voltage_adc = readChannel(VOLT_SUPPLY_MEAS, 20*ADC_LOOPS);

	double diag_voltage_map = map_to_volt(diag_voltage_adc);
	double diag_current_map = map_to_current(diag_current_adc);
	double diag_main_voltage_map = map_to_volt(diag_main_voltage_adc);

	//ADCV
	LCD_Print("     ", xoff+X_PD_50, Y_PD_20, 2, 1, 1, FGC, BGC);
	draw_int(diag_voltage_adc, xoff+X_PD_50, Y_PD_20,"", FGC, 2);
	LCD_Print("     ", xoff+X_PD_90, Y_PD_20, 2, 1, 1, FGC, BGC);
	draw_int(diag_voltage_adc+r_zero, xoff+X_PD_90, Y_PD_20,"", FGC, 2);   //???

	//ADCI
	LCD_Print("     ", xoff+X_PD_50, Y_PD_40, 2, 1, 1, FGC, BGC);
	draw_int(diag_current_adc, xoff+X_PD_50, Y_PD_40, "", FGC, 2);
	LCD_Print("     ", xoff+X_PD_90, Y_PD_40, 2, 1, 1, FGC, BGC);
	draw_int(diag_current_adc+r_zero, xoff+X_PD_90, Y_PD_40,"", FGC, 2);  //???

	//I
	LCD_Print("       ", xoff+X_PD_50, Y_PD_60, 2, 1, 1, FGC, BGC);
	draw_double(diag_current_map, xoff+X_PD_50, Y_PD_60, 1,"mA", FGC, 2);

	//U
	LCD_Print("       ", xoff+X_PD_50, Y_PD_80, 2, 1, 1, FGC, BGC);
	draw_double(diag_voltage_map, xoff+X_PD_50, Y_PD_80, 2,"V", FGC, 2);

	//Us
	LCD_Print("       ", xoff+X_PD_50, Y_PD_100, 2, 1, 1, FGC, BGC);
	draw_double(diag_main_voltage_map, xoff+X_PD_50, Y_PD_100, 2,"V", FGC, 2);
}

void diag_set_temp_r_span(double temp_r_span){
	diag_model.temp_r_span = temp_r_span;
}

void diag_set_temp_r_zero(double temp_r_zero){
	diag_model.temp_r_zero = temp_r_zero;
}


