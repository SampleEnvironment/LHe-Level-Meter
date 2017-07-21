#include "commutils.h"

void reconnect(uint32_t *addr_low, uint32_t *addr_high)
{
	#ifdef ALLOW_COM
		if(!reset_connection())
		{
			SET_NETWORK_ERROR
			#ifdef ALLOW_DEBUG
				LCD_Print("recon", 5, 5, 2, 1, 1, FGC, BGC); 
			#endif
			return;
		}
		else if (!((*addr_low = get_addr(DL_MSG_TYPE)) && (*addr_high = get_addr(DH_MSG_TYPE)))) 
			 {
				SET_NETWORK_ERROR
				#ifdef ALLOW_DEBUG
					LCD_Print("addr", 5, 20, 2, 1, 1, FGC, BGC); 
				#endif
				return;
			 }
	CLEAR_NETWORK_ERROR
	#endif
}

///send request
void send_request(uint8_t cmd_type, uint8_t *buffer, uint8_t length, uint32_t *dest_high, uint32_t *dest_low)
{
	//XBEE_WAKE_UP
	
	reconnect(dest_low, dest_high);
	
	if(!CHECK_NETWORK_ERROR)
	{
		#ifdef ALLOW_DEBUG
			LCD_Print("sending", 5, 20, 2, 1, 1, FGC, BGC); 
		#endif
		//pack packet
		uint8_t temp_bytes_number = pack_tx64_frame(cmd_type, buffer, length, *dest_high, *dest_low);
		
		if(get_reply(buffer, temp_bytes_number, cmd_type, 1000) == 0xFF)	//request failed!
		{	
			#ifdef ALLOW_DEBUG
				LCD_Print("sending bad", 5, 20, 2, 1, 1, FGC, BGC);
				getkey(); 
			#endif
			SET_NO_REPLY_ERROR
			switch(cmd_type)
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
			#ifdef ALLOW_DEBUG
				LCD_Print("no network", 5, 20, 2, 1, 1, FGC, BGC);
				getkey();
			#endif
			switch(cmd_type)
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

//XBEE_SLEEP
//return replyID if you want to get reply data afterwards
//note, that data copied to buffer anyway, delete memcpy and use frameBuffer[index].data[] instead
}

///calculates checksum
uint8_t getChecksum(uint8_t *buffer, uint8_t start, uint8_t stop)
{
	if (start > stop) return 0;
	
	uint8_t summe = 0;
	uint8_t index = start;
	
	while(index < stop)
	{//add all bytes from start to stop
		summe+=buffer[index++];
	}
	return 0xFF-summe;//result;
}
///returns true if checksum is correct, false otherwise
bool chkChecksum(uint8_t *buffer)
{
	uint8_t index = 0;
	uint8_t summe = 0;
	uint16_t bytes_number = get_packet_len(buffer)+1; //data + checksum
	
	while(bytes_number)
	{//add all bytes after frame length (3.byte)
		summe += buffer[index+3];
		bytes_number--;
		index++;
	}
	return (summe == 0xFF);
}

///packs data (string + int parameters) in a XBEE-frame (TX) ready to send from frame_buffer
///returns number of bytes packed
uint8_t pack_tx64_frame(uint8_t cmd_type, uint8_t *params, uint8_t paramsNumber, uint32_t dest_high, uint32_t dest_low)
{
	uint8_t temp_buffer[SINGLE_FRAME_LENGTH];
	uint8_t index = 0;
	
	uint8_t *temp = params;
	
	temp_buffer[0] = 0x7E;
	//len
	temp_buffer[3] = 0x00;   	//API ID 64 bit TX
	temp_buffer[4] = 0x00;
	
	temp_buffer[5] = (dest_high >> 24);
	temp_buffer[6] = (dest_high >> 16);
	temp_buffer[7] = (dest_high >> 8);
	temp_buffer[8] = (uint8_t) dest_high;
	
	temp_buffer[9] = (dest_low >> 24);
	temp_buffer[10] = (dest_low >> 16);
	temp_buffer[11] = (dest_low >> 8);
	temp_buffer[12] = (uint8_t) dest_low;
	
	temp_buffer[13] = 0x01;		//disable ACK
	temp_buffer[14] = cmd_type;
	
	index = 15;
	//parameter
	while (paramsNumber)
	{
		temp_buffer[index++] = *params++;	
		paramsNumber--;
	}
	
	//calculate checksum
	temp_buffer[index] = getChecksum(temp_buffer,3,index);
	
	//add frame lenght
	temp_buffer[1] = ((index-3) >> 8);
	temp_buffer[2] = (index-3);
	
	memcpy(temp, temp_buffer, index+1);		//frame is in the params array
	
	return index+1;
}

/*//function provided for convenience
uint8_t pack_tx64_frame_no_params(const char *data_buffer, uint8_t *frame_buffer, uint32_t dest_high, uint32_t dest_low)
{
	uint8_t index = 0;
	
	frame_buffer[index++] = 0x7E;
	
	index = 3;
	
	frame_buffer[index++] = 0x00;   	//API ID 64 bit TX
	frame_buffer[index++] = 0x00;
	
	frame_buffer[index++] = (dest_high >> 24);
	frame_buffer[index++] = (dest_high >> 16);
	frame_buffer[index++] = (dest_high >> 8);
	frame_buffer[index++] = (uint8_t) dest_high;
	
	frame_buffer[index++] = (dest_low >> 24);
	frame_buffer[index++] = (dest_low >> 16);
	frame_buffer[index++] = (dest_low >> 8);
	frame_buffer[index++] = (uint8_t) dest_low;
	
	frame_buffer[index++] = 0x01;		//disable ACK
	
	while (*data_buffer!='\0')
	{
		frame_buffer[index++] = *data_buffer++;	
	}
	
	frame_buffer[index] = getChecksum(frame_buffer,3,index-1);
	
	frame_buffer[1] = ((index-3) >> 8);
	frame_buffer[2] = (index-3);
	
	return index;
}*/

///packs data (string + int parameters) in a XBEE-frame (AT) ready to send from frame_buffer
///returns number of bytes packed
uint8_t pack_at_frame(uint8_t *params, uint8_t paramsNumber)
{
	uint8_t temp_buffer[SINGLE_FRAME_LENGTH];
	uint8_t index = 5;
	
	uint8_t *temp = params;

	temp_buffer[0] = 0x7E;
	//len 
	temp_buffer[3] = 0x08;   	//API ID AT CMD
	temp_buffer[4] = 0x42;		//constant frame ID
	
	while (paramsNumber)
	{
		temp_buffer[index++] = *params++;
		paramsNumber--;
	}
	
	temp_buffer[index] = getChecksum(temp_buffer,3,index);
	
	temp_buffer[1] = ((index-3) >> 8);
	temp_buffer[2] = (index-3);
	
	memcpy(temp, temp_buffer, index+1);		//frame is in the params array
		
	return index+1;
}
//function provided for convenience
//when using this function you will able to send the data properly only once
//pure data passed in frame_buffer will be packed in frame_buffer

/*uint8_t pack_at_frame2(uint8_t *frame_buffer)
{
	uint8_t temp_buffer[50];
	uint8_t *temp_ptr;
	memcpy(temp_buffer,frame_buffer,50);
	temp_ptr = temp_buffer;
	
	uint8_t index = 0;
	frame_buffer[index++] = 0x7E;
	
	index = 3;
	
	frame_buffer[index++] = 0x08;   	//API ID AT CMD
	frame_buffer[index++] = 0x42;		//const Frame ID always x42
	
	while (*temp_ptr!=0x7E)
	{
		frame_buffer[index++] = *temp_ptr++;	
	}
	frame_buffer[index] = getChecksum(frame_buffer,3,index-1);
	
	frame_buffer[1] = ((index-3) >> 8);
	frame_buffer[2] = (index-3);
	
	return index;
}*/
/*//clears passed array
void clear_buffer(uint8_t *buffer)
{
	memset(buffer,0x00,arr_size(buffer));
}*/

///returns the lenght of the passed XBEE-frame
uint8_t get_packet_len(uint8_t *buffer)
{
	return (buffer[1]<<8) + buffer[2];
}

uint8_t hasReply(uint8_t cmd_type, uint8_t range)
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
		case LESS_THEN:
					for(uint8_t i = 0; i < bufferSize; ++i)
					{	
						if((frameBuffer[i].status == 0x00) && (frameBuffer[i].type < cmd_type))
						{
							return i;
						}
					}
					break;
		case GREATER_THEN:
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
///returns true if reconncetion was successfull, false otherwise
bool reset_connection(void)
{	
//	uint8_t trial_count = 10;
	uint8_t buffer[SINGLE_FRAME_LENGTH];  		// put DA here then send
	
	buffer[0] = (uint8_t)'D';
	buffer[1] = (uint8_t)'A';
	//pack frame according to type
	uint8_t temp_bytes_number = pack_at_frame(buffer, 2);  	// add DA
	
	if (get_reply(buffer, temp_bytes_number, DA_MSG_TYPE, 1000) == 0xFF)
		return 0;											// couldn't disassociate (= no reply from xbee on my request)
	//dont need any data from DA so nothing else happens

	/*/disassociate from network
	set_timeout(COM_TIMEOUT_TIME, TIMER_3, USE_TIMER);
	while(!send_and_wait(DA_MSG_TYPE, buffer, 0, 1, 1, 2000))
	{
		if(!set_timeout(0, TIMER_3, USE_TIMER))	return 0;	//stop trying on timeout
	}
	set_timeout(0, TIMER_3, RESET_TIMER);*/
	
	//LCD_Cls(blue);
	//getkey();
	
	
	//pack frame according to type
	temp_bytes_number = pack_at_frame(buffer, 0);
	
	//wait for status = SUCCESS; length 0 - do not send anything
	if (get_reply(buffer, 0, STATUS_MSG_TYPE, 1000) == 0xFF)  	// couldn't get status = SUCCESS from xbee on my request)
		return 0;												
	//dont need any data from STA so nothing else happens 
	//(note that status SUCCESS = 2 is mapped to status 0 (general okay status) while building the frame)
	return 1; //recconect successfull
}

//returns true if Module is connected, false otherwise
/*bool connected(void)
{
	uint8_t send_buffer[50];
	uint8_t dummy[1] = {END};
	
	//set_timeout(COM_TIMEOUT_TIME, TIMER_3, USE_TIMER);
	if(!send_and_wait("AI",dummy, send_buffer, AT,1,1,1000)) 
	{
		return 1;
	}
	else {
		get_data64(send_buffer, AT);
		switch(send_buffer[0])		
		{	
			case 0x00: 	return 1;
			
			case 0x13:	return 0;
			
			default:	return 0;
		}
	}	
}*/

///gets high and low bytes of basis station
///used only with "DL" and "DH" cmd_type
uint32_t get_addr(uint8_t cmd_type)
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
	
	//pack frame according to type
	uint8_t temp_bytes_number = pack_at_frame(send_buffer, 2);  //add DL or DH
	
	if (get_reply(send_buffer, temp_bytes_number, cmd_type, 500) != 0xFF)
	{
		/*uint8_t bytes_count = *///get_data64(send_buffer, AT);
		/*draw_int(bytes_count, 80,2, "", red);
		
		draw_int(send_buffer[0], 80,20, "", red);
		draw_int(send_buffer[1], 80,40, "", red);
		draw_int(send_buffer[2], 80,60, "", red);
		draw_int(send_buffer[3], 80,80, "", red);
		
		delay_ms(300);	
		getkey();
		*/
		dest_addr = (unsigned long int)send_buffer[0]<<24;
		dest_addr += (unsigned long int)send_buffer[1]<<16;
		dest_addr += (unsigned long int)send_buffer[2]<<8;
		dest_addr += (unsigned long int)send_buffer[3];
		
		return dest_addr;
	}
	else return 0;		//coundn't read addr_high or addr_low
}

///login transaction 
uint8_t send_login_msg(uint8_t cmd_type, uint16_t device_id, uint8_t *buffer, uint32_t dest_high, uint32_t dest_low)
{
	#ifdef ALLOW_LOGIN
		//char *cmd_str;
		uint8_t reply_Id = 0;
		
		LCD_Cls(BGC);
		LCD_Print("connecting", 5,40,2,1,1, FGC, BGC);
		LCD_Print("to server...", 5,60,2,1,1, FGC, BGC);
		
		//pack entered device number in buffer
		buffer[0] = device_id >> 8;
		buffer[1] = device_id;
		
		//pack packet
		uint8_t temp_bytes_number = pack_tx64_frame(cmd_type, buffer, 2, dest_high, dest_low);
		
		//try to send login message "number_trials" times
		uint8_t number_trials = 3;
		
		while(number_trials)
		{
			reply_Id = get_reply(buffer, temp_bytes_number, cmd_type, 2000);
			
			if(reply_Id != 0xFF)
			{
				if(frameBuffer[reply_Id].data_len == NUMBER_OPTIONS_BYTES) 	return 0;	//good options
				else {
						if(frameBuffer[reply_Id].data[0] == 0x56)	return 2;	//bei 0x56="V" kanne unknown			
																	return 1;	//bad options
				}
			}
			
			if(!(--number_trials)) 
			{//stop trying and go in error mode; no functionality available from here on
				return 0xFF;
			}
			LCD_Cls(BGC);
			LCD_Print("Login", 5, 40, 2, 1, 1, FGC, BGC);
			LCD_Print("failed!", 5, 60, 2, 1, 1, ERR, BGC);
			LCD_Print("press any key to try again", 2, 100, 1, 1, 1, FGC, BGC);
			
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
				delay_ms(500);
				if(keyhit() == HIDDEN_FUNCTION)
				{
					//mode = MODE_OFFLINE;
					//err_code = OFFLINE;
					//break;
					return 3;
				}
			}
			LCD_Cls(BGC);
			LCD_Print("logging in...", 5,50,2,1,1, FGC, BGC);
		}//while(number_trials)	
		return 0xFF;
	#else
		return 1;	//bad options /main will set default
	#endif
}



///start sending 
uint8_t get_reply(uint8_t *buffer, uint8_t length, uint8_t cmd_type, uint16_t delay)
{
	uint8_t reply_Id = 0xFF;
	
	set_timeout(0, TIMER_3, RESET_TIMER);	//reset timer!
	set_timeout(COM_TIMEOUT_TIME, TIMER_3, USE_TIMER);
	while(1)
	{
		if(!set_timeout(0,TIMER_3, USE_TIMER)) 
		{
			break;			//stop trying on timeout return bad reply
		}
		//check for reply
		reply_Id = hasReply(cmd_type, EQUAL);
		
		if (reply_Id == 0xFF)
		{//no reply - send again - wait delay ms
			send_MSG(buffer, length, delay);
		}
		else {//valid reply - mark as read - finish
			frameBuffer[reply_Id].status = 0xFF;											//mark as read
			memcpy(buffer, frameBuffer[reply_Id].data, frameBuffer[reply_Id].data_len); 	//copy data to buffer
			return reply_Id;
		}
	}
	return 0xFF;
}

///send stuff
void send_MSG(uint8_t *buffer, uint8_t length, uint16_t delay)
{
	if (!length) return;
	
	sending_cmd = length;		//bytes number to send
	basic_send(buffer);			//send first
	
	delay_ms(delay);			//wait delay in ms
}

///starts USART RS232 transmission 
void basic_send(uint8_t *data)
{
	send_str_reader = data;						//point to data
	//draw_int(*send_str_reader,65,5,"",blue);	
	
	COM = *send_str_reader++;					//send first data byte (interrupt will do the rest)
	sending_cmd--;								//1 byte is sent, so decrease counter
}

