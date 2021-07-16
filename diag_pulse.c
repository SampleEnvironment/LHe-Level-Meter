/*
* diag_pulse.c
*
* Created: 08.01.2021 15:27:20
*  Author: Weges
*/



#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include "diag_pulse.h"
#include "Controller/pulse_select_mode.h"
#include "Controller/option_mode.h"
#include "timer_utilities.h"
#include "avr-util-library/I2C_utilities.h"
#include "avr-util-library/DS3231M.h"

// 1. legacy diag Pulse

// 2. long Diagpulse with constant meas current

// 3. incrementally increasing current delta I each delta t

double heattime_linear;


void diag_pulse_init(diag_pulseType* dp, _Bool headless, uint8_t pulse_type){
	Controller_Model_pulse_select* pselect_model = get_pulse_select_model();
	
	dp->elapsed_t = 0;
	dp->elapsed_t_last = 0;
	dp->headless = headless;
	dp->draw_all = 1;
	dp->top_unit = dp->u_point;
	dp->r_span = LVM.options->r_span;
	dp->r_zero = LVM.options->r_zero;
	dp->y_maxpixels = DP_Y_MAX_PIXELS_50;
	dp->quench_on = true;

	
	dp->active_point = 0;
	

	
	
	dp->top_zero = DP_U_ZEROLINE_70;
	dp->bot_zero = DP_I_ZEROLINE_125;
	dp->cursor_len = Y_DP_126 - Y_DP_curser;
	
	dp->u_avg = 0;
	dp->i_avg = 0;
	dp->n_for_avg = 0;
	
	dp->r_max = (uint16_t)LVM.options->res_max;
	
	switch (pulse_type)
	{
		case NORMAL:

		
		dp->x_fact = DP_X_FACTOR;
		dp->heat_time = pselect_model->quench_time;
		dp->wait_time = pselect_model->wait_time;
		dp->quench_current = pselect_model->quench_curr;
		dp->meas_current = pselect_model->meas_curr;



		dp->points_in_plot = DP_NUMBER_OF_POINTS_140;


		dp->t_end_quench = (uint32_t) ((dp->heat_time*1000)/OVERFLOW_IN_MS_8_BIT);
		dp->t_end_wait   = dp->t_end_quench + ((uint32_t) ((dp->wait_time*1000)/OVERFLOW_IN_MS_8_BIT));

		dp->u_max =  round(LVM.options->res_max * dp->quench_current / 1000);  // in V
		dp->i_max = (dp->quench_current > dp->meas_current) ? round(dp->quench_current) : round(dp->meas_current);

		
		dp->I_increment = 0;
		
		
		dp->pulse_type = NORMAL;
		
		dp->points_in_plot = DP_NUMBER_OF_POINTS_140;
		



		dp->diag_res = 0;
		break;
		case CONST:
		dp->x_fact = DP_X_FACTOR;
		
		dp->heat_time = pselect_model->pulse_duration;
		dp->quench_current = pselect_model->const_current;
		dp->meas_current = pselect_model->const_current;
		dp->I_increment = 0;
		dp->u_max =  round(LVM.options->res_max * dp->quench_current / 1000);  // in V
		dp->i_max = (dp->quench_current > dp->meas_current) ? round(dp->quench_current) : round(dp->meas_current);
		dp->points_in_plot = DP_NUMBER_OF_POINTS_140;
		
		dp->delta_t_timer_steps = (((uint32_t)dp->heat_time* 1000)/DP_NUMBER_OF_POINTS_140)/OVERFLOW_IN_MS_8_BIT;
		dp->t_end_quench = (uint32_t) ((dp->heat_time*1000)/OVERFLOW_IN_MS_8_BIT);

		dp->quench_end_position = 20;
		
		dp->pulse_type = CONST;

		dp->t_end_wait = UINT32_MAX-1;
		
		break;
		case LINEAR:

		dp->delta_t_timer_steps = (uint32_t)(pselect_model->delta_t*1000 /OVERFLOW_IN_MS_8_BIT);
		dp->delta_t_points = pselect_model->delta_t* 1000;
		dp->I_increment = pselect_model->delta_I;
		dp->u_max =  round(LVM.options->res_max * pselect_model->I_max / 1000);  // in V
		dp->i_max = pselect_model->I_max;
		
		dp->quench_end_position = 10;
		
		uint16_t steps = (pselect_model->I_max -pselect_model->I_min)/pselect_model->delta_I;
		heattime_linear =  steps * pselect_model->delta_t;
		dp->points_in_plot = steps +1;
		dp->heat_time = heattime_linear;
		dp->quench_current = pselect_model->I_min;
		
		dp->x_fact = DP_X_FACTOR*(DP_NUMBER_OF_POINTS_140 / dp->points_in_plot);
		
		dp->pulse_type = LINEAR;
		dp->t_end_quench = UINT32_MAX-1;
		dp->t_end_wait = UINT32_MAX-1;
		break;
	}
	


}


void diag_pulse_coords(diag_pulseType *dp){
	// Paint coordinate system and labels

	uint16_t x0 = xoff+X_DP_1-2;

	uint16_t x_len = -4+xoff+X_DP_1-2+DP_NUMBER_OF_POINTS_140 * DP_X_FACTOR;

	uint16_t y_len = dp->y_maxpixels;

	uint16_t y0_top = dp->top_zero;
	uint16_t y0_bot = dp->bot_zero;

	uint16_t y1_top = dp->top_zero - dp->y_maxpixels;
	uint16_t y1_bot = dp->bot_zero - dp->y_maxpixels;






	if (dp->draw_all)
	{
		//coordinate system for diagnostic pulse
		LCD_Print("Pulse", xoff+X_DP_20, Y_DP_2, 2, 1,1, ERR, BGC);
		
		if (dp->pulse_type == NORMAL )
		{
			paint_buttons("save", "esc", 2);
			}else{
			paint_buttons("", "esc", 2);
		}
		
		
		LCD_Box(x0,y0_top-y_len,x0+x_len,y0_top,BGC);

		// upper coordinate system for voltage
		LCD_vline(x0,y1_top,y_len,FGC); // | y-axis
		LCD_vline(x0+1,y1_top,y_len,FGC); // | y-axis
		LCD_hline(x0,y0_top,x_len,FGC); // _ x-axis
		LCD_hline(x0,y0_top+1,x_len,FGC); // _ x-axis

		LCD_hline(x0,y1_top,x_len,FGC);


		// lower coordinate system for current
		LCD_vline(x0,y1_bot,y_len,FGC); // | y-axis
		LCD_vline(x0+1,y1_bot,y_len,FGC); // | y-axis
		LCD_hline(x0,y0_bot,x_len,FGC); // _ x-axis
		LCD_hline(x0,y0_bot+1,x_len,FGC); // _ x-axis
		
		LCD_hline(x0,y1_bot,x_len,FGC);
		
		draw_int_without_erasing(dp->i_max, xoff+X_DP_3,y1_bot+Y_DP_OFFS, "mA", FGC, DP_AXIS_FONTNR);
	}

	dp->draw_all = false;

	if (global_mode.netstat == online)
	{
		

		if(LVM.options->display_reversed){
			LCD_Print("send",X_DP_2_SEND,Y_DP_31, DP_SEND_FONTNR, 1,DP_SEND_YFACTOR, FGC, BGC);
		}
		else
		{
			LCD_Print("send",X_DP_147_SEND,Y_DP_31, DP_SEND_FONTNR, 1,DP_SEND_YFACTOR, FGC, BGC);
		}
	}

	if (dp->top_unit == dp->u_point){
		draw_int(dp->u_max, xoff+X_DP_3,y1_top+Y_DP_OFFS, "V", FGC, DP_AXIS_FONTNR);
		if(LVM.options->display_reversed){
			LCD_Print("RES",X_DP_2,Y_DP_85, 2, 1,1, FGC, BGC);
		}
		else
		{
			LCD_Print("RES",X_DP_147,Y_DP_85, 2, 1,1, FGC, BGC);
		}




	}

	if (dp->top_unit == dp->r_point){
		draw_int(dp->r_max, xoff+X_DP_3,y1_top+Y_DP_OFFS, "Ohm", FGC, DP_AXIS_FONTNR);
		if(LVM.options->display_reversed){
			LCD_Print(" V ",X_DP_2,Y_DP_85, 2, 1,1, FGC, BGC);
		}
		else
		{
			LCD_Print(" V ",X_DP_147,Y_DP_85, 2, 1,1, FGC, BGC);
		}
	}








}


void diag_pulse_measure_point(diag_pulseType *dp){

	double u = map_to_volt(readChannel(VOLT_PROBE_MEAS, ADC_LOOPS*5));
	double i = map_to_current(readChannel(CURRENT_PROBE_MEAS, ADC_LOOPS*5));


	if (i < 0)
	{
		i = 0;
	}

	uint16_t r;


	double r_val = 0;

	// Calculate resistance with correction
	if (i > 10) // > current should be > 10 mA, else broken cable?
	r_val = (dp->r_span * u * 1000 / i) + dp->r_zero;  // factor 1000:  mA to A
	else
	r_val = 600;  // Any cable connected

	if(r_val < 0 ){
		r_val = 0;
	}

	r = (uint16_t) r_val;
	
	double r_ext = r_val*100;
	double u_ext = u*100;
	double i_ext = i*100;

	dp->u[dp->active_point] = (uint16_t)u_ext;
	dp->i[dp->active_point] = (uint16_t)i_ext;
	dp->r[dp->active_point] = (uint16_t)r_ext;


	dp->u_point[dp->active_point] = toY(u,dp->u_max,dp->y_maxpixels,dp->top_zero);
	dp->i_point[dp->active_point] = toY(i,dp->i_max,dp->y_maxpixels,dp->bot_zero);
	dp->r_point[dp->active_point] = toY(r,dp->r_max,dp->y_maxpixels,dp->top_zero);


	// Plot Color
	if (dp->elapsed_t <= dp->t_end_quench) // red during quench
	{
		dp->color[dp->active_point] = bright_red;
	}
	else if ((dp->elapsed_t > dp->t_end_quench) &&(dp->elapsed_t <= dp->t_end_wait)) // blue
	{
		dp->color[dp->active_point] = blue;
	}
	else if (dp->elapsed_t > dp->t_end_wait) // white for the rest
	{
		dp->color[dp->active_point] = white;

		dp->i_avg += i;
		dp->u_avg += u;
		dp->n_for_avg++;
	}


	dp->active_point++;
}


void diag_pulse_plot_seg(uint16_t index,diag_pulseType *dp){

	uint16_t col = dp->color[index];

	uint16_t x0  = xoff+(index*dp->x_fact)+X_DP_1;
	uint16_t x1  = xoff+((index+1)*dp->x_fact)+X_DP_1;

	uint16_t y0_top  = dp->top_unit[index];
	uint16_t y0_bot  = dp->i_point[index];

	uint16_t y1_top  = dp->top_unit[index+1];
	uint16_t y1_bot  = dp->i_point[index+1];

	if (!dp->headless)
	{
		LCD_Draw(x0,y0_top,x1,y1_top,0,col);
		LCD_Draw(x0,y0_bot,x1,y1_bot,0,col);
	}



}


void diag_pulse_Measure(diag_pulseType *dp){
	
	
	if (dp->headless)
	{
		paint_info_line("Diag Pulse",1);
	}

	// switch current off
	MEASURE_PIN_OFF
	// Stop PWM
	DDRD &= (0 << PD7);			// Set PORTD.7 as input
	_delay_ms(2000);



	// Set quench current

	// Start PWM
	TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (0 << WGM22) | (1 << CS22) | (0 << CS21) | (1 << CS20);
	DDRD |= (1 << PD7);			// Set PORTD.7 as output

	//	OCR2A = 160;
	OCR2A = (uint8_t)round(dp->quench_current*(double)(SET_CURRENT_FACTOR)+(double)(SET_CURRENT_OFFSET));


	diag_pulse_measure_point(dp); //measure first 0 point

	
	MEASURE_PIN_ON				// Switch on the current supply board


	// measure first while quench current is applied
	// then change to measurement current
	
	set_timeout(0, TIMER_7, RESET_TIMER);
	set_timeout(1, TIMER_7, USE_TIMER);

	while(dp->active_point < DP_NUMBER_OF_POINTS_140)
	{
		
		diag_pulse_measure_point(dp);
		diag_pulse_plot_seg(dp->active_point-2,dp);

		// change to measurement current
		
		switch (dp->pulse_type)
		{
			case NORMAL:
			if(dp->quench_on && (dp->elapsed_t >= dp->t_end_quench) ){
				OCR2A = (uint8_t)round(dp->meas_current*(double)(SET_CURRENT_FACTOR)+(double)(SET_CURRENT_OFFSET));
				dp->quench_on = false;
			}
			break;
			
			
			
			case LINEAR:
			if ( dp->quench_current <= dp->i_max)
			{
				dp->quench_current += dp->I_increment;
				if (dp->quench_current > dp->i_max)
				{
					dp->quench_current = dp->i_max;
				}
				OCR2A = (uint8_t)round(dp->quench_current*(double)(SET_CURRENT_FACTOR)+(double)(SET_CURRENT_OFFSET));
			}
			
			while ((dp->elapsed_t - dp->elapsed_t_last) <= dp->delta_t_timer_steps)
			{
				dp->elapsed_t = set_timeout(0, TIMER_7, USE_TIMER);
			}
			dp->elapsed_t_last = dp->elapsed_t;
			
			if (dp->active_point > dp->points_in_plot)
			{

				set_timeout(0,TIMER_7,RESET_TIMER);
				// switch current off
				MEASURE_PIN_OFF

				// Stop PWM
				DDRD &= (0 << PD7);			// Set PORTD.7 as input
				return;
				
			}
			
			
			break;
			
			
			case CONST:
			while ((dp->elapsed_t - dp->elapsed_t_last) <= dp->delta_t_timer_steps)
			{
				dp->elapsed_t = set_timeout(0, TIMER_7, USE_TIMER);
				if (dp->elapsed_t > dp->t_end_quench)
				{
					dp->delta_t_points = dp->heat_time*1000/(double)dp->active_point;
					
					dp->points_in_plot = dp->active_point;
					set_timeout(0,TIMER_7,RESET_TIMER);
					// switch current off
					MEASURE_PIN_OFF

					// Stop PWM
					DDRD &= (0 << PD7);			// Set PORTD.7 as input
					return;
				}
			}
			dp->elapsed_t_last = dp->elapsed_t;
			
			break;
			
			default:
			break;
		}
		dp->elapsed_t = set_timeout(0, TIMER_7, USE_TIMER);


		
		
	}
	dp->elapsed_t = set_timeout(0, TIMER_7, USE_TIMER);
	set_timeout(0,TIMER_7,RESET_TIMER);
	
	// switch current off
	MEASURE_PIN_OFF

	// Stop PWM
	DDRD &= (0 << PD7);			// Set PORTD.7 as input
	
	
	if(dp->pulse_type == CONST)
	{
		dp->delta_t_points = dp->heat_time*1000/(double)dp->active_point;
		dp->points_in_plot = dp->active_point;
	}

	if (dp->pulse_type == NORMAL)
	{
		dp->delta_t_points = (((double) dp->elapsed_t / ((double)dp->active_point))*OVERFLOW_IN_MS_8_BIT);
		
		dp->quench_end_position = (uint16_t) (dp->heat_time*1000/dp->delta_t_points);
	}
	


}


void diag_pulse_move_cursor(diag_pulseType *dp,int8_t direction){

	// erase old cursor line
	uint16_t x0 = xoff+X_DP_1 + dp->x_fact * (dp->active_point - direction);
	uint16_t y0 = Y_DP_curser;
	uint16_t lenght = dp->cursor_len;

	LCD_vline(x0,y0,lenght,BGC);


	//draw top Plot line segments
	for(uint8_t i = 0 ; i < 4; i++){
		diag_pulse_plot_seg((dp->active_point-2)+i,dp);
	}



	// redraw the erased pixel in x-axes
	LCD_vline(xoff+X_DP_1+(dp->active_point-direction)*dp->x_fact,Y_DP_71-1,2, FGC);

	LCD_vline(xoff+X_DP_1+(dp->active_point-direction)*dp->x_fact,Y_DP_126-1,2, FGC);


	uint16_t y1_top = dp->top_zero - dp->y_maxpixels;
	uint16_t y1_bot = dp->bot_zero - dp->y_maxpixels;

	LCD_Plot(xoff+X_DP_1+(dp->active_point-direction)*dp->x_fact,y1_top,0, FGC);
	LCD_Plot(xoff+X_DP_1+(dp->active_point-direction)*dp->x_fact,y1_bot,0, FGC);


	// new vertline

	x0 = xoff+X_DP_1 + dp->x_fact * dp->active_point ;
	LCD_vline(x0,y0,lenght,ERR);

	// erase old value
	LCD_Print("      ", xoff+X_DP_75, Y_DP_6, 1, 1, 1, FGC, BGC);

	// draw new value
	double current_time = round(((double)dp->active_point)*dp->delta_t_points);
	
	if (dp->heat_time >= 10)
	{
		draw_double_without_erasing(current_time/1000, xoff+X_DP_75, Y_DP_6,1,"s",ERR,1);
		}else{
		draw_int_without_erasing((uint32_t) current_time, xoff+X_DP_75, Y_DP_6, "ms ", ERR, 1);
	}
	




	// draw old labels


	if (dp->top_unit == dp->u_point){
		draw_int_without_erasing(dp->u_max, xoff+X_DP_3,dp->top_zero - dp->y_maxpixels + Y_DP_OFFS, "V", FGC, DP_AXIS_FONTNR);
		
		if(dp->pulse_type == NORMAL){
			draw_double_without_erasing(dp->diag_res, xoff+X_DP_60, Y_DP_6, 1, "Ohm", ERR, 1);
		}
		else
		{
			LCD_Print("        ", xoff+X_DP_60, Y_DP_6, 1, 1, 1, FGC, BGC);
			draw_double_without_erasing(((double)dp->u[dp->active_point])/100,xoff+X_DP_60, Y_DP_6,1, "V", ERR, 1);
		}
	}

	if (dp->top_unit == dp->r_point){
		draw_int_without_erasing(dp->r_max, xoff+X_DP_3,dp->top_zero - dp->y_maxpixels+ Y_DP_OFFS, "Ohm", FGC, DP_AXIS_FONTNR);
		LCD_Print("        ", xoff+X_DP_60, Y_DP_6, 1, 1, 1, FGC, BGC);
		draw_double_without_erasing(((double)dp->r[dp->active_point])/100,xoff+X_DP_60, Y_DP_6,1, "Ohm", ERR, 1);
	}


	draw_int_without_erasing(dp->i_max, xoff+X_DP_3 ,dp->bot_zero - dp->y_maxpixels+Y_DP_OFFS, "mA", FGC, DP_AXIS_FONTNR);

}


void diag_pulse_send(diag_pulseType *dp){
	uint8_t diag_send_buffer[SINGLE_FRAME_LENGTH];



	// Wake up XBee module
	xbee_wake_up_plus();
	

	

	if (connected.DS3231M)
	{
		DS3231M_read_time();

	}
	else
	{
		Time.tm_sec = 0;
		Time.tm_min = 0;
		Time.tm_hour  = 0;
		Time.tm_mday   = 0;
		Time.tm_mon  = 0;
		Time.tm_year   = 0;
	}

	diag_send_buffer[0] = Time.tm_sec;
	diag_send_buffer[1] = Time.tm_min;
	diag_send_buffer[2] = Time.tm_hour;
	diag_send_buffer[3] = Time.tm_mday;
	diag_send_buffer[4] = Time.tm_mon;
	diag_send_buffer[5] = Time.tm_year;

	//TODO
	uint8_t Datatpoints_per_Packet = 30;
	
	uint8_t n_Packets = (dp->points_in_plot+(Datatpoints_per_Packet-1))/ Datatpoints_per_Packet; // ineger division rounded up
	
	// Number of Packets the Pulse is divided into
	diag_send_buffer[6] = n_Packets;
	
	
	InitScreen_AddLine("Sending Pulse Data:",1);


	InitScreen_AddLine("Sending I data...",0);
	diag_send_sub_packets(dp,GET_PULSE_I,diag_send_buffer,n_Packets);
	

	_delay_ms(2000);


	InitScreen_AddLine("Sending U data...",0);
	diag_send_sub_packets(dp,GET_PULSE_U,diag_send_buffer,n_Packets);

	

	
	_delay_ms(1000);

	LCD_Box(0,0,200,100,BGC);
	
	






}


void diag_send_sub_packets(diag_pulseType *dp,uint8_t Message_code,uint8_t * diag_send_buffer,uint8_t n_Packets){
	uint16_t points_sent =  0;
	uint8_t header_offset = 11;
	
	for (uint8_t i = 0;i < n_Packets; i++)
	{
		// current Packet
		diag_send_buffer[7]= i+1;
		
		//number of Datapoints in fragment
		uint8_t datapoints_in_fragment = ((dp->points_in_plot - points_sent) > 30) ? 30 : (dp->points_in_plot - points_sent);
		diag_send_buffer[8] =  ((uint16_t)dp->delta_t_points)>>8;
		diag_send_buffer[9] =  (uint16_t)dp->delta_t_points;
		diag_send_buffer[10] = datapoints_in_fragment;

		
		
		
		for (uint16_t j = 0; j < datapoints_in_fragment; j ++)
		{
			switch (Message_code)
			{
				case GET_PULSE_U:
				
				diag_send_buffer[(j*2)+header_offset]   = (dp->u[points_sent])>>8;
				diag_send_buffer[(j*2)+header_offset+1] = dp->u[points_sent];
				break;
				
				case GET_PULSE_I:
				
				diag_send_buffer[(j*2)+header_offset]   = (dp->i[points_sent])>>8;
				diag_send_buffer[(j*2)+header_offset+1] = dp->i[points_sent];
				break;
				
			}
			points_sent++;
		}
		
		
		switch (Message_code)
		{
			case GET_PULSE_U:
			sprintf(LVM.temp->string,"Sending U data (%i/%i)",i+1,n_Packets);
			break;
			
			case GET_PULSE_I:
			sprintf(LVM.temp->string,"Sending I data (%i/%i)",i+1,n_Packets);
			break;
		}
		
		
		InitScreen_AddLine(LVM.temp->string,2);
		memcpy(LVM.temp->buffer,diag_send_buffer,(datapoints_in_fragment*2)+header_offset+1);
		xbee_send_message(Message_code,LVM.temp->buffer,(datapoints_in_fragment*2)+header_offset+1);
		_delay_ms(500);
	}
}



void diag_pulse(diag_pulseType *dp){

	diag_pulse_coords(dp); // draw Window

	diag_pulse_Measure(dp); // do Measurement --> draw plot



	// move active point to End of the quench pulse
	dp->active_point = dp->quench_end_position;




	uint16_t x0 = xoff+X_DP_1 + dp->x_fact * dp->active_point;

	uint16_t y0 = Y_DP_curser;

	uint16_t lenght = dp->cursor_len;


	LCD_vline(x0,y0,lenght,ERR);

	// draw new value
	double current_time = round(((double)dp->active_point)*dp->delta_t_points);
	
	if (dp->heat_time >= 10)
	{
		draw_double(current_time/1000, xoff+X_DP_75, Y_DP_6,1,"s",ERR,1);
		}else{
		draw_int_without_erasing((uint32_t) current_time, xoff+X_DP_75, Y_DP_6, "ms ", ERR, 1);
	}

	// Calculate Resistance
	if ((dp->n_for_avg > 0) && !(dp->i_avg== 0) && (dp->pulse_type == NORMAL))
	{
		dp->u_avg = (double) dp->u_avg/ dp->n_for_avg;
		dp->i_avg = (double) dp->i_avg / dp->n_for_avg;


		dp->diag_res = ((double) (dp->r_span* dp->u_avg* 1000 / dp->i_avg) + dp->r_zero);  // conversion from mA to A !!!!
		draw_double(dp->diag_res, xoff+X_DP_60, Y_DP_6, 1, "Ohm", ERR, 1);

		}else{
		draw_double_without_erasing(((double)dp->u[dp->active_point])/100,xoff+X_DP_60, Y_DP_6,1, "V", ERR, 1);
	}

	// adjust the end of the quench pulse with the cursor
	_Bool back = true;

	//wait for button to be released, don't accept any button before here
	while (!(keyhit_block() == 0));  // wait until no key is pressed

	// accept a new key to be pressed
	ready_for_new_key();




	while(back)
	{

		switch(keyhit_block())
		{

			case KEY_LEFT_S7:

			if(--dp->active_point < DP_PADDING_10)
			{
				dp->active_point++;
				break;
			}



			diag_pulse_move_cursor(dp,-1);

			#ifdef ili9341
			_delay_ms(50);
			#endif // ili9341

			#ifdef DISP_3000
			_delay_ms(300);
			#endif // DISP_3000

			while(keyhitwithoutinterrupt()==KEY_LEFT_S7)
			{
				if(--dp->active_point< DP_PADDING_10)
				{
					dp->active_point++;
					break;
				}

				diag_pulse_move_cursor(dp,-1);

				#ifdef DISP_3000
				_delay_ms(10);
				#endif // DISP_3000
			}

			break;
			case KEY_RIGHT_S8:
			if(++dp->active_point> dp->points_in_plot-3)
			{
				dp->active_point--;
				break;
			}

			diag_pulse_move_cursor(dp,1);

			#ifdef ili9341
			_delay_ms(50);
			#endif // ili9341

			#ifdef DISP_3000
			_delay_ms(300);
			#endif // DISP_3000

			while(keyhitwithoutinterrupt()==KEY_RIGHT_S8)
			{
				if(++dp->active_point> dp->points_in_plot-3)
				{
					dp->active_point--;
					break;
				}
				diag_pulse_move_cursor(dp,1);

				#ifdef ili9341

				#endif // ili9341

				#ifdef DISP_3000
				_delay_ms(10);
				#endif // DISP_3000
			}

			break;
			case KEY_TOP_S5: //SAVE

			
			if (dp->pulse_type != NORMAL)
			{
				break;
			}
			
			char params[20];
			char params1[20];
			char params2[20];
			char params3[20];
			dtostrf(LVM.options->quench_current,3,0,params);
			dtostrf(LVM.options->quench_time,4,1,params1);
			dtostrf(LVM.options->meas_current,3,0,params2);
			dtostrf(LVM.options->wait_time,4,1,params3);
			sprintf(LVM.temp->string,"New Values:\nQuench Current: %s mA\nQuench time: %ss\nMeas Current: %smA\nWait time: %ss",params,params1,params2,params3);
			
			if (LCD_Dialog(STR_SAVE_PULSE_PARAMS,LVM.temp->string, D_FGC, D_BGC,SAVE_PLUS_PARAMS_TIMEOUT_TIME))
			{
				if ((LVM.options->quench_current != dp->quench_current)||
				(LVM.options->quench_time    != dp->heat_time)||
				(LVM.options->meas_current   != dp->meas_current)||
				(LVM.options->wait_time      != dp->wait_time))
				{
					set_options_changed();
				}
				LVM.options->quench_current = dp->quench_current;
				LVM.options->quench_time = dp->heat_time;
				LVM.options->meas_current = dp->meas_current;
				LVM.options->wait_time = dp->wait_time;
				
			}

			back = false;
			_delay_ms(100);
			break;
			case KEY_BOT_S10: // ESC
			back = false;
			_delay_ms(100);
			not_ready_for_new_key();
			return;
			break;

			case  KEY_DOWN_S9:
			if (dp->top_unit == dp->r_point )
			{
				dp->top_unit = dp->u_point;
				}else if(dp->top_unit == dp->u_point){
				dp->top_unit = dp->r_point;
			}


			dp->draw_all = true;
			
			
			diag_pulse_coords(dp);

			for(uint8_t i = 0; i< dp->points_in_plot-1; i++){
				diag_pulse_plot_seg(i,dp);
			}
			diag_pulse_move_cursor(dp,0);

			break;

			case KEY_UP_S6: //SEND
			if (global_mode.netstat == offline)
			{
				break;
			}
			
			diag_pulse_send(dp);
			
			dp->draw_all = true;
			diag_pulse_coords(dp);

			for(uint8_t i = 0; i< dp->points_in_plot-1; i++){
				diag_pulse_plot_seg(i,dp);
			}
			diag_pulse_move_cursor(dp,0);


			break;

			default:
			break;
		}
		
		ready_for_new_key();
	}



}



