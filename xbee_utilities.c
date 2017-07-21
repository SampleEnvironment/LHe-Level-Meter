// xbee_utilities.c - Copyright 2016, HZB, ILL/SANE & ISIS
#define RELEASE_XBEE_UTILS 1.00

// HISTORY -------------------------------------------------------------
// 1.00 - First release on June 2016

#include <avr/io.h>
#include <string.h>
//#include "buffer.h"
//#include <util/delay.h>

#include "xbee.h"
#include "xbee_utilities.h"

// Global Variables
volatile BuffType *setPtr;
volatile BuffType frameBuffer[BUFFER_LENGTH];
volatile uint8_t bufferSize;


// 
void buffer_init(void)
{
	setPtr = (BuffType*)&frameBuffer; 	// Set up the IN pointer to the start of the buffer

	bufferSize = 0;           			// Reset the buffer elements counter
}

void buffer_storeData(BuffType data)
{
	if(bufferSize < BUFFER_LENGTH)
	bufferSize++;
	
	*setPtr = data;   							// store data
	setPtr++;									// increment the IN pointer to the next element
	
	if (setPtr == (BuffType*)&frameBuffer[BUFFER_LENGTH])
	setPtr = (BuffType*)&frameBuffer; 	// Wrap pointer if end of array reached
}

// Clear buffer status
void buffer_removeData(uint8_t index)
{
	frameBuffer[index].status = 0xFF;		//bad frame
}

// Return the index of reply corresponding to the command type
uint8_t xbee_hasReply(uint8_t cmd_type, uint8_t range)
{
	switch(range)
	{
		case EQUAL:
			for(uint8_t i = 0; i < bufferSize; ++i)
			{	
				if((frameBuffer[i].status == 0x00) && (frameBuffer[i].type == cmd_type))
				{
					return i;
				}
			}
			break;
// 		case LESS_THAN:
// 			for(uint8_t i = 0; i < bufferSize; ++i)
// 			{	
// 				if((frameBuffer[i].status == 0x00) && (frameBuffer[i].type < cmd_type))
// 				{
// 					return i;
// 				}
// 			}
// 			break;
		case GREATER_THAN:
			for(uint8_t i = 0; i < bufferSize; ++i)
			{	
				if((frameBuffer[i].status == 0x00) && (frameBuffer[i].type > cmd_type))
				{
					return i;
				}
			}
			break;
	}
	return 0xFF;
}

// ...
inline uint8_t get_at_frame_type(char *id, uint8_t len)
{
	//if(len < 2) return UNDEFINED;
	if((id[0] == 'D') && (id[1] == 'A')) return DA_MSG_TYPE;
	if((id[0] == 'D') && (id[1] == 'L')) return DL_MSG_TYPE;
	if((id[0] == 'D') && (id[1] == 'H')) return DH_MSG_TYPE;
	
	return 0;
}

// Build a frame for XBee communication
inline void xbee_build_frame(uint8_t *buffer, uint8_t length)
{
	if(length < 5) return;
	if(!xbee_chkChecksum(buffer)) return;
	
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
	buffer_storeData(newFrame);
}

// Store device position (instrument id or labs) into the buffer
inline uint8_t devicePos_to_buffer(char *pos, uint8_t offset, uint8_t *buffer)
{
	uint8_t index = 0;
	
	while (pos[index]!='\0')
	{
		buffer[index+offset] = pos[index];
		index++;
	}
	
	//fill spaces if pos is less then 4 letters
	for(uint8_t i=0; i<(4 - index); i++) buffer[i+index+offset] = 0x20;
	
	//return index, add spaces
	return (index + offset) + (4 - index);
}

// Pack data (string + int parameters) in a XBee-frame (AT) ready to send from frame_buffer
// Only used with xbee_reset_connection & xbee_getAddress functions
uint8_t xbee_pack_tx_frame(uint8_t *params, uint8_t paramsNumber)
{
	uint8_t temp_buffer[SINGLE_FRAME_LENGTH];
	uint8_t index = 5;
	
	uint8_t *temp = params;

	temp_buffer[0] = 0x7E;		// Start delimiter
	// Let index 1 & 2 free for storing frame length 
	temp_buffer[3] = 0x08;   	// API Identifier Value for AT Command type allows for module parameters to be queried or set.
	temp_buffer[4] = 0x42;		// Constant Frame ID arbitrarily selected / Different of 0 to get an answer
	
	while (paramsNumber)
	{
		temp_buffer[index++] = *params++;
		paramsNumber--;
	}
	
	temp_buffer[index] = xbee_getChecksum(temp_buffer,3,index);		// Calculate checksum
	
	temp_buffer[1] = ((index-3) >> 8);
	temp_buffer[2] = (index-3);
	
	memcpy(temp, temp_buffer, index+1);		// Store frame in the params array
		
	return index+1;		// Return number of bytes packed
}
// Function provided for convenience.
// When using this function you will able to send the data properly only once.
// Pure data passed in frame_buffer will be packed in frame_buffer.

// Pack data (string + int parameters) in a XBee-frame (TX 64-bit address) ready to send
// Disable acknowledgment and response frame
// Returns number of bytes packed
uint8_t xbee_pack_tx64_frame(uint8_t db_cmd_type, uint8_t *params, uint8_t paramsNumber, uint32_t dest_high, uint32_t dest_low)
{
	uint8_t temp_buffer[SINGLE_FRAME_LENGTH];
	uint8_t index = 0;
	
	uint8_t *temp = params;
	
	temp_buffer[0] = 0x7E;		// Start delimiter
	// Let index 1 & 2 free for storing frame length 
	temp_buffer[3] = 0x00;   	// API Identifier Value for 64-bit TX Request message will cause the module to send RF Data as an RF Packet.
	temp_buffer[4] = 0x00;		// Constant Frame ID arbitrarily selected
								// Identifies the UART data frame for the host to correlate with a subsequent ACK (acknowledgment).
								// Setting Frame ID to ‘0' will disable response frame.
	
	// 32-bit destination address high
	temp_buffer[5] = (uint8_t)(dest_high >> 24);
	temp_buffer[6] = (uint8_t)(dest_high >> 16);
	temp_buffer[7] = (uint8_t)(dest_high >> 8);
	temp_buffer[8] = (uint8_t) dest_high;
	
	// 32-bit destination address low
	temp_buffer[9] = (uint8_t)(dest_low >> 24);
	temp_buffer[10] = (uint8_t)(dest_low >> 16);
	temp_buffer[11] = (uint8_t)(dest_low >> 8);
	temp_buffer[12] = (uint8_t) dest_low;
	
	temp_buffer[13] = 0x01;				// Disable acknowledgment
	temp_buffer[14] = db_cmd_type;		// Database command type
	
	index = 15;
	// Parameter
	while (paramsNumber)
	{
		temp_buffer[index] = *params++;	
		index++;
		paramsNumber--;
	}
	
	// Calculate checksum
	temp_buffer[index] = xbee_getChecksum(temp_buffer,3,index);
	
	// Add frame length - excludes Start delimiter, Length and checksum
	temp_buffer[1] = ((index-3) >> 8);
	temp_buffer[2] = (index-3);
	
	memcpy(temp, temp_buffer, index+1);		// Frame is in the params array
	
	return index+1;
}

// Calculate checksum of the packed frame
uint8_t xbee_getChecksum(uint8_t *buffer, uint8_t start, uint8_t stop)
{
	if (start > stop) return 0;
	
	uint8_t summe = 0;
	uint8_t index = start;
	
	while(index < stop)
	{//add all bytes from start to stop
		summe += buffer[index++];
	}
	return 0xFF-summe;//result;
}

// Return true if checksum is correct, false otherwise
_Bool xbee_chkChecksum(uint8_t *buffer)
{
	uint8_t index = 0;
	uint8_t summe = 0;
	uint16_t bytes_number = xbee_get_packet_len(buffer)+1; //data + checksum
	
	while(bytes_number)
	{//add all bytes after frame length (3.byte)
		summe += buffer[index+3];
		bytes_number--;
		index++;
	}
	return (summe == 0xFF);
}

// Return the length of the passed XBee-frame
uint8_t xbee_get_packet_len(uint8_t *buffer)
{
	return (buffer[1]<<8) + buffer[2];
}

// Get high and low bytes of basis station
// Used only with "DL" and "DH" command type
uint32_t xbee_get_addr(uint8_t cmd_type)
{
	uint32_t dest_addr;	
	uint8_t send_buffer[SINGLE_FRAME_LENGTH];
	
	if(cmd_type == DL_MSG_TYPE)
	{
		send_buffer[0] = (uint8_t)'D';
		send_buffer[1] = (uint8_t)'L';
	}
	else {
		send_buffer[0] = (uint8_t)'D';
		send_buffer[1] = (uint8_t)'H';
	}
	
	// Pack frame according to type
	uint8_t temp_bytes_number = xbee_pack_tx_frame(send_buffer, 2);  // Add DL or DH
	
	if (xbee_get_reply(send_buffer, temp_bytes_number, cmd_type, 500) != 0xFF)
	{
		dest_addr = (unsigned long int)send_buffer[0]<<24;
		dest_addr += (unsigned long int)send_buffer[1]<<16;
		dest_addr += (unsigned long int)send_buffer[2]<<8;
		dest_addr += (unsigned long int)send_buffer[3];
		
		return dest_addr;
	}
	else return 0;		// Coundn't read addr_high or addr_low
}

