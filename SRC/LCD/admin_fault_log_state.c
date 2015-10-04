#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "ft800_driver.h"
#include "admin_fault_log_state.h"

void admin_fault_log_state(void) {
	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
	uint8_t i = 0;
	
	while(1)
	{		
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

		FT800_CMD_Button(243, 218, 75, 52, 27, 0, "BACK");
		FT800_CMD_Button(320, 218, 160, 52, 27, 0, "CONFIRM ALL!");
		
		FT800_CMD_Text(10,2,20, 0,"ID");
		FT800_CMD_Text(30,2,20, 0,"Code");
		FT800_CMD_Text(75,2,20, 0,"Description");
		FT800_CMD_Text(201,2,20, 0,"Count");
		FT800_CMD_Text(480,192,28, OPT_RIGHTX,"3000");
		
		for(i = 0; i < 17; i++) {
			FT800_CMD_Text(3,14 + i*16,21,0,"120 9832  Forward switch go...   99");
		}
		
		for(i = 0; i < 11; i++) {
			FT800_CMD_Text(243,i*16,21,0,"120 9832  Forward switch go...   99");
		}
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(255,80,0)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_CMD_Text(244,192,28, 0,"TOTAL COUNT:");
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | 0xffffff); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);

		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), LINE_WIDTH(20)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), BEGIN(LINES)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(239*16,0*16)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(239*16,270*16)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(194*16,0*16)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(194*16,270*16)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(241*16,185*16)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,185*16)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(437*16,0*16)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(437*16,185*16)); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
		increase_cmd_offset( 4);
		
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
		increase_cmd_offset( 4);								// Update the command pointer
	
		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
		increase_cmd_offset( 4);								// Update the command pointer

		FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());					// Update the ring buffer pointer so the graphics processor starts executing	

		osDelay(30);
  }
}
