#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "ft800_driver.h"

void main_state(void) {
	uint32_t tagval = 0;
	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
	uint16_t x = 0, y = 0;
	char num[1];
	char batteryCurrent[5];
	char motorACCurrent[5];
	
	static uint16_t count5s = 0;
		
	/************************* SCREEN INIT ************************************/
	do
	{
		cmd_buffer_read = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
		cmd_buffer_write = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
	}while (cmd_buffer_write != cmd_buffer_read);									// Wait until the two registers match

	set_cmd_offset(cmd_buffer_write);														// The new starting point the first location after the last command
		
	tagval =  FT800_Mem_Read32(REG_ID);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_DLSTART));// Start the display list
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR_RGB | BLACK));
																											// Set the default clear color to black
	increase_cmd_offset( 4);								// Update the command pointer


	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
																											// Clear the screen - this and the previous prevent artifacts between lists
																											// Attributes are the color, stencil and tag buffers
	increase_cmd_offset( 4);								// Update the command pointer

	/************************* TOUCH ENGINE ************************************/
	tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
//	if((uint32_t)(tagval & (1UL << 31)) == 0)
//	{
//		y = tagval & 0x3ff;
//		x = (tagval >> 16) & 0x3ff;
//		
//		if(x > CONVERTCOORDINATES_X(266) && y > CONVERTCOORDINATES_Y(270-31))
//		{
//			//event = EV_USERFAULTLOG;
//		}
//		else {
//			count5s++;
//			if(count5s >= 40) {
//				//event = EV_ADMINMODE;
//				count5s = 0;
//			}
//		}		
//	}
//	else {
//		count5s = 0;
//	}
	
	/************************* LOG BUTTON ************************************/
	FT800_CMD_Button(266, 0, 214, 31, 29, 0, "               Log");
	
	/************************* DRAW SOC STATUS BAR ************************************/
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | GREEN); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);	
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_BEGIN | RECTS)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);	
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(0*16,(uint16_t)(136-100/*((float)inputData.batterySOC/100)*/*136)*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);	
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240*16, 136*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | WHITE); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);	
			
	/*************************************************************************/
	/***************** Nacrtaj najvecu kruznicu ******************************/
	/*************************************************************************/
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | 0xFF8000); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), POINT_SIZE(2240)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_BEGIN | FTPOINTS)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);	
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240*16,136*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);	
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | BLACK); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), POINT_SIZE(2160)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_BEGIN | FTPOINTS)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);	
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240*16,136*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);	
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
				
	/*************************************************************************/
	/***************** Nacrtaj linije  ******************************/
	/*************************************************************************/			
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | 0xFF8000); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), LINE_WIDTH(20)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), BEGIN(LINES)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(0*16,136*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(100*16,136*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(0*16,204*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(120*16,204*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(359*16,204*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,204*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(378*16,136*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,136*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(328*16,31*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,31*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	/********************** ALARM NOTIFICATION **************************/
//	if(inputData.alarm & 1) {
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(255,0,0)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(434*16,61*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(402*16,119*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(402*16,119*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(470*16,119*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(435*16,61*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(470*16,118*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//	}
	
	
	/* SPEEDOMETER */
	{
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(255,255,255)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);	
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(135 *16,194 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(123 *16,201 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(131 *16,187 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(119 *16,193 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(128 *16,180 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(115 *16,185 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(126 *16,173 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(113 *16,177 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(124 *16,166 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(110 *16,169 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(122 *16,158 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(108 *16,161 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(121 *16,151 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(107 *16,153 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(120 *16,144 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(106 *16,144 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(120 *16,136 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(106 *16,136 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(120 *16,128 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(106 *16,128 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(121 *16,121 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(107 *16,119 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(122 *16,114 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(108 *16,111 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(124 *16,106 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(110 *16,103 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(126 *16,99 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(113 *16,95 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(128 *16,92 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(115 *16,87 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(131 *16,85 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(119 *16,79 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(135 *16,78 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(123 *16,71 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(139 *16,72 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(127 *16,64 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(143 *16,65 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(132 *16,57 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(148 *16,60 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(137 *16,51 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(153 *16,54 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(142 *16,44 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(158 *16,49 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(148 *16,38 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(164 *16,44 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(155 *16,33 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(169 *16,39 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(161 *16,28 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(176 *16,35 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(168 *16,23 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(182 *16,31 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(175 *16,19 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(189 *16,27 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(67,67,67)); 
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(183 *16,15 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(196 *16,24 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(191 *16,11 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(203 *16,22 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(199 *16,9 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(210 *16,20 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(207 *16,6 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(218 *16,18 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(215 *16,4 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(225 *16,17 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(223 *16,3 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(232 *16,16 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(232 *16,2 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240 *16,16 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240 *16,2 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(248 *16,16 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(248 *16,2 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(255 *16,17 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(257 *16,3 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(262 *16,18 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(265 *16,4 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(270 *16,20 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(273 *16,6 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(277 *16,22 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(281 *16,9 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(284 *16,24 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(289 *16,11 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(291 *16,27 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(297 *16,15 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(298 *16,31 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(305 *16,19 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(304 *16,35 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(312 *16,23 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(311 *16,39 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(319 *16,28 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(316 *16,44 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(325 *16,33 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(322 *16,49 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(332 *16,38 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(327 *16,54 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(338 *16,44 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(332 *16,60 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(343 *16,51 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(337 *16,65 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(348 *16,57 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(341 *16,72 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(353 *16,64 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(345 *16,78 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(357 *16,71 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(349 *16,85 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(361 *16,79 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(352 *16,92 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(365 *16,87 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(354 *16,99 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(367 *16,95 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(356 *16,106 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(370 *16,103 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(358 *16,114 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(372 *16,111 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(359 *16,121 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(373 *16,119 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(360 *16,128 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(374 *16,128 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(360 *16,136 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(374 *16,136 *16));
		increase_cmd_offset( 4);		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(360 *16,136 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(374 *16,136 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(360 *16,144 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(374 *16,144 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(359 *16,151 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(373 *16,153 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(358 *16,158 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(372 *16,161 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(356 *16,166 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(370 *16,169 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(354 *16,173 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(367 *16,177 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(352 *16,180 *16));
		increase_cmd_offset( 4);
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(365 *16,185 *16));
		increase_cmd_offset( 4);
	}
	
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
			
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | WHITE); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	/********************** BATTERY CURRENT **************************/
	
	FT800_CMD_Text(1, 140, 26, 0, "Battery Current");
//	if(inputData.batteryCurrent < 100 && inputData.batteryCurrent >= 10) {
//		snprintf(num, 2, "%d", inputData.batteryCurrent);
//		strcpy(batteryCurrent, num);
//		strcat(batteryCurrent, ".");
//		
//		snprintf(num, 2, "%d", inputData.batteryCurrent%10);
//		strcat(batteryCurrent, num);
//		strcat(batteryCurrent, " A\0");
//		FT800_CMD_Text(5, 157, 31, 0, batteryCurrent);
//	}
//	else if(inputData.batteryCurrent > 0 && inputData.batteryCurrent <= 10) {
//		strcpy(batteryCurrent, "0.");		
//		snprintf(num, 2, "%d", inputData.batteryCurrent%10);
//		strcat(batteryCurrent, num);
//		
//		strcat(batteryCurrent, " A\0");
//		FT800_CMD_Text(5, 157, 31, 0, batteryCurrent);
//	}
//	else {
//		FT800_CMD_Text(5, 157, 31, 0, "9.9 A");
//	}
	
	/********************** BATTERY VOLTS **************************/
	FT800_CMD_Text(13, 208, 26, 0, "Battery Volts");
//	if(inputData.batteryVoltage >= 100 && inputData.batteryVoltage < 1000) {
//		char num[2];
//		char num2[1];
//		char batteryVoltage[6];
//		
//		snprintf(num, 3, "%d", inputData.batteryVoltage/10);
//		strcpy(batteryVoltage, num);
//		strcat(batteryVoltage, ".");
//		snprintf(num2, 2, "%d", inputData.batteryVoltage%10);
//		strcat(batteryVoltage, num2);
//		strcat(batteryVoltage, " V");
//		
//		FT800_CMD_Text(5, 226, 31, 0, batteryVoltage);
//	}
//	else if(inputData.batteryVoltage >= 10 && inputData.batteryVoltage < 100){
//		char num[2];
//		char num2[1];
//		char batteryVoltage[5];
//		
//		snprintf(num, 2, "%d", inputData.batteryVoltage/10);
//		strcpy(batteryVoltage, num);
//		strcat(batteryVoltage, ".");
//		snprintf(num2, 2, "%d", inputData.batteryVoltage%10);
//		strcat(batteryVoltage, num2);
//		strcat(batteryVoltage, " V");
//		
//		FT800_CMD_Text(5, 226, 31, 0, batteryVoltage);
//	}
//	else if(inputData.batteryVoltage > 0 && inputData.batteryVoltage < 10) {
//		char num2[1];
//		char batteryVoltage[5];
//		
//		strcpy(batteryVoltage, "0.");
//		snprintf(num2, 2, "%d", inputData.batteryVoltage%10);
//		strcat(batteryVoltage, num2);
//		strcat(batteryVoltage, " V");
//		
//		FT800_CMD_Text(5, 226, 31, 0, batteryVoltage);
//	}
//	else {
//		FT800_CMD_Text(5, 226, 31, 0, "78.4 V");
//	}
		
	/********************** AC MOTOR CURRENT **************************/
	FT800_CMD_Text(404, 140, 26, 0, "AC Motor");	
	FT800_CMD_Text(410, 152, 26, 0, "Current");	
//	if(inputData.motorACCurrent < 100 && inputData.motorACCurrent >= 10) {		
//		snprintf(num, 2, "%d", inputData.motorACCurrent/10);
//		strcpy(motorACCurrent, num);
//		strcat(motorACCurrent, ".");
//		
//		snprintf(num, 2, "%d", inputData.motorACCurrent%10);
//		strcat(motorACCurrent, num);
//		strcat(motorACCurrent, " A\0");
//		FT800_CMD_Text(409, 167, 30, 0, motorACCurrent);
//	}
//	else if(inputData.motorACCurrent > 0 && inputData.motorACCurrent < 10) {
//		strcpy(motorACCurrent, "0.");
//		
//		snprintf(num, 2, "%d", inputData.motorACCurrent%10);
//		strcat(motorACCurrent, num);
//		strcat(motorACCurrent, " A\0");
//		FT800_CMD_Text(409, 167, 30, 0, motorACCurrent);
//	}
//	else {
//		FT800_CMD_Text(409, 167, 30, 0, "9.9 A");
//	}
		
	/********************** DATETIME **************************/
//	if(strlen(inputData.datetime) < 23) {
//		FT800_CMD_Text(331, 244, 26, 0, inputData.datetime);
//	}
//	else{
//		FT800_CMD_Text(350, 244, 26, 0, "Datetime error!");
//	}

	/********************** SPEED(RPM) **************************/
//	if((inputData.speed < 2000) && (inputData.speed > 999)) {
//		char speed[4];
//		snprintf(speed, 5, "%d", inputData.speed);
//		FT800_CMD_Text(194, 90, 31, 0, speed);
//	}
//	else if(inputData.speed > 99 && inputData.speed < 1000) {
//		char speed[3];
//		snprintf(speed, 4, "%d", inputData.speed);
//		FT800_CMD_Text(210, 90, 31, 0, speed);
//	}
//	else if(inputData.speed > 0 && inputData.speed < 100) {
//		char speed[2];
//		snprintf(speed, 3, "%d", inputData.speed);
//		FT800_CMD_Text(240, 90, 31, 0, speed);
//	}
//  else {
//		FT800_CMD_Text(194, 90, 31, 0, "2000");
//	}	

	
	/********************** BATTERY SOC **************************/
//	if(inputData.batterySOC <= 100) {
//		char num[1];
//		char battery[4];
//		
//		snprintf(num, 4, "%d", inputData.batterySOC);
//		strcpy(battery, num);
//		strcat(battery, "%");
//		
//		FT800_CMD_Text(25, 50, 29, 0, battery);
//	}
//	else {
//		FT800_CMD_Text(25, 50, 29, 0, "100%");
//	}
		
	/********************** GEAR MODE **************************/
//	switch(inputData.gearMode) {
//		case 'D': {
//			FT800_CMD_Text(194, 139, 31, 0, "D");	
//	
//			FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(67,67,67)); // Indicate to draw a point (dot)
//			increase_cmd_offset( 4);
//			
//			FT800_CMD_Text(229, 139, 31, 0, "N");			
//			FT800_CMD_Text(264, 139, 31, 0, "R");			
//		} break;
//		case 'N': {	
//			FT800_CMD_Text(229, 139, 31, 0, "N");
//			
//			FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(67,67,67)); // Indicate to draw a point (dot)
//			increase_cmd_offset( 4);
//			
//			FT800_CMD_Text(194, 139, 31, 0, "D");			
//			FT800_CMD_Text(264, 139, 31, 0, "R");			
//		} break;
//		case 'R': {
//						
//			FT800_CMD_Text(264, 139, 31, 0, "R");
//			
//			FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(67,67,67)); // Indicate to draw a point (dot)
//			increase_cmd_offset( 4);
//			
//			FT800_CMD_Text(229, 139, 31, 0, "N");
//			FT800_CMD_Text(194, 139, 31, 0, "D");			
//		} break;
//		default: {
//			FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(67,67,67)); // Indicate to draw a point (dot)
//			increase_cmd_offset( 4);
//			
//			FT800_CMD_Text(229, 139, 31, 0, "N");
//			FT800_CMD_Text(194, 139, 31, 0, "D");
//			FT800_CMD_Text(264, 139, 31, 0, "R");
//		}
//	}
	
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(255,80,0)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_CMD_Text(293, 117, 26, 0, "rpm");
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(255,0,0)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	/********************** ALARM **************************/
//	if(inputData.alarm & 1) {
//		FT800_CMD_Text(430, 73, 31, 0, "!\0");
//	}

	/********************** FRAME END **************************/
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());					// Update the ring buffer pointer so the graphics processor starts executing	
			
	//return event;
}
