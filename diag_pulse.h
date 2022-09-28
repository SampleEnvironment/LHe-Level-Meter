/*
 * diag_pulse.h
 *
 * Created: 08.01.2021 15:26:15
 *  Author: Weges
 */ 


#ifndef DIAG_PULSE_H_
#define DIAG_PULSE_H_

#include <stdbool.h>
#include <string.h>

#include "main.h"
#include "adwandler.h"
#include "display_utilities.h"
#include "xbee.h"
#include "keyboard.h"
#include "Controller/base_controller.h"
#define NORMAL 1
#define CONST  2
#define LINEAR 3

#define TIME_TO_MEASURE 11 // time it takes the Levelmeter to do a Measurement in ms

extern globalModesType global_mode;




void diag_pulse_init(diag_pulseType* dp, _Bool headless, uint8_t pulse_type);
void diag_pulse_coords(diag_pulseType *dp);
void diag_pulse_measure_point(diag_pulseType *dp);
void diag_pulse_plot_seg(uint16_t index,diag_pulseType *dp);
void diag_pulse_Measure(diag_pulseType *dp);
void diag_pulse_move_cursor(diag_pulseType *dp,int8_t direction);
void diag_pulse_send(diag_pulseType *dp);
void diag_pulse(diag_pulseType *dp);
void diag_send_sub_packets(diag_pulseType *dp,uint8_t Message_code,uint8_t * diag_send_buffer,uint8_t n_Packets);

#endif /* DIAG_PULSE_H_ */