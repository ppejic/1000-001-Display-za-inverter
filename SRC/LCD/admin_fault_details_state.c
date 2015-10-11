#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "ft800_driver.h"

DEFINE_LCD_STATE(admin_fault_details);

lcd_state_header_t* admin_fault_details_process_input(lcd_state_header_t* lcd_state) {
	//TODO: add button handling
  
}

void admin_fault_details_update_frame(system_state_t* q_can_data) {
	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
	
	do {
		cmd_buffer_read = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
		cmd_buffer_write = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
	} while (cmd_buffer_write != cmd_buffer_read);									// Wait until the two registers match

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
	
	FT800_CMD_Text(2,2,27, 0,"ID: 234");
	FT800_CMD_Text(75,2,27, 0,"Fault code: 8998");
	FT800_CMD_Text(220,2,27, 0,"Datetime: 26 September 14 15:17");
	FT800_CMD_Text(1,20,27, 0,"Fault name(e.g. Two Direction Fault)");
	FT800_CMD_Text(1,43,27, 0,"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	FT800_CMD_Button(0, 230, 239, 39, 27, 0, "Back");
	FT800_CMD_Button(241, 230, 239, 39, 27, 0, "Confirm");
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), LINE_WIDTH(16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), BEGIN(LINES)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(1*16,41*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,41*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
			
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());	
}

//void admin_fault_details_state(void) {
//	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
//	
//	while(1)
//	{		
//		do
//		{
//			cmd_buffer_read = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
//			cmd_buffer_write = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
//		} while (cmd_buffer_write != cmd_buffer_read);									// Wait until the two registers match
//  
//		set_cmd_offset(cmd_buffer_write);															
//    		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_DLSTART));// Start the display list
//		increase_cmd_offset( 4);								// Update the command pointer
//  
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR_RGB | BLACK));
//																												// Set the default clear color to black
//		increase_cmd_offset( 4);								// Update the command pointer

//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
//																												// Clear the screen - this and the previous prevent artifacts between lists
//																												// Attributes are the color, stencil and tag buffers
//		increase_cmd_offset( 4);								// Update the command pointer
//		
//		FT800_CMD_Text(2,2,27, 0,"ID: 234");
//		FT800_CMD_Text(75,2,27, 0,"Fault code: 8998");
//		FT800_CMD_Text(220,2,27, 0,"Datetime: 26 September 14 15:17");
//		FT800_CMD_Text(1,20,27, 0,"Fault name(e.g. Two Direction Fault)");
//		FT800_CMD_Text(1,43,27, 0,"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
//		FT800_CMD_Button(0, 230, 239, 39, 27, 0, "Back");
//		FT800_CMD_Button(241, 230, 239, 39, 27, 0, "Confirm");
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), LINE_WIDTH(16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), BEGIN(LINES)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(1*16,41*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,41*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//				
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
//		increase_cmd_offset( 4);								// Update the command pointer
//	
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
//		increase_cmd_offset( 4);								// Update the command pointer

//		FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());					// Update the ring buffer pointer so the graphics processor starts executing	

//		osDelay(30);
//  }
//}
