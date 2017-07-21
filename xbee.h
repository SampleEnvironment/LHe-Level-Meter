// xbee.h - Copyright 2016, HZB, ILL/SANE & ISIS

#ifndef XBEE_H_
#define XBEE_H_

#include <util/delay.h>

extern volatile uint8_t status_byte;


#define XBEE_SLEEP (\
PORTA |= (1<<PA6);\
_delay_ms(20)\
)

#define XBEE_WAKE_UP (\
PORTA &= ~(1<<PA6);\
_delay_ms(40)\
)

#define XBEE_AWAKE_TIME			30		// Time in seconds

#define SINGLE_FRAME_LENGTH 	50		// Full length of one frame


//==============================================================
// Database server commands
//==============================================================
// Requests with answer sent from the device
#define LOGIN_MSG				1		// Request for login
#define FORCE_LOGIN_MSG			2		// Force login ???

// Requests with answer (or without answer if ALLOW_COM is false) sent from the device
#define LONG_INTERVAL_MSG 		3		// Message contains: Device ID / He, battery and pressure levels / Status
#define OPTIONS_CHANGED_MSG		4		// Send device settings to the database
#define FILLING_BEGIN_MSG		5		// He filling started / Send device position
#define FILLING_END_MSG			7		// He filling ended / Send levels and status
#define LOGOUT_MSG				8		// Send levels and status just before shutting down

// Requests without answer sent from the device
#define FILLING_MSG				6		// Send levels and status while filling
#define XBEE_ACTIVE_MSG			9		// Send device status and notify activity after a sleeping period
#define UNKNOWN_MSG				10		// Unknown command received from database server
#define STATUS_MSG				11		// Send levels and status to the database server
#define GET_OPTIONS_CMD			12		// Send device settings to the database server
#define GET_LETTERS_CMD			14		// Send list of available device positions to the database server
#define GET_PASSWORD_CMD		16		// Send options password to the database server. This password is required to access to the settings pages.
#define GET_XBEE_SLEEP_TIME_CMD	18		// Send XBee sleeping period to the database server

// Requests sent from the database server
#define SET_OPTIONS_CMD			13		// Set device settings received from the database server
#define SET_LETTERS_CMD			15		// Set list of available device positions received from the database server
#define SET_PASSWORD_CMD		17		// Set options password received from the database server. This password is required to access to the settings pages.
#define SET_XBEE_SLEEP_TIME_CMD	19		// Set XBee sleeping period received from the database server

//==============================================================
// XBee commands
//==============================================================
void xbee_sleep(void);			// Set XBee module to sleep mode
void xbee_wake_up(void);			// Wake up XBee module
_Bool xbee_reset_connection(void);		// Reset connection with the xbee coordinator and initiate a new one
										// Returns true if reconnection is successfull, false otherwise
void xbee_reconnect(uint32_t *addr_low, uint32_t *addr_high);			// Try a new connection with the server
void xbee_send(uint8_t *data);			// Start USART0 transmission to XBee module
void xbee_send_msg(uint8_t *buffer, uint8_t length, uint16_t delay);	// Send message via XBee module
uint8_t xbee_get_reply(uint8_t *buffer, uint8_t length, uint8_t db_cmd_type, uint16_t delay);	// Look for reply from database and copy data to the buffer
void xbee_send_request(uint8_t db_cmd_type, uint8_t *buffer, uint8_t length, uint32_t *dest_high, uint32_t *dest_low);	// Send request and receive answer
uint8_t xbee_send_login_msg(uint8_t db_cmd_type, uint16_t device_id, uint8_t *buffer, uint32_t dest_high, uint32_t dest_low);	// Login transaction

void delay_ms(uint16_t period);	 // User defined delay routine (milliseconds)

#endif /* XBEE_H_ */
