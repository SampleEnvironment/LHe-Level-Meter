// Includes:
#include <avr/io.h>
#include "buffer.h"

// Global Variables:
volatile BuffType       *setPtr;
volatile BuffType       *getPtr;
volatile BuffType       frameBuffer[BUFFER_LENGTH];
volatile uint8_t       	bufferSize;


// Routines:
void init_Buffer(void)
{
	//uint8_t SREG_Save = SREG;
	//cli();

	setPtr = (BuffType*)&frameBuffer; 	// Set up the IN pointer to the start of the buffer
	getPtr = (BuffType*)&frameBuffer; 	// Set up the OUT pointer to the start of the buffer

	bufferSize = 0;           			// Reset the buffer elements counter
	
	//SREG = SREG_Save;
}

void BUFF_storeData(BuffType data)
{
	//uint8_t SREG_Save = SREG;
	//cli();

	if(bufferSize < BUFFER_LENGTH) 			
		bufferSize++; 
	
	*setPtr = data;   							// store data
	setPtr++;									// increment the IN pointer to the next element
	
	if (setPtr == (BuffType*)&frameBuffer[BUFFER_LENGTH])
		setPtr = (BuffType*)&frameBuffer; 	// Wrap pointer if end of array reached
		
	//SREG = SREG_Save;
}	

void BUFF_removeData(uint8_t index)			
{
	frameBuffer[index].status = 0xFF;		//bad frame
}

