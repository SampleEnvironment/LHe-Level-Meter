#ifndef COMMUTILS_H
#define COMMUTILS_H

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "grafik.h"
#include "screenutils.h"
#include "timerutils.h"
#include "usart.h"
#include "kanne.h"

#define AT_ID					0x88
#define RX_ID					0x80
#define STA_ID					0x8A

#define SINGLE_FRAME_LENGTH 	50


#define DA_MSG_TYPE 			18
#define DH_MSG_TYPE 			19
#define DL_MSG_TYPE 			20
#define STATUS_MSG_TYPE 		21

//command types
//with ack
#define LOGIN_MSG				1
#define FORCE_LOGIN_MSG			2
#define LONG_INTERVAL_MSG 		3
#define OPTIONS_CHANGED_MSG		4
#define FILLING_BEGIN_MSG		5
#define FILLING_MSG				6
#define FILLING_END_MSG			7
#define LOGOUT_MSG				8

//no ack needed
#define XBEE_ACTIVE_MSG			9
#define LAST_NON_CMD_MSG		10	//for id search only (not a type)
#define STATUS_MSG				11	
#define GET_OPTIONS_CMD			12
#define SET_OPTIONS_CMD			13
#define GET_LETTERS_CMD			14
#define SET_LETTERS_CMD			15
#define GET_PASSWORD_CMD		16
#define SET_PASSWORD_CMD		17
#define GET_XBEE_SLEEP_TIME_CMD	18
#define SET_XBEE_SLEEP_TIME_CMD	19

#define ERROR_MSG				10


//Frame IDs
enum  reply_type{
	EQUAL = 1,
	LESS_THEN,
	GREATER_THEN
};

extern volatile uint8_t status_byte;



void reconnect(uint32_t *addr_low, uint32_t *addr_high);
void send_request(uint8_t cmd_type, uint8_t *buffer, uint8_t length, uint32_t *dest_high, uint32_t *dest_low);

bool chkChecksum(uint8_t *buffer);
uint8_t getChecksum(uint8_t *buffer, uint8_t start, uint8_t stop);
//uint8_t parse_cmd(uint8_t *buffer); 

//uint8_t pack_tx16_frame(uint8_t *data_buffer, uint8_t *frame_buffer, uint16_t destination);
//uint8_t pack_tx16_frame2(uint8_t *frame_buffer, uint16_t destination);

uint8_t pack_tx64_frame(uint8_t cmd_type, uint8_t *params, uint8_t paramsNumber, uint32_t dest_high, uint32_t dest_low);
uint8_t pack_tx64_frame_no_params(const char *data_buffer, uint8_t *frame_buffer, uint32_t dest_high, uint32_t dest_low);
//uint8_t pack_tx64_frame2(uint8_t *frame_buffer, uint32_t dest_low, uint32_t dest_high);


uint8_t pack_at_frame(uint8_t *params, uint8_t paramsNumber);
//uint8_t pack_at_frame2(uint8_t *frame_buffer);

uint8_t hasReply(uint8_t apiId, uint8_t range);
void send_MSG(uint8_t *buffer, uint8_t length, uint16_t delay);
uint8_t get_reply(uint8_t *buffer, uint8_t length, uint8_t cmd_type, uint16_t delay);

void clear_buffer(uint8_t *buffer);
uint8_t get_packet_len(uint8_t *buffer);
//uint8_t get_data64(uint8_t *buffer, bool tx);
//uint8_t send_and_wait(uint8_t cmd_type, uint8_t *params, uint8_t paramsNumber, uint32_t addr_high, uint32_t addr_low, uint16_t timeout);
uint8_t send_login_msg(uint8_t cmd_type, uint16_t device_id, uint8_t *buffer, uint32_t dest_high, uint32_t dest_low);
bool reset_connection(void);
//bool connected(void);
uint32_t get_addr(uint8_t cmd_type);

void basic_send(uint8_t *data);

#endif  // commutils.h
