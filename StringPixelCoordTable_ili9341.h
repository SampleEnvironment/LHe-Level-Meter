// String and Pixel Coord Table for ILI9341


// there is 5px padding around the outer edge of the Display for placing elements,
// due to viewing Angle issues


// Please Select Font for HE-Level Display 
//#define inconsolata
//#define Lucida_Console
#define Lucida_Console_Alpha_Num
//#define source_code_pro
//#define consolas


//BORDER_OFFSET:
/////////////////////////////
//           /\            //
//           ||            //
//           5px           //
//           ||            //
//           \/            //
//<--5px-->content<--5px-->//
//           /\            //
//           ||            //
//           5px           //
//           ||            //
//           \/            //
/////////////////////////////
#define BORDER_OFFSET 5
#define LCD_WIDTH  319
#define LCD_HEIGHT 239

#define FONT1_H 16
#define FONT1_W 8

#define FONT2_H 24
#define FONT2_W 12

// High res Fonts
#ifdef Lucida_Console_Alpha_Num
#define FONT3_H 40
#define FONT3_W 34
#define FONT3_W1 29
#endif

#ifdef Lucida_Console
#define FONT3_H 40
#define FONT3_W 27
#endif

#ifdef inconsolata
#define FONT3_H 40
#define FONT3_W 27
#endif

#ifdef consolas
#define FONT3_H 40
#define FONT3_W 30
#endif

#ifdef source_code_pro
#define FONT3_H 40
#define FONT3_W 31
#endif

// For reversing Display (Landscape and Landsacpe180)
#define XOFFSET_32 (56 + BORDER_OFFSET)


#define CENTER_LINE (LCD_WIDTH-XOFFSET_32)/2

//Font Params //TODO use Values that are defined by the Font
#define CHAR_CELL_WIDTH_FONT_1 8//6
#define HALF_SPACE_WIDTH_FONT_1 4
#define CHAR_CELL_WIDTH_FONT_2 12//9
#define HALF_SPACE_WIDTH_FONT_2 6//3

//InitScreen_AddLine()
#define IAdd_Line_Max_Lines 15//12
#define IAdd_Line_LineFeed  16//10
#define X_IA_2 (BORDER_OFFSET+3) 

//main()
#define FONTNR_M_1 2
 
#define X_M_0 0
#define X_M_1 (X_GHL_5-1)
#define X_M_2 X_GHL_5
#define X_M_5 X_GHL_5
#define X_M_40 ((LCD_WIDTH/2)-FONT2_W* 4)
#define X_M_50 50
#define X_M_60 (X_PIL_2+(8*FONT1_W))
#define X_M_85 85
#define X_M_100 100
#define X_M_150 280

#define Y_M_5 5
#define Y_M_20 20
#define Y_M_40 40
#define Y_M_40_NOTICE 50
#define Y_M_60 60
#define Y_M_57 80
#define Y_M_66 104
#define Y_M_80 80
#define Y_M_90 Y_PIL_90
#define Y_M_100 Y_GHL_105
#define Y_M_105 Y_GHL_105

//======================================================================================
// paint_enter_number()
//======================================================================================
#define SCALE_PE_2 2
#define FONTNR_PE_2 3
#define SPACE_FAC_PE FONT3_W + 3
#define FONT_W_PE FONT3_W
#define FONT_H_PE FONT3_H


#define X_PE_4    (X_PM_1)									   // Standard Distance from Buttons/edge
#define X_PE_5	  ((LCD_WIDTH-XOFFSET_32)/2)-((FONT2_W*15)/2)  // "Enter Password:" centered
#define X_PE_42   ((LCD_WIDTH-XOFFSET_32)/2)-((FONT2_W*7)/2)   // "Options" centered
#define X_PE_30   X_PE_60 - (FONT3_W*SCALE_PE_2 + 10)	  	   // 1.Digit
#define X_PE_60   ((LCD_WIDTH-XOFFSET_32)/2)-FONT3_W     	   // 2.Digit
#define X_PE_90   X_PE_60 + (FONT3_W*SCALE_PE_2 + 10)		   // 3.Digit


#define Y_PE_2    (BORDER_OFFSET)							   // First  Line
#define Y_PE_15   (BORDER_OFFSET + FONT2_H)					   // Second Line
#define Y_PE_35   (Y_PE_15+ 11)					               // Second Line +  ~half Line
#define Y_PE_40   (BORDER_OFFSET + FONT2_H*2)				   // Third  Line
#define Y_PE_70   90                                           // Digits "0 0 0"

//======================================================================================
// LCD_iDtdalog() & timed_dialog()
//======================================================================================
#define LD_Line_Height       (FONT1_H)
#define LD_MAX_Height        (LCD_HEIGHT - Y_LD_28 - (2*Y_LD_1)-FONT1_H) // Maximal vertical space reserved for Text
#define LD_Line_Length      (X_LD_166-X_LD_20-BORDER_OFFSET*2)           // Maximal horizontal space for Text
#define LD_HALF_CHAR_WIDTH   (FONT2_W /2)		
#define LD_MID_LANDSCAPE    (LCD_WIDTH/2)
#define LD_MID_LANDSCAPE180 (LCD_WIDTH/2)

//Landscape180:
#define X_LD_3    (BORDER_OFFSET+3)                    // "YES   NO" vertically aligned left side
#define X_LD_17   (X_LD_3+BORDER_OFFSET+FONT2_W)	   // Dialog-Box Left Edge
#define X_LD_166  (LCD_WIDTH -X_LD_17)				   // Dialog-Box Right Edge
#define X_LD_20   (X_LD_17+BORDER_OFFSET)			   // horizontal alignment for Text in Dialog-Box

//Landscape:
#define X_LD_10   (X_LD_17)						       // Dialog-Box Left Edge
#define X_LD_160  (X_LD_166)						   // Dialog-Box Right Edge
#define X_LD_165  (LCD_WIDTH-BORDER_OFFSET-FONT2_W)	   //"YES   NO" vertically aligned right side
#define X_LD_13    (X_LD_20)                           //horizontal alignment for Text in Dialog-Box

//default:
#define X_LD_120  215								   // Dialog-Box Right Edge


#define Y_LD_1   (BORDER_OFFSET )					   //"Y" and "title"
#define Y_LD_17  (Y_LD_1  + FONT2_H)                   //"e"    
#define Y_LD_33	 (Y_LD_17 + FONT2_H)                   //"s"

#define Y_LD_99  (Y_LD_115-FONT2_H)                    //"N"
#define Y_LD_115 (LCD_HEIGHT - BORDER_OFFSET - FONT2_H)//"o"

#define Y_LD_25    (Y_LD_1 + FONT2_H)				   // upper edge Dialog-box
#define Y_LD_28    (Y_LD_25 + BORDER_OFFSET)		   // First Line of text in Dialog-Box
#define Y_LD_120  (LCD_HEIGHT - 2* Y_LD_1)	           //lower edge Dialog-Box
#define Y_LD_165  294                                  //lower edge Dialog-Box (default, does usually never happen)

//======================================================================================
//paint_progress_bar()
//======================================================================================
/*
//X_PPB_OFFS_2
// <-><-----------------------X_PPB_OFFS_100---------------------------------->
// ======...     ============================================================== /\Y_PPB_OFFS_2
// |             <--X_PPB_OFFS_8--->                                          | \/
// |  ===...     =================== =================== ===================  | /\
// |  ###...     ################### ################### ###################  | ||
// |  ###...     ################### ################### ###################  | ||
// |  ###...     ################### ################### ###################  | || Y_PPB_OFFS_7
// |  ###...     ################### ################### ###################  | ||
// |  ###...     =================== =================== ===================  | ||
// |             <---X_PPB_OFFS_10-->                                         | ||
// ======...     ============================================================== \/
*/
#define  FONT_PPB_1 2
#define  Percentage_Font_Cellwidth CHAR_CELL_WIDTH_FONT_2


#define X_PPB_OFFS_2  2                    // Padding for Box around bar 1px on each side
#define X_PPB_OFFS_8  (X_PPB_OFFS_10-2)    // Length of individual Bars without whitespace (-2 because index starts at 0)
#define X_PPB_OFFS_10 (X_PPB_OFFS_100/10)  // Length of individual Bars + whitespace
#define X_PPB_OFFS_100  190				   // total Length of of inner progress Bar without box around it (has to be divisible by 10)
#define X_PPB_OFFS_103  (X_PPB_OFFS_100+5) // Percentage (i.e. "90%") on the right side of progress Bar

#define Y_PPB_OFFS_2  2					   // Padding for Box around bar 1px on each side
#define Y_PPB_OFFS_5  15				   // Height of bar elements
#define Y_PPB_OFFS_1  4					   // Height adjustment for "90%"
#define Y_PPB_OFFS_7  (Y_PPB_OFFS_5+2)	   // Height of Bar + Padding


//======================================================================================
//get_he_level()
//======================================================================================
#define X_GHL_5  (3 + X_PM_1)								  // Paint progress-bar same distance from Buttons/edge as everything else
#define Y_GHL_105 (Y_PBATT_120-(Y_PPB_OFFS_2+Y_PPB_OFFS_7)-6) // Progress-bar above Bat-Level Bar (6px space)
#define Y_GHL_90 168									      //TODO same as paint_info_line()

//======================================================================================
//paint_he_level()
//======================================================================================
#define FONTNR_PHL_2 3										  // Highres Font Selected
#define SCALE_PHL_4 2										  // Highres-Font only needs 2x Scaling
#define X_PHL_1 1			                           		  // Clear He-Percentage Line
#define X_PHL_HE_LVL  // Centered He level
#define X_PHL_2 (X_PM_1)									  // Distance of He-Level and Percentage from left border/Buttons
#define X_PHL_5 5											  // debug mode
#define X_PHL_25 (((LCD_WIDTH-XOFFSET_32)/2)-(2*6*FONT2_W)/2) // "Cable?" centered

#define X_PHL_L_OFFSET 8 		  // "l" offset to the left

#define X_PHL_85 (X_PM_1+FONT2_W*9+HALF_SPACE_WIDTH_FONT_2)   // unit "l" of percentage He-Level < 100
#define X_PHL_93 (X_PM_1+FONT2_W*12+HALF_SPACE_WIDTH_FONT_2)  // unit "l" of percentage He-Level else


#define Y_PHL_37 (Y_PTP_20+ FONT2_H + 25)                     // High-Res He-Level 25px under Time and Pressure
#define Y_PHL_61 (Y_PHL_37+FONT3_H*2-FONT2_H*2+8)			  // "l" High-Res He-Level (+8 because there is some whitespace encoded in Font2)
#define Y_PHL_90 90											  // debug mode???
#define Y_PHL_100 (Y_PBATT_119-FONT2_H-5)					  // He-Level-Percentace directly above Batt-Level

/*
//======================================================================================
//paint_buttons()
//======================================================================================
//
//          Width
//        LEN_PB_30
//     <------------->
//     .-------------. / \    / \
//     |             |  |      |   PB_FIELD_h
//     |     TOP     |  |      |
//     |             |  |     \ /
//     .-------------.  | H
//     |             |  | e
//     |      +      |  | i
//     |             |  | g
//     --------------.  | h
//     |      |      |  | t
//     |   <- | ->   |  |
//     |      |      |  | LEN_PB_129
//     .-------------.  |
//     |             |  |
//     |      -      |  |
//     |             |  |
//	   .-------------.  |
//	   |             |  |
//	   |   BOTTOM    |  |
//	   |             |  |
//	   .-------------. \ /
*/

#define FONTNR_PB_1 2

#define PB_FIELD_h  46                              //  height of individual fields: (240-2*BORDER_OFFSET)/5
#define LEN_PB_26 (PB_FIELD_h)                      //  height of vline in the middle
#define LEN_PB_30 56                                //  total width of outer Button Box
#define LEN_PB_129 (LCD_HEIGHT-(2*BORDER_OFFSET)+1) // 129  total height of outer Button Box


//x-coords Landscape 180  (Reversed)
#define X_PB_Box_0 0		                        // top left corner (clear Box)
#define X_PB_0 (BORDER_OFFSET)                      // top left corner (Button Box) vline, hline
#define X_PB_1 12									// <-
#define X_PB_6 21									// +
#define X_PB_7 21									// -
#define X_PB_15 ((LEN_PB_30/2)+BORDER_OFFSET)		// "top";"bottom";vline-seperator
#define X_PB_15a X_PB_15
#define X_PB_17 38									// ->
#define X_PB_30 (LEN_PB_30+BORDER_OFFSET)			// top right corner (Button Box) vline
#define X_PB_32 (LEN_PB_30+BORDER_OFFSET+2)			// bottom right corner (clearBox)


//x-coords Landscape
#define X_PB_144 (LCD_WIDTH-XOFFSET_32)				// top left corner (clear Box),vline
#define X_PB_145 (X_PB_144-BORDER_OFFSET+X_PB_1)	// <-
#define X_PB_150 (X_PB_144-BORDER_OFFSET+X_PB_6)	// +
#define X_PB_151 (X_PB_144-BORDER_OFFSET+X_PB_7)	// -
#define X_PB_159 (X_PB_144-BORDER_OFFSET+X_PB_15)	// "top";"bottom";vline-seperator
#define X_PB_159a X_PB_159
#define X_PB_161 (X_PB_144-BORDER_OFFSET+X_PB_17)	// ->
#define X_PB_174 (X_PB_144-BORDER_OFFSET+XOFFSET_32)// top right corner (Button Box) vline
#define X_PB_176 (LCD_WIDTH)						// bottom right corner (clearBox)



//y-coords
#define Y_PB_Box_0 0								// top left corner (clear Box)
#define Y_PB_1 (BORDER_OFFSET)						// top left corner (Button Box) vline, 1.hline
#define Y_PB_5 14									// "top" String
#define Y_PB_25 49									// +
#define Y_PB_26 (BORDER_OFFSET+PB_FIELD_h-1)		// 2.hline
#define Y_PB_52 (BORDER_OFFSET+PB_FIELD_h*2-1)		// 3.hline
#define Y_PB_61  111								// <- ->
#define Y_PB_77  139								//  -
#define Y_PB_78  (BORDER_OFFSET+PB_FIELD_h*3-1)		// 4.hline
#define Y_PB_104 (BORDER_OFFSET+PB_FIELD_h*4-1)		// 5.hline
#define Y_PB_109 198								// "bottom" String
#define Y_PB_130 (LCD_HEIGHT-BORDER_OFFSET)			// bottom 6.hline Button-Box
#define Y_PB_131 (LCD_HEIGHT)						// bottom right corner (Box)




//======================================================================================
//paint_batt()
//======================================================================================

//                      Y_PBATT_120
//  X_PBATT_38-->|========================|<--X_PBATT_140
//         Batt  |XXXXXXXXXXXXXXXX	      |
//               |========================|
//                      Y_PBATT_130

#define  X_PBATT_1  (BORDER_OFFSET+3)                     // "Batt"
#define  X_PBATT_38 (X_PBATT_1+4*FONT2_W+2)               // left edge Box
#define  X_PBATT_39 (X_PBATT_38+1)                        // left edge Battery-Bar
#define  X_PBATT_140 (LCD_WIDTH-XOFFSET_32-BORDER_OFFSET) // right edge Box 140 319-6-54

#define  Y_PBATT_118 (LCD_HEIGHT-FONT2_H-BORDER_OFFSET+1) // "Batt"
#define  Y_PBATT_119 (Y_PBATT_118+1)					  // upper edge clear Box
#define  Y_PBATT_120 (Y_PBATT_118+2)					  // upper edge Box
#define  Y_PBATT_121 (Y_PBATT_118+3)					  // upper edge Battery-Bar
#define  Y_PBATT_131 (LCD_HEIGHT-BORDER_OFFSET+1)         // lower edge clear Box
#define  Y_PBATT_130 (Y_PBATT_131-1)					  // lower edge Box
#define  Y_PBATT_129 (Y_PBATT_131-2)                      // lower edge Battery-Bar

//======================================================================================
//paint_info_line()
//======================================================================================
#define X_PIL_2 (X_PM_1+4)					 // Info line Directly above Progress-Bar and to the right of
#define Y_PIL_90 (Y_GHL_105-FONT1_H-4)       // Buttons/edge


//======================================================================================
//paint_main()
//======================================================================================

#define FONTNR_PM_1 2                                            // enlarge "conn" Font1 to Font2
#define AUTOFILL_BOX_WIDTH 13

#define X_PM_1 (BORDER_OFFSET + 3)                               // dist to buttons/left edge of STR_VESSEL_NR
#define X_PM_4 (BORDER_OFFSET + 5)                               // "conn" in Landscape 180
#define X_PM_148 (X_PB_144-BORDER_OFFSET+X_PM_4)                 // "conn" in Landscape
#define X_PM_25 (X_PHL_25)				     					 // HE-Level Large Number
#define X_PM_65 (8*FONT2_W+BORDER_OFFSET+1)                      // Vessel# "Number"
#define X_PM_100 (LCD_WIDTH-XOFFSET_32-BORDER_OFFSET-(4*FONT2_W))// 100 319-4*12 (4 char String top right corner)
#define X_AUTOFFILL_INDICATOR  (LCD_WIDTH-(XOFFSET_32+BORDER_OFFSET+AUTOFILL_BOX_WIDTH+5))


#define Y_PM_2 (BORDER_OFFSET + 3)                               // dist to buttons/upper edge of STR_VESSEL_NR
#define Y_PM_37 (Y_PHL_37)                                       // HE-Level Large Number
#define Y_PM_83 152                                              // "conn" String in Button Field

//======================================================================================
//paint_time_pressure()
//======================================================================================
#define X_PTP_2  (X_PM_1)                      //Time: "hh:mm" same coord as STR_VESSEl_NR
#define X_PTP_60 (X_PM_1 + 6 * FONT2_W)        //Pressure

#define Y_PTP_20 (Y_PM_2+FONT2_H)              //Time + Pressure under STR_VESSEl_NR



//======================================================================================
//paint_start_filling()
//======================================================================================
#define  FONTNR_PSF_2 3
#define OFFS_PSF_20 20 


#define  X_PSF_5  ((LCD_WIDTH-XOFFSET_32)/2)-((FONT2_W*14)/2)   // "Enter Position" centered
#define  X_PSF_30 ((LCD_WIDTH-XOFFSET_32)/2)-((FONT3_W*4*2)/2)   // "xxxx" Pos centered
#define  X_PSF_40 (X_PSF_30+10) 
#define  X_PSF_52 ((LCD_WIDTH-XOFFSET_32)/2)-((FONT2_W*4)/2)   // "Fill" centered
#define  X_PSF_zero (((LCD_WIDTH-XOFFSET_32)/2)-5)

#define Y_PSF_2 Y_PE_2
#define Y_PSF_35 Y_PE_35
#define Y_PSF_70 90

//======================================================================================
//paint_filling() --> see paint_main()
//======================================================================================

//======================================================================================
//paint_options() & paint_current_opt_page#()
//======================================================================================


//==============================================
//<----X_PO_25----->Options   1/5              |
//<---->Shutdown:	   					off    |
//X_PO_2                                       |
//		Pos:							none   |
//                                             |
//		Autofill:						off    |
//                                             |
//		He.Min:							20%    |
//<-------------X_PO_85---------------->       |
//		Timeout:						60min  |
//==============================================


#define X_PO_2  (BORDER_OFFSET*2)							  //Vertical alignment of Options 
#define X_PO_25 ((LCD_WIDTH-XOFFSET_32)/2)-((11*FONT2_W)/2)//"Options X/5" centered
#define X_PO_85 185											  //Vertical alignment of Option Values 
#define X_PO_120 240

#define OPTION_LINE_DISTANCE (LCD_HEIGHT - (Y_PO_2 +FONT2_H +20))/ 5
#define Y_PO_2 (BORDER_OFFSET*2)							  //"Options X/5" 
#define Y_PO_20 50											  //1.Line
#define Y_PO_40 (Y_PO_20+OPTION_LINE_DISTANCE)				  //2.Line
#define Y_PO_60 (Y_PO_20+OPTION_LINE_DISTANCE*2)		      //3.Line
#define Y_PO_80 (Y_PO_20+OPTION_LINE_DISTANCE*3)			  //4.Line
#define Y_PO_100 (Y_PO_20+OPTION_LINE_DISTANCE*4)			  //5.Line

//======================================================================================
//paint_opt_values_page#()
//======================================================================================

#define X_POVP_76 (X_PO_85-FONT2_W) //Vertical alignment of Option Values offset one Char to the left 


//======================================================================================
//paint_diag() + diag_page1() + diag_page2() 
//======================================================================================

#define FONTNR_PD_1 2
#define X_PD_4a ((LEN_PB_30/2)+BORDER_OFFSET)-(FONT2_W*3)/2				// "cal"  landscape180
#define X_PD_4b 1+((LEN_PB_30/2)+BORDER_OFFSET)-(FONT2_W*4)/2			// "curr" landscape180
#define X_PD_2 (X_PO_2)													// Vertical alignment of Options same as in Options
#define X_PD_20 20														// "Pulse"
#define X_PD_35 ((LCD_WIDTH-XOFFSET_32)/2)-((8*FONT2_W)/2)			    // "Diag 1/2" centered
#define X_PD_40 126														// Vertical alignment for Diag values
#define X_PD_50 126														//    "         "      "   "     "
#define X_PD_90 (X_PD_50+FONT2_W*5)										// Second collumn for Diag values
#define X_PD_148a (X_PB_144-BORDER_OFFSET+X_PB_15)-(FONT2_W*3)/2	    // "cal" landscape
#define X_PD_148b (2+(X_PB_144-BORDER_OFFSET+X_PB_15)-(FONT2_W*4)/2)	// "curr"landscape



#define Y_PD_2  (Y_PO_2)	 //Same as Optionspages "Diag 1/2"
#define Y_PD_20 (Y_PO_20)	 //1.Line
#define Y_PD_40 (Y_PO_40)    //2.Line
#define Y_PD_60 (Y_PO_60)    //3.Line
#define Y_PD_80 (Y_PO_80)    //4.Line
#define Y_PD_100 (Y_PO_100)  //5.Line

#define Y_PD_31 59			 // "cal
#define Y_PD_82 153		     // "curr"

//======================================================================================
//diag_pulse()
//======================================================================================

//diag_pulse() function was significantly changed for ili9341
#define DP_NUMBER_OF_POINTS_140 125
#define DP_POINTS_IN_PULSE_80 70
#define DP_U_ZEROLINE_70 125
#define DP_I_ZEROLINE_125 232
#define DP_Y_MAX_PIXELS_50 95
#define DP_PADDING_10 2//20 
#define DP_X_FACTOR 2
#define DP_AXIS_FONTNR 1
#define DP_SEND_FONTNR 2
#define DP_SEND_YFACTOR 1


#define X_DP_1 (BORDER_OFFSET)+2
#define X_DP_2 (X_PB_15-(3*LD_HALF_CHAR_WIDTH)+1)
#define X_DP_2_SEND (X_PB_15-(3*LD_HALF_CHAR_WIDTH)-5)
#define X_DP_3 (BORDER_OFFSET + 200)//(BORDER_OFFSET +3) 
#define X_DP_20 (BORDER_OFFSET +3) 
#define X_DP_60 90
#define X_DP_60_AVG 74
#define X_DP_60_NORMAL X_DP_60_AVG+4*FONT1_W
#define X_DP_75 (180+1*FONT1_W)
#define X_DP_147 (X_PB_159-(3*LD_HALF_CHAR_WIDTH)+1)
#define X_DP_147_SEND (X_PB_159-(3*LD_HALF_CHAR_WIDTH)-5)

#define Y_DP_2 2
#define Y_DP_6 10
#define Y_DP_OFFS 5
#define Y_DP_cursor 30
#define Y_DP_31 60
#define Y_DP_71 (DP_U_ZEROLINE_70+1)
#define Y_DP_85 Y_PB_77+13
#define Y_DP_126 (DP_I_ZEROLINE_125+1)

//======================================================================================
//Pulse Select
//======================================================================================
#define PLOT_X 260
#define PLOT_Y 40

#define X_AXIS_LEN 35
#define Y_AXIS_LEN 25



#define X_t_UNIT 2
#define X_I_UNIT 9


#define Y_t_UNIT 5

#define Y_COODRS_NORMAL uint16_t Ypoints[17] = {0, 1, 6, 14, 20, 23, 24,23,23, 22, 18,17,16,16,16,16,16 };
#define X_COORDS_NORMAL 16

#define Y_COODRS_CONST 15

#define Y_COODRS_LINEAR 5

//===============================================================================
//-----INIT-Screen---------------------------------------------------------------
//===============================================================================
#define STR_HZB_LEVELMETER             "Levelmeter"
#define STR_FIRMWARE_VERSION           "Firmware Version: "
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
#define STR_NO_DEVICE_ENTERED "No Device ID was entered"


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
#define STR_Conn   "conn"  //Changed old was "Conn"

//paint_batt()
#define STR_BATT   "Batt"

//paint_offline()
#define STR_OFF "OFF"

#define STR_EEPROM_UNDEFINED "EEPROM undefined"
#define STR_VALUES_MIGHT_BE_UNDEFINED "Values stored in EEPROM\nmight be undefined, please\nrecalibrate!\nPress YES to not show this\nmessage again."

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
#define STR_ENABLE_PR				"Press Sens:"
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
#define STR_ALPHANUM					"Alphanumeric:"
#define STR_NLOGINCHARS 				"N Login Chars:"

//-----------------




//pulse_select_Mode-----------------
#define STR_PULSE_TYPE "Pulse type:"

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
#define STR_UPLOAD_CHANGES_TO_DATABASE  "Upload changed \noptions to \ndatabase?"
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