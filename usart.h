// Usart.h - Copyright 2016, HZB, ILL/SANE & ISIS

#ifndef USART_H
#define USART_H

//RS232 USART constants
#define USART_BAUDRATE 	9600			// Define USART0 baudrate
#define USART_IODR 		UDR0			// Define USART0 I/O Data Register


//==============================================================
// USART commands
//==============================================================

void usart_init(uint16_t baud);			// Initialize Interrupt Service Routine driven USART communication
ISR(USART0_RX_vect); 			// USART0 data received interrupt service routine (from XBee module)
ISR(USART0_TX_vect); 			// USART0 data transmitted interrupt service routine (to XBee module)

#endif  // usart.h
