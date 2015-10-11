#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "ft800_driver.h"
#include "admin_menu_state.h"

DEFINE_LCD_STATE(admin_menu);

lcd_state_header_t* admin_menu_process_input(lcd_state_header_t* lcd_state) {
	uint32_t tagval;
	uint16_t x, y;

	tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
	if((uint32_t)(tagval & (1UL << 31)) == 0) {
		y = tagval & 0x3ff;
		
		if(y >= CONVERTCOORDINATES_Y(270-54)) {
			//Main
			//event = EV_ADMINMAIN;
			__NOP();
			return admin_main_state();
		}				
		else if(y >= CONVERTCOORDINATES_Y(270-2*54) && y <= CONVERTCOORDINATES_Y(270-54)) {
			//Vehicle status
			//event = EV_ADMINVEHICLESTATUS;
			__NOP();
			return admin_vehicle_status_state();
		}
		else if(y >= CONVERTCOORDINATES_Y(270-3*54) && y <= CONVERTCOORDINATES_Y(270-2*54)) {
			//Test
			//event = EV_ADMINTEST;
			__NOP();
		}
		else if(y >= CONVERTCOORDINATES_Y(270-4*54) && y <= CONVERTCOORDINATES_Y(270-3*54)) {
			//Fault Log
			//event = EV_ADMINFAULTLOG;
			__NOP();
			return admin_fault_log_state();
		}
		else {
			//Log out
			//event = EV_LOGOUT;
			__NOP();
			return main_state();
		}
	}
	
	return lcd_state;
	
}

void admin_menu_update_frame(system_state_t* q_can_data) {
	
	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
			
	do
	{
		cmd_buffer_read = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
		cmd_buffer_write = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
	}while (cmd_buffer_write != cmd_buffer_read);									// Wait until the two registers match

	set_cmd_offset(cmd_buffer_write);															
			
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_DLSTART));// Start the display list
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR_RGB | BLACK));
																											// Set the default clear color to black
	increase_cmd_offset( 4);								// Update the command pointer


	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
																											// Clear the screen - this and the previous prevent artifacts between lists
																											// Attributes are the color, stencil and tag buffers
	increase_cmd_offset( 4);								// Update the command pointer
	
	FT800_CMD_Button(0, 0, 480, 54, 29, 0, "Main");
	FT800_CMD_Button(0, 54, 480, 54, 29, 0, "Vehicle Status");	
	FT800_CMD_Button(0, 108, 480, 54, 29, 0, "Test");	
	FT800_CMD_Button(0, 162, 480, 54, 29, 0, "Fault Log");
	FT800_CMD_Button(0, 216, 480, 54, 29, 0, "Log out");
			
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());					// Update the ring buffer pointer so the graphics processor starts executing	
		
	//return event;
}

//void admin_menu_state(void) {
//	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
//			
//	do
//	{
//		cmd_buffer_read = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
//		cmd_buffer_write = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
//	}while (cmd_buffer_write != cmd_buffer_read);									// Wait until the two registers match

//	set_cmd_offset(cmd_buffer_write);															
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

////	tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
////	if((uint32_t)(tagval & (1UL << 31)) == 0) {
////		y = tagval & 0x3ff;
////		
////		if(y >= CONVERTCOORDINATES_Y(270-54)) {
////			//Main
////			event = EV_ADMINMAIN;
////		}				
////		else if(y >= CONVERTCOORDINATES_Y(270-2*54) && y <= CONVERTCOORDINATES_Y(270-54)) {
////			//Vehicle status
////			event = EV_ADMINVEHICLESTATUS;
////		}
////		else if(y >= CONVERTCOORDINATES_Y(270-3*54) && y <= CONVERTCOORDINATES_Y(270-2*54)) {
////			//Test
////			event = EV_ADMINTEST;
////		}
////		else if(y >= CONVERTCOORDINATES_Y(270-4*54) && y <= CONVERTCOORDINATES_Y(270-3*54)) {
////			//Fault Log
////			event = EV_ADMINFAULTLOG;
////		}
////		else {
////			//Log out
////			event = EV_LOGOUT;
////		}
////	}
//	
//	FT800_CMD_Button(0, 0, 480, 54, 29, 0, "Main");
//	FT800_CMD_Button(0, 54, 480, 54, 29, 0, "Vehicle Status");	
//	FT800_CMD_Button(0, 108, 480, 54, 29, 0, "Test");	
//	FT800_CMD_Button(0, 162, 480, 54, 29, 0, "Fault Log");
//	FT800_CMD_Button(0, 216, 480, 54, 29, 0, "Log out");
//			
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
//	increase_cmd_offset( 4);								// Update the command pointer

//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
//	increase_cmd_offset( 4);								// Update the command pointer

//	FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());					// Update the ring buffer pointer so the graphics processor starts executing	
//		
//	//return event;
//}
