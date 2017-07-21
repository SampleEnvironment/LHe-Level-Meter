#include "usart.h"



///initializes interrupt driven USART232 communication
void init_usart(uint16_t baud) 
{
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1 << TXCIE0);   					//Turn on the transmission and reception circuitry				
																									//Enable the USART Recieve Complete interrupt
	
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); 					//Use 8-bit character sizes
	
	//UCSR0C &= ~((1<<UMSEL00) | (1<<UMSEL01) | (1<<USBS0));	//asynchronous, 1 stop bit
	
																//Load lower 8-bits of the baud rate value into the low byte of the UBRR register
	UBRR0 = baud; 												//Load upper 8-bits of the baud rate value into the high byte of the UBRR register

	init_Buffer();												//Init RingBuffer
}


