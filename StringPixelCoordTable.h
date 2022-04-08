// String and Pixel Coord Table

#define LCD_WIDTH 176

#define XOFFSET_32 32

#define FONT1_W 6
#define FONT1_H 9


#define FONT2_W 9
#define FONT2_H 15

#define FONT3_W 9
#define FONT3_H 15

//Font Params //TODO use Values that are defined by the Font
#define CHAR_CELL_WIDTH_FONT_1 6
#define HALF_SPACE_WIDTH_FONT_1 3
#define CHAR_CELL_WIDTH_FONT_2 9
#define HALF_SPACE_WIDTH_FONT_2 3


#define CENTER_LINE (LCD_WIDTH-XOFFSET_32)/2

//InitScreen_AddLine()
#define IAdd_Line_Max_Lines 12
#define IAdd_Line_LineFeed  10
#define X_IA_2 2 

//main()
#define  FONTNR_M_1 1

#define X_M_0 0
#define X_M_1 1
#define X_M_2 2
#define X_M_5 5
#define X_M_40 40
#define X_M_50 50
#define X_M_60 60
#define X_M_85 85
#define X_M_100 100
#define X_M_150 150

#define Y_M_5 5
#define Y_M_20 20
#define Y_M_40 40
#define Y_M_40_NOTICE 40
#define Y_M_57 57
#define Y_M_66 66
#define Y_M_60 60
#define Y_M_80 80
#define Y_M_90 90
#define Y_M_100 100
#define Y_M_105 105


//Koords paint_enter_number()
#define SPACE_FAC_PE FONT2_W + 5
#define FONT_W_PE FONT2_W
#define FONT_H_PE FONT2_H
#define SCALE_PE_2 2
#define FONTNR_PE_2 2

#define X_PE_4     1
#define X_PE_5	   5
#define X_PE_30   30
#define X_PE_42   42
#define X_PE_60   60
#define X_PE_90   90

#define Y_PE_2     2
#define Y_PE_15   15
#define Y_PE_35   35
#define Y_PE_40   40
#define Y_PE_70   70




//Koords LCD_Dialog()
#define LD_Line_Height  10
#define LD_MAX_Height  110
#define LD_Line_Length 160
#define LD_HALF_CHAR_WIDTH 4
#define LD_MID_LANDSCAPE 87
#define LD_MID_LANDSCAPE180 94

//Text Box Left Edge
#define X_LD_3      3
#define X_LD_10    10
#define X_LD_13    13
#define X_LD_20    20
#define X_LD_17    17
#define X_LD_120  120
#define X_LD_160  160
#define X_LD_165  165
#define X_LD_166  166

#define Y_LD_1      1
#define Y_LD_2      2
#define Y_LD_17    17
#define Y_LD_25    25
#define Y_LD_28    28
#define Y_LD_33	   33
#define Y_LD_99    99
#define Y_LD_115  115
#define Y_LD_120  120
#define Y_LD_165  165


//paint_progress_bar()
#define  Percentage_Font_Cellwidth CHAR_CELL_WIDTH_FONT_1
#define  FONT_PPB_1 1

#define X_PPB_OFFS_2  2
#define X_PPB_OFFS_8  8
#define X_PPB_OFFS_10 10
#define X_PPB_OFFS_100  100
#define X_PPB_OFFS_103  103

#define Y_PPB_OFFS_2  2
#define Y_PPB_OFFS_5  5
#define Y_PPB_OFFS_1  1
#define Y_PPB_OFFS_7  7

//get_he_level()
#define X_GHL_5  5
#define Y_GHL_105 105
#define Y_GHL_90 90

//paint_he_level()
#define FONTNR_PHL_2 2
#define SCALE_PHL_4 4
#define X_PHL_1 1
#define X_PHL_2 2
#define X_PHL_5 5
#define X_PHL_10 10
#define X_PHL_25 25

#define X_PHL_L_OFFSET 0 		  // "l" offset to the left

#define X_PHL_85 85
#define X_PHL_93 93


#define Y_PHL_37 37
#define Y_PHL_61 61
#define Y_PHL_90 90
#define Y_PHL_100 100

//paint_buttons()
#define FONTNR_PB_1 1


#define LEN_PB_26 26
#define LEN_PB_30 30
#define LEN_PB_129 129
#define X_PB_Box_0 0
#define Y_PB_Box_0 0
#define X_PB_0 0
#define X_PB_1 1
#define X_PB_3 3
#define X_PB_6 6
#define X_PB_7 7
#define X_PB_15 15 +3
#define X_PB_15a 15
#define X_PB_17 17
#define X_PB_30 30
#define X_PB_32 32
#define X_PB_144 144
#define X_PB_145 145
#define X_PB_150 150
#define X_PB_151 151
#define X_PB_159 159 +3
#define X_PB_159a 159
#define X_PB_161 161
#define X_PB_174 174
#define X_PB_176 176

#define Y_PB_0 0
#define Y_PB_1 1
#define Y_PB_5 5
#define Y_PB_25 25
#define Y_PB_26 26
#define Y_PB_52 52
#define Y_PB_61 61
#define Y_PB_77 77
#define Y_PB_78 78
#define Y_PB_104 104
#define Y_PB_109 109
#define Y_PB_130 130
#define Y_PB_131 131



//paint_batt
#define  X_PBATT_1 1
#define  X_PBATT_38 38
#define  X_PBATT_39 39
#define  X_PBATT_140 140

#define  Y_PBATT_118 118
#define  Y_PBATT_119 119
#define  Y_PBATT_120 120
#define  Y_PBATT_121 121
#define  Y_PBATT_129 129
#define  Y_PBATT_130 130
#define  Y_PBATT_131 131

//paint_info_line()
#define X_PIL_2 2
#define Y_PIL_90 90

//paint_main()
#define FONTNR_PM_1 1
#define AUTOFILL_BOX_WIDTH 8

#define X_PM_1 1
#define X_PM_4 4
#define X_PM_25 25
#define X_PM_65 65
#define X_PM_100 100
#define X_PM_148 148
#define X_AUTOFFILL_INDICATOR  (LCD_WIDTH-(XOFFSET_32+AUTOFILL_BOX_WIDTH+4))


#define Y_PM_2 2
#define Y_PM_37 37
#define Y_PM_83 83
#define Y_AUTOFFILL_INDICATOR Y_PTP_20 + 4

//paint_time_pressure()
#define X_PTP_2 2
#define X_PTP_60 55

#define Y_PTP_20 20




//paint_start_filling()

#define OFFS_PSF_20 20

#define  X_PSF_5 5
#define  X_PSF_30 30
#define  X_PSF_40 40
#define  X_PSF_52 52

#define Y_PSF_2 2
#define Y_PSF_35 35
#define Y_PSF_70 70


//update_filling_pos()
#define  X_PSF_zero (((LCD_WIDTH-XOFFSET_32)/2)-5)
#define X_PSF_30 30 
#define X_PSF_40 40

#define Y_PSF_70 70

//paint_filling()
#define  X_PF_1 1
#define  X_PF_65 65
#define  X_PF_100 100


#define  Y_PF_2 2

//paint_options() & paint_current_opt_page#()
#define X_PO_2 2
#define X_PO_25 25
#define X_PO_85 89
#define X_PO_120 140


#define Y_PO_2 2
#define Y_PO_20 20
#define Y_PO_40 40
#define Y_PO_60 60
#define Y_PO_80 80
#define Y_PO_100 100

//paint_opt_values_page#()
#define X_PO_85 89
#define X_POVP_76 85

#define Y_PO_20 20
#define Y_PO_40 40
#define Y_PO_60 60
#define Y_PO_80 80
#define Y_PO_100 100


//paint_diag()
#define FONTNR_PD_1 1

#define X_PD_4a 4
#define X_PD_4b 4
#define X_PD_2 2
#define X_PD_20 20
#define X_PD_35 35
#define X_PD_40 40
#define X_PD_50 50
#define X_PD_90 90
#define X_PD_148a 148
#define X_PD_148b 148

#define Y_PD_2 2
#define Y_PD_20 20
#define Y_PD_31 31
#define Y_PD_40 40
#define Y_PD_60 60
#define Y_PD_80 80
#define Y_PD_82 82
#define Y_PD_100 100 


//diag_pulse()
#define DP_NUMBER_OF_POINTS_140 140
#define DP_POINTS_IN_PULSE_80 80 
#define DP_U_ZEROLINE_70 70
#define DP_I_ZEROLINE_125 125
#define DP_Y_MAX_PIXELS_50 50
#define DP_PADDING_10 2
#define DP_X_FACTOR 1
#define DP_AXIS_FONTNR 1
#define DP_SEND_FONTNR 1
#define DP_SEND_YFACTOR 2


#define X_DP_1 3//1
#define X_DP_2 2
#define X_DP_2_SEND X_DP_2
#define X_DP_3 3

#define X_DP_20 X_DP_2 
#define X_DP_25 25
#define X_DP_60 50
#define X_DP_60_AVG X_DP_60
#define X_DP_60_NORMAL X_DP_60
#define X_DP_75 97
#define X_DP_147 147
#define X_DP_147_SEND X_DP_147

#define Y_DP_2 2
#define Y_DP_6 6
#define Y_DP_OFFS 3
#define Y_DP_cursor 20
#define Y_DP_31 31
#define Y_DP_40 40
#define Y_DP_71 71
#define Y_DP_85 85
#define Y_DP_100 100
#define Y_DP_126 126


//======================================================================================
//Pulse Select
//======================================================================================
#define PLOT_X 130
#define PLOT_Y 15

#define X_AXIS_LEN 20
#define Y_AXIS_LEN 12



#define X_t_UNIT 1
#define X_I_UNIT 7


#define Y_t_UNIT 2

#define Y_COODRS_NORMAL uint16_t Ypoints[9] = {0, 5,10,11,10,7,6,6,6 };
#define X_COORDS_NORMAL 8

#define Y_COODRS_CONST 8

#define Y_COODRS_LINEAR 2



//===============================================================================
//-----INIT-Screen---------------------------------------------------------------
//===============================================================================
#define STR_HZB_LEVELMETER             "Levelmeter"
#define STR_FIRMWARE_VERSION           "Build "
#define STR_INIT_I2C_TIMER             "Init I2C Timer"
#define STR_SUCCESSFUL                 "...successful"
#define STR_UN_SUCCESSFUL              "...not successful"
#define STR_INIT_I2C_PRESSURE          "Init I2C Press"
#define STR_DEFECT_PRESSURE_SENSOR     "Pressure sensor defect!"
#define STR_NO_PRESSURE_SENSOR         "No pressure sensor!"


#define STR_OFFLINE_MODE               "OFFLINE MODE"
#define STR_NETWORK_CONN               "Network connection"
#define STR_IN_PROGRESS                "in progress..."
#define STR_FAILED_DOT					   "...failed!"
#define STR_PRESS_KEY_TO_TRY_AGAIN	   "Press any key to try again"
#define STR_SUCCEEDED                  "...succeeded"

//===============================================================================
//-----Device Login--------------------------------------------------------------
//===============================================================================

// Device ID
#define STR_CONNECTION   "Connection"
#define STR_TO_SERVER	"to server"
#define STR_ENTER_VESSEL_ID "Enter Vessel ID:"
#define STR_EXIT "exit"
#define STR_OK   "ok"
#define STR_CANCEL "canc"
#define STR_0  "0"
#define STR_DO_YOU_REALLY_WANT_SHUTDOWN "Do you really\nwant to shut down\nthe system?"
#define STR_SHUTDOWN  "Shutdown:"
#define STR_DO_YOU_WANT_TO_CONNECT_VESSEL_TO_SERVER "Do you really want\nto connect vessel %s\nto server?"
#define STR_SERVER "Server"
#define STR_SHUTDOWN_NOW "Shutting down now"
#define STR_NO_DEVICE_ENTERED "No Device ID entered"

//xbee_send_login_msg()
#define STR_CONNECTING_TO_SERVER "Connecting to server..."
#define STR_DATE_RECEIVED "...data received"
#define STR_LOGIN_FAILED  "Login failed!"


#define STR_GOOD_OPTIONS "Good options"
#define STR_NEW_DATE "new date: %i.%i.%i"
#define STR_NEW_TIME "new time: %i:%i:%i"
#define STR_BAD_OPTS_RECEIVED "BAD options received!"
#define STR_DEFAILT_OPTS_SET "Default Options set"

#define STR_VESSEL_WITH_ID_NOT_VALID "Vessel with ID\n%s parameters not valid,\nstill login?"
#define STR_VESSEL_NOT_ON_CAMPUS "Vessel with ID\n%s not on campus,\nstill login?"
#define STR_VESSEL_ALREADY_CONNECTED "Vessel with ID\n%s is already connected,\nstill login?"
#define STR_VESSEL_WITH_ID_NOT_FOUND "No vessel with ID\n%s found,\nstill login?"
#define STR_LEVEL_METER_NOT_KNOWN "Level meter not known!"

//===============================================================================
//-----Initial Setup-------------------------------------------------------------
//===============================================================================

#define STR_1ST_MEASUREMENT    "1st measurement"
#define STR_CHECK_NETWORK      "Check network..."
#define STR_NETWORK_ERROR      "Network error"
#define STR_NETWORK_ERROR_ADDR "Network err.addr"
#define STR_SENDING            "Sending..."
#define STR_SENDING_ERROR      "Sending error"
#define STR_SENDING_OK         "Sending  ok"
#define STR_NO_NETWORK		   "...no network"
#define STR_DATA_STORED_N      "Data stored (%d)"
#define STR_RECONNECTING       "reconnecting..."
#define STR_SENDING_MESSAGE    "sending message..."
#define STR_VESSEL_NR          "Vessel#" 

// paint_main()
#define STR_Conn   "Conn"

//paint_batt()
#define STR_BATT   "Batt"

//paint_offline()
#define STR_OFF "OFF"

#define STR_EEPROM_UNDEFINED "EEPROM undef."
#define STR_VALUES_MIGHT_BE_UNDEFINED "Values stored in EEPROM\nmight be undefined,\nplease recalibrate!\nPress YES to not show\nthis message again."

//===============================================================================
//-----Main loop-------------------------------------------------------------
//===============================================================================

#define STR_MEASURING "Measuring..."

//update_filling_pos()

//paint_start_filling()
#define STR_FILL "Fill"
#define STR_ENTER_POS "Enter Position"


#define STR_SENDING_INFO "Sending info"
#define STR_START_FILLING "start filling"
#define STR_TO_THE_SERVER "to the server..."
#define STR_AWAKE_MESSAGE_SENT "Awake message sent"
#define STR_TO_SERVER_DOT "to server..."
#define STR_NEW_TIME_RECEIVED "new time received"
#define STR_STATUS "status: %i"
#define STR_TRANSMITTING_STORED "Transmitting stored"
#define STR_MEASUREMENTS "measurements..."
#define STR_REMAINING "remaining: %d"




// paint_options()
#define  STR_off "off"

#define STR_OPTIONS_1of6 "Options 1/6"
#define STR_OPTIONS_2of6 "Options 2/6"
#define STR_OPTIONS_3of6 "Options 3/6"
#define STR_OPTIONS_4of6 "Options 4/6"
#define STR_OPTIONS_5of6 "Options 5/6"
#define STR_OPTIONS_6of6 "Options 6/6"

//options strings 9 chars only!
//page 1
#define STR_SHUTDOWN_OPT 		   "Shutdown:"
#define STR_POS 					"Pos:"
#define STR_AUTOFILL 				"Autofill:"
#define STR_HE_MIN_LVL 				"He.Min:"
#define STR_FILLING_TIMEOUT 		"Timeout:"

//page 2
#define STR_DIAG 					"Diagmode:"
#define STR_QUENCH_CURRENT 			"I quench:"
#define STR_HEAT_TIME 				"Pulse:"
#define STR_MEAS_CURRENT 			"I meas:"
#define STR_WAIT_TIME 				"Wait:"

//page 3
#define STR_RES_MIN 				"R.Min:"
#define STR_RES_MAX 				"R.Max:"
#define STR_ENABLE_PR				"Enable:"
#define STR_SPAN					"Pr.Span:"
#define STR_ZERO					"Pr.Zero:"

//page 4
#define STR_TRANSMIT_SLOW 			"Int.Slow:"
#define STR_TRANSMIT_FAST 			"Int.Fast:"
#define STR_BATTMIN					"Bat.Min:"
#define STR_BATTMAX					"Bat.Max:"
#define STR_CRITVOLT				"Bat.Low:"

//page 5
#define STR_ADCSPAN					"R.Span:"
#define STR_ADCZERO 				"R.Zero:"
#define STR_MEASUREMENT_CYCLES 		"ADCcycs:"
#define STR_TOTAL_VOL 				"Volume:"
#define STR_FLIP_DISP 				"FlipDisp:"

//page 6
#define STR_ALPHANUM					"Alphanum:"
#define STR_NLOGINCHARS 				"ID len:"
//-----------------

//pulse_select_Mode-----------------
#define STR_PULSE_TYPE "Pulse:"

#define STR_PULSE_DURATION "Duration:"
#define STR_PULSE_CURRENT "Current:"

#define STR_PULSE_DELTA_I "Delta I:"
#define STR_PULSE_I_MIN "I Start:"
#define STR_PULSE_I_MAX "I End:"
#define STR_PULSE_DELTA_t "Delta t:"



#define STR_SHOULD_NOT_HAPPEN "should not happen"
#define STR_FILLING "Filling"
#define STR_DO_YOU_REALLY_WANT_TO_STOP_FILLING "Do you really\nwant to stop\nfilling procedure?"
#define STR_END_OF_FILLING "end of filling"
#define  STR_WAITING "Waiting"

#define STR_HE_LEVEL "HE LEVEL"
#define STR_AUTFILL_IS_DISABLED_HE_LEVEL_TOO_LOW "Autofill is disabled.\nHe Level is too low."
#define STR_FILLING_TERMINATED "Filling terminated"
#define STR_OPTIONS "Options"																	    		
#define STR_UPLOAD_CHANGES_TO_DATABASE  "Upload changed \noptions to \ndatabase? \nTo make changes permane-nt please confirm them\nin the Helium ManagementSystem."
#define STR_SAVING_SETTINGS "Saving settings"
#define STR_ON "ON "

//paint_diag()----------------
//page1
#define STR_DIAG_1of2 "Diag 1/2"

#define STR_RESISATANCE "Res:"
#define STR_HELIUM      "HeL:"
#define STR_BATTERY     "Bat:"
#define STR_PRESSURE    "Prs:"
#define STR_CURRENT     "Cur:"

//page2
#define STR_DIAG_2of2 "Diag 2/2"

#define STR_ADCV        "ADCV:"
#define STR_ADCI        "ADCI:"
#define STR_I           "I:"
#define STR_U           "U:"
#define STR_US          "Us:"

//diag_pulse()
#define STR_SAVE_PULSE_PARAMS "Save Pules Prameters"
#define STR_DO_YOU_WANT_TO_USE_THE_NEW_QUENCHTIME "Do you want to \nuse the new quench time?"
//----------------------------------------------------

#define STR_TO_SERVER_FAILED  "to server failed!"
#define STR_RESTART_DEVICE_TO_TRY_AGAIN "Restart device to try again"
#define STR_FAILED "Failed"
#define STR_TO_SHUTDOWN "to shutdown!"
#define STR_PROBABLY_HARDWARE_ISSUE "Probably hardware issue..."
#define STR_UNKNOWN_ERROR "unknown error"
#define STR_PROBABLY_HARDWARE "(probably hardware)"


#define STR_CALIBRATION "Calibration"
#define STR_SET_RESISTANCE_OR_NZERO_TO_DEFAULT "Set resistance span/\nzero to default?"
#define STR_SET_RESISTANCE_TO_N_OHM           " Set resistance\nto %d ohm"
#define STR_CALIBRATION "Calibration"

#define STR_R_CALIB "R_calib:"
#define STR_VOLT    "Volt:"
#define STR_CURR    "Curr:"
#define STR_RMEAS   "R_meas:"
#define STR_RESULTS "Results"
#define STR_R_SPAN  "r_span:"
#define STR_R_ZERO  "r_zero:"

#define STR_WOULD_YOU_LIKE_TO_SAVE " Would you like to save?"
#define STR_FATAL_ERROR_UNKNOWN_MODE "Fatal error: unknown mode"
#define STR_SEND_AWAKE_MSG "Send awake msg."
#define STR_TRANSMIT_STORED "Transmit stored"
#define STR_XBEE_AWAKE "XBee awake"
#define STR_FILLING_IS_DISABLE_LEVEL_TOO_LOW "Filling is disabled.\nHe Level is too low."
#define STR_AUTOFILL_STARTED "AutoFill started"
#define STR_SENING_DATA "sending data..."
#define STR_TIMEOUT_MIN "timeout: %i min"
#define STR_INTERVAL_S "interval: %i s"
#define STR_INTERVAL_MIN "interval: %i min"

#define STR_THE_BATTERY_IS_CRITTICALLY_LOW_SYSTEM_WILL_SHUT_DOWN "The battery is\ncritically low (%d%%).\nSystem will shut down!\nLast He Level:\n%d%%"
#define STR_SHUTTING_DOWN "Shutting down..."
#define STR_GOOD_OPTIONS_RECEIVED "Good options received"
#define STR_PRESS_MEASURE_X2 "press MEASURE"
#define STR_TO_CONTINUE "to continue"
#define STR_DEFAULT_OPTS_SET "default options set"
#define STR_POSITIONS_SENT "positions sent."
#define STR_POS_WRITTEN "Pos. written."
#define STR_PASSW_SENT "Passw. sent."
#define STR_NEW_PASSW "New passw.: %d"
#define STR_SLEEP_TIME_SENT "Sleep time sent."
#define STR_XBEE_SLEEP "Xbee sleep: %d m"
#define STR_NOTICE "Notice: "
#define STR_WARNING "Warning: "
#define STR_ERROR "Error: "
#define STR_PRESS_MEASURE "press MEASURE"
#define STR_AWAKE_TIME_SENT "Awake time sent."
#define STR_AWAKE_s "Awake: %d s"