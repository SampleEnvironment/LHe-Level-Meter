// xbee_utilities.h - Copyright 2016, HZB, ILL/SANE & ISIS

#ifndef XBEE_UTILITIES_H_
#define XBEE_UTILITIES_H_

// Includes
#include <avr/io.h>

// Configuration
#define BUFFER_LENGTH	10        		// maximum frames
#define DATA_LENGTH		50				// maximum data in frame
//#define BYTE 			unsigned char

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

extern volatile BuffType frameBuffer[BUFFER_LENGTH];
extern volatile uint8_t bufferSize; 		// Holds the number of elements in the buffer

//Frame IDs
enum  reply_type{
	EQUAL = 1,
//	LESS_THAN,
	GREATER_THAN
};

#define LAST_NON_CMD_MSG		10		// Search no ack commands only (see xbee_hasReply)

//==============================================================
// XBee API commands
//==============================================================
#define AT_ID					0x88	// In response to an AT Command message, the module will send an AT Command Response message.
#define RX_ID					0x80	// RX Packet: 64-bit Address / When the module receives an RF packet, it is sent out the UART using this message type.
#define STA_ID					0x8A	// RF module status messages are sent from the module in response to specific conditions.

#define DA_MSG_TYPE 			18		// Force dissociation API command
#define DH_MSG_TYPE 			19		// Destination address high API command
#define DL_MSG_TYPE 			20		// Destination address low API command
#define STATUS_MSG_TYPE 		21		//

//==============================================================
// XBee utilities commands
//==============================================================
void buffer_init(void);					// ...
void buffer_storeData(BuffType data);	// ...
void buffer_removeData(uint8_t index);	// Clear buffer status
uint8_t xbee_hasReply(uint8_t cmd_type, uint8_t range);	// Return the index of reply corresponding to the command type
uint8_t get_at_frame_type(char *id, uint8_t len);
void xbee_build_frame(uint8_t *buffer, uint8_t length);	// Build a frame for XBee communication
uint8_t devicePos_to_buffer(char *pos, uint8_t offset, uint8_t *buffer);	// Store device position (instrument id or labs) into the buffer
uint8_t xbee_pack_tx_frame(uint8_t *params, uint8_t paramsNumber);	// Pack data (string + int parameters) in a XBee-frame (AT) ready to send from frame_buffer
uint8_t xbee_pack_tx64_frame(uint8_t db_cmd_type, uint8_t *params, uint8_t paramsNumber, uint32_t dest_high, uint32_t dest_low);	// Pack data (string + int parameters) in a XBee-frame (TX 64-bit address) ready to send
uint8_t xbee_getChecksum(uint8_t *buffer, uint8_t start, uint8_t stop);	// Calculate checksum of the packed frame
_Bool xbee_chkChecksum(uint8_t *buffer);		// Return true if checksum is correct, false otherwise
uint8_t xbee_get_packet_len(uint8_t *buffer);	// Return the length of the passed XBee-frame
uint32_t xbee_get_addr(uint8_t cmd_type);		// Get high and low bytes of basis station

#endif /* XBEE_UTILITIES_H_ */