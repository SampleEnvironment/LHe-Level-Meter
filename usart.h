#ifndef USART_H
#define USART_H


#include <avr/io.h>
#include "buffer.h"

//RS232 USART constants
#define USART_BAUDRATE 	9600
#define BAUD_PRESCALE 	(((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define COM 			UDR0

void init_usart(uint16_t baud);

#endif  // usart.h
