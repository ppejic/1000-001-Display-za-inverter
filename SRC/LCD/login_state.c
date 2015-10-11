#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "ft800_driver.h"
#include "config.h"

static char password[PASSWORD_LENGTH+1];

//lcd_state_header_ptr login_state() {																						
//		static lcd_state_header_t state;																						
//																																								
//		state.process_input = login_process_input;																		
//		state.update_frame  = login_update_frame;																		
//																																								
//		return &state;																															
//	}
DEFINE_LCD_STATE(login);

lcd_state_header_t* login_process_input(lcd_state_header_t* lcd_state) {
	uint32_t tagval;
	uint16_t x, y;
	
	static uint8_t i = 0;
	
	//TODO:logic for transition
	tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
	if((uint32_t)(tagval & (1UL << 31)) == 0)
	{
		y = tagval & 0x3ff;
		x = (tagval >> 16) & 0x3ff;
		
		if(x <= CONVERTCOORDINATES_X(158)) {
			if(y <= CONVERTCOORDINATES_Y(270-82) && y >= CONVERTCOORDINATES_Y(270-127)) {
				//1
				if(i < PASSWORD_LENGTH) password[i++] = '1';
			}
			else if(y <= CONVERTCOORDINATES_Y(270-129) && y >= CONVERTCOORDINATES_Y(270-174)) {
				//4
				if(i < PASSWORD_LENGTH) password[i++] = '4';
			}
			else if(y <= CONVERTCOORDINATES_Y(270-176) && y >= CONVERTCOORDINATES_Y(270-221)) {
				//7
				if(i < PASSWORD_LENGTH) password[i++] = '7';
			}
			else if(y <= CONVERTCOORDINATES_Y(270-223)) {
//				//Back
//				event = EV_BACK;
				
				memset(password, 0x00, PASSWORD_LENGTH);
				i = 0;
				return main_state();
			}
		}
		else if(x >= CONVERTCOORDINATES_X(160) && x <= CONVERTCOORDINATES_X(318)) {
			if(y <= CONVERTCOORDINATES_Y(270-82) && y >= CONVERTCOORDINATES_Y(270-127)) {
				//2
				if(i < PASSWORD_LENGTH) password[i++] = '2';
			}
			else if(y <= CONVERTCOORDINATES_Y(270-129) && y >= CONVERTCOORDINATES_Y(270-174)) {
				//5
				if(i < PASSWORD_LENGTH) password[i++] = '5';
			}
			else if(y <= CONVERTCOORDINATES_Y(270-176) && y >= CONVERTCOORDINATES_Y(270-221)) {
				//8
				if(i < PASSWORD_LENGTH) password[i++] = '8';
			}
			else if(y <= CONVERTCOORDINATES_Y(270-223)) {
				//0
				if(i < PASSWORD_LENGTH) password[i++] = '0';
			}
		}
		else if(x >= CONVERTCOORDINATES_X(320)) {
			if(y <= CONVERTCOORDINATES_Y(270-82) && y >= CONVERTCOORDINATES_Y(270-127)) {
				//3
				if(i < PASSWORD_LENGTH) password[i++] = '3';
			}
			else if(y <= CONVERTCOORDINATES_Y(270-129) && y >= CONVERTCOORDINATES_Y(270-174)) {
				//6
				if(i < PASSWORD_LENGTH) password[i++] = '6';
			}
			else if(y <= CONVERTCOORDINATES_Y(270-176) && y >= CONVERTCOORDINATES_Y(270-221)) {
				//9
				if(i < PASSWORD_LENGTH) password[i++] = '9';
			}
			else if(y <= CONVERTCOORDINATES_Y(270-223)) {
				//LOGIN!
								
				if(strncmp(password, PASSWORD, PASSWORD_LENGTH ) == 0) {
//					event = EV_ADMINENTER;
					return admin_main_state();
				}
				
				memset(password, 0x00, PASSWORD_LENGTH);
				i = 0;
			}
		}
	}
	
	return lcd_state;
}

void login_update_frame(q_can_data_item_t* q_can_data) {
	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
	uint32_t tagval = 0;
	uint16_t x = 0;
	uint16_t y = 0;
	//static char password[PASSWORD_LENGTH+1] = {0};
	static uint8_t i = 0;
	
	//event = EV_NONE;	
	
	do
	{
		cmd_buffer_read = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
		cmd_buffer_write = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
	}while (cmd_buffer_write != cmd_buffer_read);									// Wait until the two registers match

	set_cmd_offset(cmd_buffer_write);															// The new starting point the first location after the last command
			
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_DLSTART));// Start the display list
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR_RGB | BLACK));
																											// Set the default clear color to black
	increase_cmd_offset( 4);								// Update the command pointer


	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
																											// Clear the screen - this and the previous prevent artifacts between lists
																											// Attributes are the color, stencil and tag buffers
	increase_cmd_offset( 4);								// Update the command pointer
			
	tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
	if((uint32_t)(tagval & (1UL << 31)) == 0)
	{
		y = tagval & 0x3ff;
		x = (tagval >> 16) & 0x3ff;
		
//		if(x <= CONVERTCOORDINATES_X(158)) {
//			if(y <= CONVERTCOORDINATES_Y(270-82) && y >= CONVERTCOORDINATES_Y(270-127)) {
//				//1
//				if(i < PASSWORD_LENGTH) password[i++] = '1';
//			}
//			else if(y <= CONVERTCOORDINATES_Y(270-129) && y >= CONVERTCOORDINATES_Y(270-174)) {
//				//4
//				if(i < PASSWORD_LENGTH) password[i++] = '4';
//			}
//			else if(y <= CONVERTCOORDINATES_Y(270-176) && y >= CONVERTCOORDINATES_Y(270-221)) {
//				//7
//				if(i < PASSWORD_LENGTH) password[i++] = '7';
//			}
//			else if(y <= CONVERTCOORDINATES_Y(270-223)) {
//				//Back
//				event = EV_BACK;
//				
//				memset(password, 0x00, PASSWORD_LENGTH);
//				i = 0;
//			}
//		}
//		else if(x >= CONVERTCOORDINATES_X(160) && x <= CONVERTCOORDINATES_X(318)) {
//			if(y <= CONVERTCOORDINATES_Y(270-82) && y >= CONVERTCOORDINATES_Y(270-127)) {
//				//2
//				if(i < PASSWORD_LENGTH) password[i++] = '2';
//			}
//			else if(y <= CONVERTCOORDINATES_Y(270-129) && y >= CONVERTCOORDINATES_Y(270-174)) {
//				//5
//				if(i < PASSWORD_LENGTH) password[i++] = '5';
//			}
//			else if(y <= CONVERTCOORDINATES_Y(270-176) && y >= CONVERTCOORDINATES_Y(270-221)) {
//				//8
//				if(i < PASSWORD_LENGTH) password[i++] = '8';
//			}
//			else if(y <= CONVERTCOORDINATES_Y(223)) {
//				//0
//				if(i < PASSWORD_LENGTH) password[i++] = '0';
//			}
//		}
//		else if(x >= CONVERTCOORDINATES_X(320)) {
//			if(y <= CONVERTCOORDINATES_Y(270-82) && y >= CONVERTCOORDINATES_Y(270-127)) {
//				//3
//				if(i < PASSWORD_LENGTH) password[i++] = '3';
//			}
//			else if(y <= CONVERTCOORDINATES_Y(270-129) && y >= CONVERTCOORDINATES_Y(270-174)) {
//				//6
//				if(i < PASSWORD_LENGTH) password[i++] = '6';
//			}
//			else if(y <= CONVERTCOORDINATES_Y(270-176) && y >= CONVERTCOORDINATES_Y(270-221)) {
//				//9
//				if(i < PASSWORD_LENGTH) password[i++] = '9';
//			}
//			else if(y <= CONVERTCOORDINATES_Y(270-223)) {
//				//LOGIN!
//								
//				if(strncmp(password, PASSWORD, PASSWORD_LENGTH ) == 0) {
//					event = EV_ADMINENTER;
//				}
//				
//				memset(password, 0x00, PASSWORD_LENGTH);
//				i = 0;
//			}
//		}
	}
	
	FT800_CMD_Button(0,82, 158,45,29,0, "1");
	FT800_CMD_Button(0,129,158,45,29,0, "4");
	FT800_CMD_Button(0,176,158,45,29,0, "7");
	FT800_CMD_Button(0,223,158,45,29,0, "Back");
	FT800_CMD_Button(160,82, 158,45,29,0, "2");
	FT800_CMD_Button(160,129,158,45,29,0, "5");
	FT800_CMD_Button(160,176,158,45,29,0, "8");
	FT800_CMD_Button(160,223,158,45,29,0, "0");
	FT800_CMD_Button(320,82, 158,45,29,0, "3");
	FT800_CMD_Button(320,129,158,45,29,0, "6");
	FT800_CMD_Button(320,176,158,45,29,0, "9");
	FT800_CMD_Button(320,223,158,45,29,0, "Login");	
	FT800_CMD_Text(189,10,31, 0, password);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());
		
	//return event;
	
}

//void login_state(void) {
//	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
//	uint32_t tagval = 0;
//	uint16_t x = 0;
//	uint16_t y = 0;
//	//static char password[PASSWORD_LENGTH+1] = {0};
//	static uint8_t i = 0;
//	
//	//event = EV_NONE;	
//	
//	do
//	{
//		cmd_buffer_read = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
//		cmd_buffer_write = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
//	}while (cmd_buffer_write != cmd_buffer_read);									// Wait until the two registers match

//	set_cmd_offset(cmd_buffer_write);															// The new starting point the first location after the last command
//			
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_DLSTART));// Start the display list
//	increase_cmd_offset( 4);								// Update the command pointer

//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR_RGB | BLACK));
//																											// Set the default clear color to black
//	increase_cmd_offset( 4);								// Update the command pointer


//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
//																											// Clear the screen - this and the previous prevent artifacts between lists
//																											// Attributes are the color, stencil and tag buffers
//	increase_cmd_offset( 4);								// Update the command pointer
//			
//	tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
//	if((uint32_t)(tagval & (1UL << 31)) == 0)
//	{
//		y = tagval & 0x3ff;
//		x = (tagval >> 16) & 0x3ff;
//		
////		if(x <= CONVERTCOORDINATES_X(158)) {
////			if(y <= CONVERTCOORDINATES_Y(270-82) && y >= CONVERTCOORDINATES_Y(270-127)) {
////				//1
////				if(i < PASSWORD_LENGTH) password[i++] = '1';
////			}
////			else if(y <= CONVERTCOORDINATES_Y(270-129) && y >= CONVERTCOORDINATES_Y(270-174)) {
////				//4
////				if(i < PASSWORD_LENGTH) password[i++] = '4';
////			}
////			else if(y <= CONVERTCOORDINATES_Y(270-176) && y >= CONVERTCOORDINATES_Y(270-221)) {
////				//7
////				if(i < PASSWORD_LENGTH) password[i++] = '7';
////			}
////			else if(y <= CONVERTCOORDINATES_Y(270-223)) {
////				//Back
////				event = EV_BACK;
////				
////				memset(password, 0x00, PASSWORD_LENGTH);
////				i = 0;
////			}
////		}
////		else if(x >= CONVERTCOORDINATES_X(160) && x <= CONVERTCOORDINATES_X(318)) {
////			if(y <= CONVERTCOORDINATES_Y(270-82) && y >= CONVERTCOORDINATES_Y(270-127)) {
////				//2
////				if(i < PASSWORD_LENGTH) password[i++] = '2';
////			}
////			else if(y <= CONVERTCOORDINATES_Y(270-129) && y >= CONVERTCOORDINATES_Y(270-174)) {
////				//5
////				if(i < PASSWORD_LENGTH) password[i++] = '5';
////			}
////			else if(y <= CONVERTCOORDINATES_Y(270-176) && y >= CONVERTCOORDINATES_Y(270-221)) {
////				//8
////				if(i < PASSWORD_LENGTH) password[i++] = '8';
////			}
////			else if(y <= CONVERTCOORDINATES_Y(223)) {
////				//0
////				if(i < PASSWORD_LENGTH) password[i++] = '0';
////			}
////		}
////		else if(x >= CONVERTCOORDINATES_X(320)) {
////			if(y <= CONVERTCOORDINATES_Y(270-82) && y >= CONVERTCOORDINATES_Y(270-127)) {
////				//3
////				if(i < PASSWORD_LENGTH) password[i++] = '3';
////			}
////			else if(y <= CONVERTCOORDINATES_Y(270-129) && y >= CONVERTCOORDINATES_Y(270-174)) {
////				//6
////				if(i < PASSWORD_LENGTH) password[i++] = '6';
////			}
////			else if(y <= CONVERTCOORDINATES_Y(270-176) && y >= CONVERTCOORDINATES_Y(270-221)) {
////				//9
////				if(i < PASSWORD_LENGTH) password[i++] = '9';
////			}
////			else if(y <= CONVERTCOORDINATES_Y(270-223)) {
////				//LOGIN!
////								
////				if(strncmp(password, PASSWORD, PASSWORD_LENGTH ) == 0) {
////					event = EV_ADMINENTER;
////				}
////				
////				memset(password, 0x00, PASSWORD_LENGTH);
////				i = 0;
////			}
////		}
//	}
//	
//	FT800_CMD_Button(0,82, 158,45,29,0, "1");
//	FT800_CMD_Button(0,129,158,45,29,0, "4");
//	FT800_CMD_Button(0,176,158,45,29,0, "7");
//	FT800_CMD_Button(0,223,158,45,29,0, "Back");
//	FT800_CMD_Button(160,82, 158,45,29,0, "2");
//	FT800_CMD_Button(160,129,158,45,29,0, "5");
//	FT800_CMD_Button(160,176,158,45,29,0, "8");
//	FT800_CMD_Button(160,223,158,45,29,0, "0");
//	FT800_CMD_Button(320,82, 158,45,29,0, "3");
//	FT800_CMD_Button(320,129,158,45,29,0, "6");
//	FT800_CMD_Button(320,176,158,45,29,0, "9");
//	FT800_CMD_Button(320,223,158,45,29,0, "Login");	
//	//FT800_CMD_Text(189,10,31, 0, password);
//	
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
//	increase_cmd_offset( 4);								// Update the command pointer

//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
//	increase_cmd_offset( 4);								// Update the command pointer

//	FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());
//		
//	//return event;
//}
