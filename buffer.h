#ifndef BUFFER_H
#define BUFFER_H

//#include <avr/io.h> 

 
// Configuration:
#define BUFFER_LENGTH	10        		// maximum frames
#define DATA_LENGTH		50				// maximum data in frame
#define BYTE 			unsigned char

#define BUFF_ClearBuffer()     bufferSize = 0;

typedef struct {
	uint8_t  type;
	uint16_t length;
	uint8_t api_id;
	uint8_t status;						//0 - ack server, 0 - OK AT, 0xFF - bad frame
	uint8_t data_len;
	uint8_t data[DATA_LENGTH];
} frame;


typedef frame BuffType; 						// Replace "uint8_t" with desired buffer storage type

extern volatile BuffType	frameBuffer[BUFFER_LENGTH];
extern volatile uint8_t 	bufferSize; 		// Holds the number of elements in the buffer



void init_Buffer(void);
void BUFF_storeData(BuffType data);
void BUFF_removeData(uint8_t index);


#endif  //buffer.h
