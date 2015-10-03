#include "_FT800.h"
#include "events.h"

uint32_t ramDisplayList;		
uint32_t ramCommandBuffer;
uint8_t FT800Gpio;
unsigned int cmdOffset = 0x0000;
unsigned int cmdBufferRd = 0x0000;											// Used to navigate command ring buffer
unsigned int cmdBufferWr = 0x0000;

static uint8_t  SPI_Send(uint8_t data);

void RTX_ScreenController(void const *arg) {
	
	uint8_t screenId;
	
	while(1) {
		 
		 FT800_VehicleStatusScreen();
	 }
}

uint8_t SPI_Send(uint8_t data) {
	
	SPI1->DR = data;
	while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); 
	while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); 
	while( SPI1->SR & SPI_I2S_FLAG_BSY ); 
	
	return SPI1->DR; 
}

uint32_t FT800_Mem_Read8(uint32_t ftAddress) {
	
	uint32_t ftData8 = 0;
	uint8_t  cTempAddr[3];
	uint8_t  cZeroFill = 0;
	int i;
	
	cTempAddr[2] = (uint8_t) (ftAddress >> 16) | MEM_READ;
	cTempAddr[1] = (char) (ftAddress >> 8);	
	cTempAddr[0] = (char) (ftAddress);	
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_RESET);
	
	for (i = 2; i >= 0; i--) {
		SPI_Send(cTempAddr[i]);
	}
	
	ftData8 = SPI_Send(cZeroFill);
	ftData8 = SPI_Send(cZeroFill);
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_SET);
	
	return ftData8;
}

uint32_t FT800_Mem_Read16(uint32_t ftAddress) {
	
	uint32_t ftData16;																// 16-bits to return
	uint8_t cTempAddr[3];														// FT800 Memory Address
	uint8_t cTempData[2];														// Place-holder for 16-bits being read
	uint8_t cZeroFill;
	int i, j;
	
	cTempAddr[2] = (char) (ftAddress >> 16) | MEM_READ;		// Compose the command and address to send
	cTempAddr[1] = (char) (ftAddress >> 8);								// middle byte
	cTempAddr[0] = (char) (ftAddress);										// low byte

	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_RESET);

  for (i = 2; i >= 0; i--)
	{
		SPI_Send(cTempAddr[i]);
	}

  SPI_Send(cZeroFill);

	for (j = 0; j < 2; j++)	{
		cTempData[j] = SPI_Send(cZeroFill);
	}

	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_SET);

	ftData16 = (cTempData[1]<< 8) | 											// Compose value to return - high byte
					   (cTempData[0]); 														// low byte

  return ftData16;	
}

uint32_t FT800_Mem_Read32(uint32_t ftAddress) {
	
	uint32_t ftData32 = 0;
	uint8_t  cTempAddr[3];
	uint8_t  cTempData[4];
	uint8_t  cZeroFill = 0;
	int i, j;
	
	cTempAddr[2] = (uint8_t) (ftAddress >> 16) | MEM_READ;
	cTempAddr[1] = (char) (ftAddress >> 8);	
	cTempAddr[0] = (char) (ftAddress);	
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_RESET);
	
	for (i = 2; i >= 0; i--) {
		SPI_Send(cTempAddr[i]);
	}
	
	SPI_Send(cZeroFill);
	
	for (j = 0; j < 4; j++) {
		cTempData[j] = SPI_Send(cZeroFill);
	}
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_SET);
	
	ftData32 = (cTempData[3]<< 24) | 						
						 (cTempData[2]<< 16) | 
						 (cTempData[1]<< 8) | 
						 (cTempData[0]);
	
	return ftData32;
}

void FT800_Mem_Write8(uint32_t ftAddress, uint8_t ftData8) {
	
	uint8_t cTempAddr[3];
	int i;
	
	cTempAddr[2] = (char) (ftAddress >> 16) | MEM_WRITE;
	cTempAddr[1] = (char) (ftAddress >> 8);								
	cTempAddr[0] = (char) (ftAddress);
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_RESET);
	
	for (i = 2; i >= 0; i--)
	{
		SPI_Send(cTempAddr[i]);
	}
	
	SPI_Send(ftData8);
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_SET);
}

void FT800_Mem_Write16(uint32_t ftAddress, uint32_t ftData16) {
	
	uint8_t cTempAddr[3];
	uint8_t cTempData[2];
	int i,j;
	
	cTempAddr[2] = (char) (ftAddress >> 16) | MEM_WRITE;	
	cTempAddr[1] = (char) (ftAddress >> 8);								
	cTempAddr[0] = (char) (ftAddress);										
	
	cTempData[1] = (char) (ftData16 >> 8);								
	cTempData[0] = (char) (ftData16);
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_RESET);
	
	for (i = 2; i >= 0; i--) {
		SPI_Send(cTempAddr[i]);
	}
	
	for (j = 0; j < 2; j++) {
		SPI_Send(cTempData[j]);
	}
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_SET);
}

void FT800_Mem_Write32(uint32_t ftAddress, uint32_t ftData32) {
	
	uint8_t cTempAddr[3];
	uint8_t cTempData[4];
	int i,j;
	
	cTempAddr[2] = (char) (ftAddress >> 16) | MEM_WRITE;	
	cTempAddr[1] = (char) (ftAddress >> 8);								
	cTempAddr[0] = (char) (ftAddress);	
	
	cTempData[3] = (char) (ftData32 >> 24);		
	cTempData[2] = (char) (ftData32 >> 16);	
	cTempData[1] = (char) (ftData32 >> 8);
	cTempData[0] = (char) (ftData32);	
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_RESET);
	
	for (i = 2; i >= 0; i--) {
		SPI_Send(cTempAddr[i]);
	}
	
	for (j = 0; j < 4; j++) {
		SPI_Send(cTempData[j]);
	}
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_SET);
}

void FT800_Cmd_Write(uint8_t ftCommand) {
	
	uint8_t cZero = 0x00;
	
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_RESET);
	SPI_Send(ftCommand);
	SPI_Send(cZero);
	SPI_Send(cZero);
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_SET);
}

void FT800_Init(void) {
	
	int duty;
	
	GPIO_WriteBit(GPIOB, FT800_PD_N, Bit_SET);
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_SET);
	Delay(20);
	GPIO_WriteBit(GPIOB, FT800_PD_N, Bit_RESET);
	Delay(20);
	GPIO_WriteBit(GPIOB, FT800_PD_N, Bit_SET);
	Delay(20);
	
	FT800_Cmd_Write(FT800_ACTIVE);
	Delay(5);
	FT800_Cmd_Write(FT800_CLKEXT);
	Delay(5);
	FT800_Cmd_Write(FT800_CLK48M);
	Delay(5);
	
	while(FT800_Mem_Read8(REG_ID) != 0x7C) {
		__NOP();
	}
	
	FT800_Mem_Write8(REG_PCLK, 		 0);
	FT800_Mem_Write8(REG_PWM_DUTY, 0);
	
	FT800_Mem_Write16(REG_HSIZE,     LCD_WIDTH);
	FT800_Mem_Write16(REG_HCYCLE,    LCD_HCYCLE);
	FT800_Mem_Write16(REG_HOFFSET,   LCD_HOFFSET);
	FT800_Mem_Write16(REG_HSYNC0,    LCD_HSYNC0);
	FT800_Mem_Write16(REG_HSYNC1,    LCD_HSYNC1);
	FT800_Mem_Write16(REG_VSIZE,     LCD_HEIGHT);
	FT800_Mem_Write16(REG_VCYCLE,    LCD_VCYCLE);
	FT800_Mem_Write16(REG_VOFFSET,   LCD_VOFFSET);
	FT800_Mem_Write16(REG_VSYNC0,    LCD_VSYNC0);
	FT800_Mem_Write16(REG_VSYNC1,    LCD_VSYNC1);
	FT800_Mem_Write16(REG_SWIZZLE,   LCD_SWIZZLE);
	FT800_Mem_Write16(REG_PCLK_POL,  LCD_PCLKPOL);
	
	FT800_Mem_Write8(REG_TOUCH_MODE, 			TOUCH_MODE_CONTINUOUS);
	FT800_Mem_Write16(REG_TOUCH_RZTHRESH, RZTHRESH_DEFAULT);
	
	/* Turn recorded audio volume down, synthesizer volume down and mute */
	FT800_Mem_Write8 (REG_VOL_PB, 	 0);
	FT800_Mem_Write8 (REG_VOL_SOUND, 0);
	FT800_Mem_Write16(REG_SOUND, 		 0x6000); 
	
	ramDisplayList = RAM_DL;
	FT800_Mem_Write32(ramDisplayList, DL_CLEAR_RGB);
	ramDisplayList += 4;
	FT800_Mem_Write32(ramDisplayList, (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
	ramDisplayList += 4;
	FT800_Mem_Write32(ramDisplayList, DL_DISPLAY);
	
	FT800_Mem_Write32(REG_DLSWAP, DLSWAP_FRAME);
	ramDisplayList = RAM_DL;
	
	FT800Gpio = FT800_Mem_Read8(REG_GPIO);
	FT800Gpio = FT800Gpio | 0x80;
	FT800_Mem_Write8(REG_GPIO, FT800Gpio);
	FT800_Mem_Write8(REG_PCLK, LCD_PCLK);
	
	for(duty = 64; duty <= 128; duty++) {
		FT800_Mem_Write8(REG_PWM_DUTY, duty);
		Delay(15);
	}
	
	FT800_Mem_Write8(REG_INT_MASK, 0x02);
	FT800_Mem_Write8(REG_INT_EN, 	 0x01);
	duty = FT800_Mem_Read8(REG_INT_FLAGS);
	
	return;
}

void FT800_CMD_Text(uint16_t x, uint16_t y, uint16_t font, uint16_t options, char* s) {
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, CMD_TEXT);
	cmdOffset = incCMDOffset(cmdOffset, 4);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, x);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, y);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, font);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, options);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	while(strlen(s) > 0)
	{
		FT800_Mem_Write16(RAM_CMD + cmdOffset, *s);
		cmdOffset = incCMDOffset(cmdOffset, 1);
		s++;
	}
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, 0);
	cmdOffset = incCMDOffset(cmdOffset, 1);
	
	if(cmdOffset % 4 == 1) {
		cmdOffset = incCMDOffset(cmdOffset, 3);
	}
	else if(cmdOffset % 4 == 2) {
		cmdOffset = incCMDOffset(cmdOffset, 2);
	}		
	else if(cmdOffset % 4 == 3)	{
		cmdOffset = incCMDOffset(cmdOffset, 1);
	}	
}

void FT800_CMD_Keys(int16_t x, int16_t y, int16_t w, int16_t h,int16_t font, uint16_t options, char* s) {
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, CMD_KEYS);
	cmdOffset = incCMDOffset(cmdOffset, 4);
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, x);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, y);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, w);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, h);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, font);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, options);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	while(strlen(s) > 0)
	{
		FT800_Mem_Write16(RAM_CMD + cmdOffset, *s);
		cmdOffset = incCMDOffset(cmdOffset, 1);
		s++;
	}
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, 0);
	cmdOffset = incCMDOffset(cmdOffset, 1);
	
	if(cmdOffset % 4 == 1) {
		cmdOffset = incCMDOffset(cmdOffset, 3);
	}
	else if(cmdOffset % 4 == 2) {
		cmdOffset = incCMDOffset(cmdOffset, 2);
	}		
	else if(cmdOffset % 4 == 3)	{
		cmdOffset = incCMDOffset(cmdOffset, 1);
	}	
}

void FT800_CMD_BgColor(uint32_t color) {
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, CMD_BGCOLOR);
	cmdOffset = incCMDOffset(cmdOffset, 4);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, color);
	cmdOffset = incCMDOffset(cmdOffset, 4);
}

void FT800_CMD_GradColor(uint32_t color) {
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, CMD_GRADCOLOR);
	cmdOffset = incCMDOffset(cmdOffset, 4);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, color);
	cmdOffset = incCMDOffset(cmdOffset, 4);
}

void FT800_CMD_FgColor(uint32_t color) {
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, CMD_FGCOLOR);
	cmdOffset = incCMDOffset(cmdOffset, 4);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, color);
	cmdOffset = incCMDOffset(cmdOffset, 4);
}

void FT800_CMD_Button(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, char *s) {
	
	FT800_Mem_Write32(RAM_CMD + cmdOffset, CMD_BUTTON);
	cmdOffset = incCMDOffset(cmdOffset, 4);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, x);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, y);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, w);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, h);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, font);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, options);
	cmdOffset = incCMDOffset(cmdOffset, 2);
	
	while(strlen(s) > 0)
	{
		FT800_Mem_Write16(RAM_CMD + cmdOffset, *s);
		cmdOffset = incCMDOffset(cmdOffset, 1);
		s++;
	}
	
	FT800_Mem_Write16(RAM_CMD + cmdOffset, 0);
	cmdOffset = incCMDOffset(cmdOffset, 1);
	
	if(cmdOffset % 4 == 1)
	{
		cmdOffset = incCMDOffset(cmdOffset, 3);
	}
	else if(cmdOffset % 4 == 2)
	{
		cmdOffset = incCMDOffset(cmdOffset, 2);
	}		
	else if(cmdOffset % 4 == 3)
	{
		cmdOffset = incCMDOffset(cmdOffset, 1);
	}		
}

void FT800_MainScreen(void) {
	
	uint32_t tagval = 0;
	uint16_t x = 0;
	uint16_t y = 0;
	
	while(1)
	{		
		do
		{
			cmdBufferRd = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
			cmdBufferWr = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
		}while (cmdBufferWr != cmdBufferRd);									// Wait until the two registers match
  
		cmdOffset = cmdBufferWr;															// The new starting point the first location after the last command
				
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (CMD_DLSTART));// Start the display list
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer
  
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_CLEAR_RGB | BLACK));
																												// Set the default clear color to black
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer


		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
																												// Clear the screen - this and the previous prevent artifacts between lists
																												// Attributes are the color, stencil and tag buffers
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer

		tagval = FT800_Mem_Read32(REG_INT_FLAGS);
		
		if((uint32_t)(tagval & (1UL << 31)) == 0)
		{
			tagval = FT800_Mem_Read32(REG_TOUCH_TAG_XY);
			
			y = tagval & 0x3ff;
			x = (tagval >> 16) & 0x3ff;
			
			if(x > 566 && y > 906)
			{
				__NOP();//fault history
				FT800_Init();
			}
		}
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_COLOR_RGB | GREEN); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);	
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_BEGIN | RECTS)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);	
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(0,0)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);	
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(240*16,136*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_END); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_COLOR_RGB | WHITE); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);	
		
		FT800_CMD_Button(266, 0, 214, 31, 29, 0, "               Log");
				
		/*************************************************************************/
		/***************** Nacrtaj najvecu kruznicu ******************************/
		/*************************************************************************/
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_COLOR_RGB | 0xFF8000); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, POINT_SIZE(2240)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_BEGIN | FTPOINTS)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);	
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(240*16,136*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);	
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_END); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_COLOR_RGB | BLACK); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, POINT_SIZE(2160)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_BEGIN | FTPOINTS)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);	
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(240*16,136*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);	
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_END); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
					
		/*************************************************************************/
		/***************** Nacrtaj linije  ******************************/
		/*************************************************************************/			
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_COLOR_RGB | 0xFF8000); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);

		FT800_Mem_Write32(RAM_CMD + cmdOffset, LINE_WIDTH(20)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, BEGIN(LINES)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(0*16,136*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(100*16,136*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(0*16,204*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(120*16,204*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(359*16,204*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(480*16,204*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(378*16,136*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(480*16,136*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(328*16,31*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(480*16,31*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, COLOR_RGB(255,0,0)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(434*16,61*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(402*16,119*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(402*16,119*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(470*16,119*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(435*16,61*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(470*16,118*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		/* SPEEDOMETER */
		FT800_Mem_Write32(RAM_CMD + cmdOffset, COLOR_RGB(255,255,255)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(135 *16,194 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(123 *16,201 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(131 *16,187 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(119 *16,193 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(128 *16,180 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(115 *16,185 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(126 *16,173 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(113 *16,177 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(124 *16,166 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(110 *16,169 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(122 *16,158 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(108 *16,161 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(121 *16,151 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(107 *16,153 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(120 *16,144 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(106 *16,144 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(120 *16,136 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(106 *16,136 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(120 *16,128 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(106 *16,128 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(121 *16,121 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(107 *16,119 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(122 *16,114 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(108 *16,111 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(124 *16,106 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(110 *16,103 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(126 *16,99 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(113 *16,95 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(128 *16,92 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(115 *16,87 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(131 *16,85 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(119 *16,79 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(135 *16,78 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(123 *16,71 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(139 *16,72 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(127 *16,64 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(143 *16,65 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(132 *16,57 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(148 *16,60 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(137 *16,51 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(153 *16,54 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(142 *16,44 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(158 *16,49 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(148 *16,38 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(164 *16,44 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(155 *16,33 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(169 *16,39 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(161 *16,28 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(176 *16,35 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(168 *16,23 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(182 *16,31 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(175 *16,19 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(189 *16,27 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, COLOR_RGB(67,67,67)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(183 *16,15 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(196 *16,24 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(191 *16,11 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(203 *16,22 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(199 *16,9 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(210 *16,20 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(207 *16,6 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(218 *16,18 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(215 *16,4 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(225 *16,17 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(223 *16,3 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(232 *16,16 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(232 *16,2 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(240 *16,16 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(240 *16,2 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(248 *16,16 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(248 *16,2 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(255 *16,17 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(257 *16,3 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(262 *16,18 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(265 *16,4 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(270 *16,20 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(273 *16,6 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(277 *16,22 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(281 *16,9 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(284 *16,24 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(289 *16,11 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(291 *16,27 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(297 *16,15 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(298 *16,31 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(305 *16,19 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(304 *16,35 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(312 *16,23 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(311 *16,39 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(319 *16,28 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(316 *16,44 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(325 *16,33 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(322 *16,49 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(332 *16,38 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(327 *16,54 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(338 *16,44 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(332 *16,60 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(343 *16,51 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(337 *16,65 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(348 *16,57 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(341 *16,72 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(353 *16,64 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(345 *16,78 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(357 *16,71 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(349 *16,85 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(361 *16,79 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(352 *16,92 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(365 *16,87 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(354 *16,99 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(367 *16,95 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(356 *16,106 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(370 *16,103 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(358 *16,114 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(372 *16,111 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(359 *16,121 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(373 *16,119 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(360 *16,128 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(374 *16,128 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(360 *16,136 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(374 *16,136 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(360 *16,136 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(374 *16,136 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(360 *16,144 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(374 *16,144 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(359 *16,151 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(373 *16,153 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(358 *16,158 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(372 *16,161 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(356 *16,166 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(370 *16,169 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(354 *16,173 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(367 *16,177 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(352 *16,180 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(365 *16,185 *16));
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_END); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
				
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_COLOR_RGB | WHITE); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		
		FT800_CMD_Text(1, 140, 26, 0, "Battery Current");	
		FT800_CMD_Text(21, 157, 31, 0, "4 A");	

		FT800_CMD_Text(13, 208, 26, 0, "Battery Volts");
			
		FT800_CMD_Text(5, 226, 31, 0, "78.4 V");
		
		FT800_CMD_Text(404, 140, 26, 0, "AC Motor");	
		FT800_CMD_Text(410, 152, 26, 0, "Current");	
		FT800_CMD_Text(409, 167, 30, 0, "7 A");

		FT800_CMD_Text(335, 244, 26, 0, "26 February 14 15:17");		
		
		FT800_CMD_Text(194, 90, 31, 0, "1124");
		
		FT800_CMD_Text(25, 50, 29, 0, "100%");
		
		FT800_CMD_Text(194, 139, 31, 0, "D");
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, COLOR_RGB(67,67,67)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_CMD_Text(229, 139, 31, 0, "N");
		
		FT800_CMD_Text(264, 139, 31, 0, "R");
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, COLOR_RGB(255,80,0)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_CMD_Text(293, 117, 26, 0, "rpm");
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, COLOR_RGB(255,0,0)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		
		FT800_CMD_Text(430, 73, 31, 0, "!");
			

		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_DISPLAY));		// Instruct the graphics processor to show the list
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer
	
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (CMD_SWAP));			// Make this list active
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer

		FT800_Mem_Write16(REG_CMD_WRITE, (cmdOffset));					// Update the ring buffer pointer so the graphics processor starts executing	
		
		Delay(10);
	}
}

void FT800_LoginScreen(void) {
	
	uint32_t tagval = 0;
	uint16_t x = 0;
	uint16_t y = 0;
	
	while(1)
	{		
		do
		{
			cmdBufferRd = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
			cmdBufferWr = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
		}while (cmdBufferWr != cmdBufferRd);									// Wait until the two registers match
  
		cmdOffset = cmdBufferWr;															// The new starting point the first location after the last command
				
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (CMD_DLSTART));// Start the display list
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer
  
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_CLEAR_RGB | BLACK));
																												// Set the default clear color to black
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer


		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
																												// Clear the screen - this and the previous prevent artifacts between lists
																												// Attributes are the color, stencil and tag buffers
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer
				
		FT800_CMD_Keys(1,82, 478,35,23,0, "qwertyuiop");
		FT800_CMD_Keys(27,120,432,35,23, 0, "asdfghjkl");
		FT800_CMD_Button(2,158,85,35,21,0,"Caps Lock");
		FT800_CMD_Keys(91,158,308,35,23, 0, "zxcvbnm");
		FT800_CMD_Button(401,158,78,35,21,0,"Delete");
		FT800_CMD_Button(2,196,38,35,20,0,"&123");
		FT800_CMD_Button(43,196,357,35,20,0,"");
		FT800_CMD_Button(403,196,18,35,24,0,".");
		FT800_CMD_Button(424,196,53,35,21,0,"Enter");
		FT800_CMD_Button(2,236,237,35,21,0,"Main Screen");
		FT800_CMD_Button(243,236,237,35,21,0,"LOGOUT");
		
		FT800_CMD_Text(129,10,31, 0,"Text|");
		//FT800_CMD_Button(50,122,300,35,23,0,"");
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_DISPLAY));		// Instruct the graphics processor to show the list
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer
	
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (CMD_SWAP));			// Make this list active
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer

		FT800_Mem_Write16(REG_CMD_WRITE, (cmdOffset));
		
		Delay(10);
	}
}

void FT800_VehicleStatusScreen(void) {
	
	uint32_t tagval = 0;
	uint16_t x, y;
	
	while(1)
	{		
		do
		{
			cmdBufferRd = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
			cmdBufferWr = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
		}while (cmdBufferWr != cmdBufferRd);									// Wait until the two registers match
  
		cmdOffset = cmdBufferWr;															// The new starting point the first location after the last command
    		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (CMD_DLSTART));// Start the display list
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer
  
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_CLEAR_RGB | BLACK));
																												// Set the default clear color to black
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer


		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
																												// Clear the screen - this and the previous prevent artifacts between lists
																												// Attributes are the color, stencil and tag buffers
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer

		tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
		if((uint32_t)(tagval & (1UL << 31)) == 0)
		{
			y = tagval & 0x3ff;
			x = (tagval >> 16) & 0x3ff;
			
			if(y > 0 && y < 210)
			{
				__NOP();//fault history
				//ft800_init();
				//MenuScreen();
			}
		}
		
		FT800_CMD_Text(0, 0, 29, 0, "Battery");
		FT800_CMD_Text(248, 0, 29, 0, "48.8 V");
		FT800_CMD_Text(480, 0, 29, OPT_RIGHTX, "4 A");
		
		FT800_CMD_Text(0, 22, 29, 0, "Traction drive state");
		FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "DRIVE");
		
		FT800_CMD_Text(0, 48, 29, 0, "Cap. Volts");
		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, "48.8 V");
		
		FT800_CMD_Text(0, 70, 29, 0, "Cont. Temp.");
		FT800_CMD_Text(480, 70, 29, OPT_RIGHTX, "13 C");
		
		FT800_CMD_Text(0, 92, 29, 0, "Motor Volts");
		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, "46.0 C");
		
		FT800_CMD_Text(0, 114, 29, 0, "Motor Amps");
		FT800_CMD_Text(480, 114, 29, OPT_RIGHTX, "5 A");
		
		FT800_CMD_Text(0, 136, 29, 0, "Motor RPM");
		FT800_CMD_Text(480, 136, 29, OPT_RIGHTX, "780 RPM");
		
		FT800_CMD_Text(0, 158, 29, 0, "Motor Temperature");
		FT800_CMD_Text(480, 158, 29, OPT_RIGHTX, "22 C");
		
		FT800_CMD_Text(0, 180, 29, 0, "Motor Torque");
		FT800_CMD_Text(480, 180, 29, OPT_RIGHTX, "0.0 Nm");
		
		FT800_CMD_Button(0, 210, 480, 62, 30, 0, "Back");
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_COLOR_RGB | 0x00ffff); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);

		FT800_Mem_Write32(RAM_CMD + cmdOffset, LINE_WIDTH(20)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, BEGIN(LINES)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(0*16,50*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, VERTEX2F(480*16,50*16)); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, DL_END); // Indicate to draw a point (dot)
		cmdOffset = incCMDOffset(cmdOffset, 4);
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_DISPLAY));		// Instruct the graphics processor to show the list
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer
	
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (CMD_SWAP));			// Make this list active
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer

		FT800_Mem_Write16(REG_CMD_WRITE, (cmdOffset));					// Update the ring buffer pointer so the graphics processor starts executing			

		Delay(10);
	}
}

void FT800_AdminMenuScreen(void) {
	
	uint32_t tagval = 0;
	uint16_t x, y;
	
	while(1)
	{		
		do
		{
			cmdBufferRd = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
			cmdBufferWr = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
		}while (cmdBufferWr != cmdBufferRd);									// Wait until the two registers match
  
		cmdOffset = cmdBufferWr;															
    		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (CMD_DLSTART));// Start the display list
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer
  
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_CLEAR_RGB | BLACK));
																												// Set the default clear color to black
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer


		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
																												// Clear the screen - this and the previous prevent artifacts between lists
																												// Attributes are the color, stencil and tag buffers
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer

		tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
		if((uint32_t)(tagval & (1UL << 31)) == 0)
		{
			y = tagval & 0x3ff;
			x = (tagval >> 16) & 0x3ff;
			
			if(y > 0 && y < 256)
			{
				__NOP();//fault log
			}
			else if(y > 256 && y < 512)
			{
				__NOP();//test
			}
			else if(y > 512 && y < 768)
			{
				__NOP();//vehicle status
			}
			else if(y > 768 && y < 1023)
			{
				__NOP();//main
			}
		}
			
		FT800_CMD_Button(0, 0, 480, 68, 31, 0, "Main");
		
		FT800_CMD_Button(0, 68, 480, 68, 31, 0, "Vehicle Status");
	
		FT800_CMD_Button(0, 136, 480, 68, 31, 0, "Test");
		
		FT800_CMD_Button(0, 204, 480, 68, 31, 0, "Fault Log");
			
		
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (DL_DISPLAY));		// Instruct the graphics processor to show the list
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer
	
		FT800_Mem_Write32(RAM_CMD + cmdOffset, (CMD_SWAP));			// Make this list active
		cmdOffset = incCMDOffset(cmdOffset, 4);								// Update the command pointer

		FT800_Mem_Write16(REG_CMD_WRITE, (cmdOffset));					// Update the ring buffer pointer so the graphics processor starts executing	

		Delay(10);
  }
}

unsigned int incCMDOffset(unsigned int currentOffset, unsigned char commandSize) {
	
	unsigned int newOffset;															// Used to hold new offset
	
	newOffset = currentOffset + commandSize;						// Calculate new offset
	
	if(newOffset > 4095)																// If new offset past boundary...
	{
			newOffset = (newOffset - 4096);									// ... roll over pointer
	}
	
	return newOffset;	
	
}



