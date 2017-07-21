// xbee.c - Copyright 2016, HZB, ILL/SANE & ISIS
#define RELEASE_XBEE 1.00

// HISTORY -------------------------------------------------------------
// 1.00 - First release on May 2016

//#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "display_utilities.h"
#include "keyboard.h"
#include "main.h"
#include "timer_utilities.h"
#include "usart.h"
#include "xbee_utilities.h"
#include "xbee.h"

// Set XBee module to sleep mode
inline void xbee_sleep(void)
{
	#ifdef ALLOW_XBEE_SLEEP				// Right defined in main.c
		PORTA|=(1<<PA6);				// Set PA6 as true
		_delay_ms(20);
	#endif
}

// Wake up XBee module
inline void xbee_wake_up(void)
{
	#ifdef ALLOW_XBEE_SLEEP				// Right defined in main.c
		PORTA&=~(1<<PA6);				// Set PA6 as false
		_delay_ms(30);
	#endif
}

// Reset connection with the xbee coordinator and initiate a new one
// Returns true if reconnection is successfull, false otherwise
_Bool xbee_reset_connection(void)
{	
	uint8_t buffer[SINGLE_FRAME_LENGTH];  		// put DA here then send
	
	// API command "DA" => Force Disassociation.
	// End Device will immediately disassociate from a Coordinator (if associated) and reattempt to associate.
	buffer[0] = (uint8_t)'D';
	buffer[1] = (uint8_t)'A';
	uint8_t temp_bytes_number = xbee_pack_tx_frame(buffer, 2);  	// Pack API "DA" command
	
	// Send packed command to the coordinator in order to check the connection
	if (xbee_get_reply(buffer, temp_bytes_number, DA_MSG_TYPE, 1000) == 0xFF)
		return 0;											// couldn't disassociate (= no reply from xbee on my request)
	//dont need any data from DA so nothing else happens
	
	//pack frame according to type
	temp_bytes_number = xbee_pack_tx_frame(buffer, 0);
	
	//wait for status = SUCCESS; length 0 - do not send anything
	if (xbee_get_reply(buffer, 0, STATUS_MSG_TYPE, 1000) == 0xFF)  	// couldn't get status = SUCCESS from xbee on my request)
		return 0;												
	//dont need any data from STA so nothing else happens 
	//(note that status SUCCESS = 2 is mapped to status 0 (general okay status) while building the frame)

	return 1; // Successfully connected
}

// Try a new connection with the server
void xbee_reconnect(uint32_t *addr_low, uint32_t *addr_high)
{
	#ifdef ALLOW_COM
		if(!xbee_reset_connection())
		{
			SET_NETWORK_ERROR
			#ifdef ALLOW_DEBUG
				LCD_Print("recon", 5, 5, 2, 1, 1, FGC, BGC); 
			#endif
			return;
		}
		else if (!((*addr_low = xbee_get_addr(DL_MSG_TYPE)) && (*addr_high = xbee_get_addr(DH_MSG_TYPE)))) 
		{
			SET_NETWORK_ERROR
			#ifdef ALLOW_DEBUG
				LCD_Print("addr", 5, 20, 2, 1, 1, FGC, BGC); 
			#endif
			return;
		}
	CLEAR_NETWORK_ERROR		// Successfully reconnected, clear network error
	#endif
}

// Start USART0 transmission to XBee module
void xbee_send(uint8_t *data)
{
	send_str_reader = data;						// point to data
	
	USART_IODR = *send_str_reader++;			// Send first data byte (ISR_Tx will do the rest)
	sending_cmd--;								// 1 byte is sent, so decrease counter
}

// Send message via XBee module
void xbee_send_msg(uint8_t *buffer, uint8_t length, uint16_t delay)
{
	if (!length) return;
	
	sending_cmd = length;		//bytes number to send
	xbee_send(buffer);			//send first
	
	delay_ms(delay);			//wait delay in ms
}

// Look for reply from database and copy data to the buffer
uint8_t xbee_get_reply(uint8_t *buffer, uint8_t length, uint8_t db_cmd_type, uint16_t delay)
{
	uint8_t reply_Id = 0xFF;
	
	set_timeout(0, TIMER_3, RESET_TIMER);	//reset timer!
	set_timeout(COM_TIMEOUT_TIME, TIMER_3, USE_TIMER);

//	xbee_send_msg(buffer, length, delay);

	while(1)
	{
		if(!set_timeout(0,TIMER_3, USE_TIMER)) 
		{
			break;			//stop trying on timeout return bad reply
		}
		// Check for reply
		reply_Id = xbee_hasReply(db_cmd_type, EQUAL);
		
		//if (reply_Id != 0xFF)
		//{//valid reply - mark as read - finish
			//frameBuffer[reply_Id].status = 0xFF;											//mark as read
			//memcpy(buffer, frameBuffer[reply_Id].data, frameBuffer[reply_Id].data_len); 	//copy data to buffer
			//return reply_Id;
		//}

		if (reply_Id == 0xFF)
		{//no reply - send again - wait delay ms
			xbee_send_msg(buffer, length, delay);
		}
		else
		{//valid reply - mark as read - finish
			frameBuffer[reply_Id].status = 0xFF;											//mark as read
			memcpy(buffer, frameBuffer[reply_Id].data, frameBuffer[reply_Id].data_len); 	//copy data to buffer
			return reply_Id;
		}
	}
	return 0xFF;
}

// Send request and receive answer
void xbee_send_request(uint8_t db_cmd_type, uint8_t *buffer, uint8_t length, uint32_t *dest_high, uint32_t *dest_low)
{
	xbee_reconnect(dest_low, dest_high);
	
	if(!CHECK_NETWORK_ERROR)
	{
		// Any network error
		#ifdef ALLOW_DEBUG
			LCD_Print("Sending ...", 5, 20, 2, 1, 1, FGC, BGC); 
		#endif
		//pack packet
		uint8_t temp_bytes_number = xbee_pack_tx64_frame(db_cmd_type, buffer, length, *dest_high, *dest_low);
		
		if(xbee_get_reply(buffer, temp_bytes_number, db_cmd_type, 1000) == 0xFF)	//request failed!
		{	
			#ifdef ALLOW_DEBUG
				LCD_Print("sending bad", 5, 20, 2, 1, 1, FGC, BGC);
				getkey(); 
			#endif
			SET_NO_REPLY_ERROR
			switch(db_cmd_type)
			{
				case FILLING_BEGIN_MSG:
							SET_STARTED_FILLING_ERROR
							break;
				case FILLING_END_MSG:
							SET_STOPED_FILLING_ERROR
							break;
				case OPTIONS_CHANGED_MSG: 
							SET_CHANGED_OPTIONS_ERROR
							break;
				case LONG_INTERVAL_MSG:
							SET_SLOW_TRANSMISSION_ERROR
							break;
			}										
		}
		else {
			#ifdef ALLOW_DEBUG
				LCD_Print("sending  ok", 5, 20, 2, 1, 1, FGC, BGC); 
			#endif
			status_byte = 0;
		}
	}
	else {	
			// Network error occurred
			#ifdef ALLOW_DEBUG
				LCD_Print("no network", 5, 20, 2, 1, 1, FGC, BGC);
				getkey();
			#endif
			switch(db_cmd_type)
			{
				case FILLING_BEGIN_MSG:
							SET_STARTED_FILLING_ERROR
							break;
				case FILLING_END_MSG:
							SET_STOPED_FILLING_ERROR
							break;
				case OPTIONS_CHANGED_MSG: 
							SET_CHANGED_OPTIONS_ERROR
							break;
				case LONG_INTERVAL_MSG:
							SET_SLOW_TRANSMISSION_ERROR
							break;
			}
	}

//return replyID if you want to get reply data afterwards
//note, that data copied to buffer anyway, delete memcpy and use frameBuffer[index].data[] instead
}

// Login transaction 
uint8_t xbee_send_login_msg(uint8_t db_cmd_type, uint16_t device_id, uint8_t *buffer, uint32_t dest_high, uint32_t dest_low)
{
	#ifdef ALLOW_LOGIN
		uint8_t reply_Id = 0;
		
		xbee_wake_up();
		
		LCD_Cls(BGC);
		LCD_Print("Connecting"  , 5, 40, 2, 1, 1, FGC, BGC);
		LCD_Print("to server...", 5, 60, 2, 1, 1, FGC, BGC);
		
		// Pack entered device number in buffer
		buffer[0] = device_id >> 8;
		buffer[1] = device_id;
		
		// Pack frame
		uint8_t temp_bytes_number = xbee_pack_tx64_frame(db_cmd_type, buffer, 2, dest_high, dest_low);
		
		// Try to send login message "number_trials" times
		uint8_t number_trials = 1;
		
		while(number_trials)
		{
			reply_Id = xbee_get_reply(buffer, temp_bytes_number, db_cmd_type, 2000);
			
			if(reply_Id != 0xFF)
			{
				if(frameBuffer[reply_Id].data_len == NUMBER_OPTIONS_BYTES) 
					return 0;	//good options
				else
				{
					if(frameBuffer[reply_Id].data[0] == 0x56)
						return 2;	//bei 0x56="V" kanne unknown			
					
					return 1;	//bad options
				}
			}
			
			if(!(--number_trials)) 
			{	
				//stop trying and go in error mode; no functionality available from here on
				xbee_sleep(); // Stop XBee module
				return 0xFF;
			}
			LCD_Cls(BGC);
			LCD_Print("Login", 5, 40, 2, 1, 1, FGC, BGC);
			LCD_Print("failed!", 5, 60, 2, 1, 1, ERR, BGC);
			LCD_Print("Press any key to try again", 2, 100, 1, 1, 1, FGC, BGC);
			
			getkey();
			set_timeout(200, TIMER_0, USE_TIMER);
			while(keyhit() != HIDDEN_FUNCTION)	
			{
				if(!set_timeout(0,TIMER_0, USE_TIMER)) 
				{
					break;	//stop trying on timeout ERROR!
				}
			}
			if(set_timeout(0,TIMER_0, USE_TIMER)) 
			{
				_delay_ms(500);
				if(keyhit() == HIDDEN_FUNCTION)
				{
					//mode = MODE_OFFLINE;
					//err_code = OFFLINE_errCode;
					//break;
					return 3;
				}
			}
			LCD_Cls(BGC);
			LCD_Print("Logging in...", 5,50,2,1,1, FGC, BGC);
		}
		return 0xFF;
	#else
		return 1;	//bad options /main will set default
	#endif
}

void delay_ms(uint16_t period)	 //delay routine (milliseconds)
{
	for(unsigned int i=0; i<=period; i++)
	_delay_ms(1);
}
