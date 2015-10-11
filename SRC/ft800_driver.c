#include "ft800_driver.h"
#include "events.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "cmsis_os.h"
#include "admin_fault_details_state.h"
#include "admin_fault_log_state.h"
#include "admin_main_state.h"
#include "admin_menu_state.h"
#include "admin_vehicle_status_state.h"
#include "main_state.h"
#include "util.h"

uint32_t ramDisplayList;		
uint32_t ramCommandBuffer;
uint8_t FT800Gpio;
//unsigned int cmdOffset = 0x0000;
unsigned int cmdBufferRd = 0x0000;											// Used to navigate command ring buffer
unsigned int cmdBufferWr = 0x0000;

static uint8_t  SPI_Send(uint8_t data);
uint32_t get_cmd_offset(void);
uint32_t increase_cmd_offset(uint32_t command_size);
void set_cmd_offset(uint32_t new_cmd_offset_value);

static void periph_gpio_init() {
	
	GPIO_InitTypeDef GPIOA_InitStruct;
	GPIO_InitTypeDef GPIOB_InitStruct;
	GPIO_InitTypeDef GPIOD_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	/************************ SPI1 PINS ************************/
	GPIOA_InitStruct.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIOA_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
	GPIOA_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIOA_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIOA_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIOA_InitStruct);
	
	/************************ LCD SS and #INT ************************/
	GPIOB_InitStruct.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
	GPIOB_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
	GPIOB_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIOB_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIOB_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIOB_InitStruct);
	
	/************************ ONBOARD LED ************************/
	GPIOD_InitStruct.GPIO_Pin 	= GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_12; 
	GPIOD_InitStruct.GPIO_Mode  = GPIO_Mode_OUT; 		
	GPIOD_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 	
	GPIOD_InitStruct.GPIO_OType = GPIO_OType_PP; 	
	GPIOD_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL; 	
	GPIO_Init(GPIOD, &GPIOD_InitStruct);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	return;
}

static void periph_spi_init() {
	
	SPI_InitTypeDef SPI_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	SPI_InitStruct.SPI_Mode 		 				 = SPI_Mode_Master;
	SPI_InitStruct.SPI_Direction 				 = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_DataSize	 				 = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL			 				 = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA							 = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS			 				 = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_FirstBit 				 = SPI_FirstBit_MSB;
	
	SPI_Init(SPI1, &SPI_InitStruct); 
	SPI_Cmd(SPI1, ENABLE);
	
	return;	
}

static void ft800_init() {
	
	int duty;
	
	GPIO_WriteBit(GPIOB, FT800_PD_N, Bit_SET);
	GPIO_WriteBit(GPIOB, FT800_CS_N, Bit_SET);
	osDelay(20);
	GPIO_WriteBit(GPIOB, FT800_PD_N, Bit_RESET);
	osDelay(20);
	GPIO_WriteBit(GPIOB, FT800_PD_N, Bit_SET);
	osDelay(20);
	
	FT800_Cmd_Write(FT800_ACTIVE);
	osDelay(5);
	FT800_Cmd_Write(FT800_CLKEXT);
	osDelay(5);
	FT800_Cmd_Write(FT800_CLK48M);
	osDelay(5);
	
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
	
//	for(duty = 64; duty <= 128; duty++) {
//		FT800_Mem_Write8(REG_PWM_DUTY, duty);
//		osDelay(5);
//	}
	FT800_Mem_Write8(REG_PWM_DUTY, 128);
	osDelay(15);
	
	FT800_Mem_Write8(REG_INT_MASK, 0x02);
	FT800_Mem_Write8(REG_INT_EN, 	 0x01);
	duty = FT800_Mem_Read8(REG_INT_FLAGS);
	
	return;	
}

static void __ft800_task_init() {

	periph_gpio_init();
	periph_spi_init();
	ft800_init();	
	
	return;
}

void __rtx_ft800_task(void const *arg) {
	osEvent event;
	q_can_data_item_t *item;
	static lcd_state_header_t* state;
	
	//Initialization of __rtx_ft800_task
	__ft800_task_init();
	state = main_state();
	
	while(1) {
		//admin_fault_details_state();
		//FT800_VehicleStatusScreen();
		//admin_fault_log_state();
		//main_state();
		//admin_menu_state();
		//admin_vehicle_status_state();
		//login_state();
		//main_state();	
//			event = osMessageGet(q_can, osWaitForever);
//			if(event.status == osEventMessage) {
//				item = event.value.p;
//				if(item) {
//					__NOP();
//				}
//				osPoolFree(q_can_pool, item);
//			}
		state = state->process_input(state);
		state->update_frame(item);
		
		osDelay(10);
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
	osDelay(20);
	GPIO_WriteBit(GPIOB, FT800_PD_N, Bit_RESET);
	osDelay(20);
	GPIO_WriteBit(GPIOB, FT800_PD_N, Bit_SET);
	osDelay(20);
	
	FT800_Cmd_Write(FT800_ACTIVE);
	osDelay(5);
	FT800_Cmd_Write(FT800_CLKEXT);
	osDelay(5);
	FT800_Cmd_Write(FT800_CLK48M);
	osDelay(5);
	
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
		osDelay(15);
	}
	
	FT800_Mem_Write8(REG_INT_MASK, 0x02);
	FT800_Mem_Write8(REG_INT_EN, 	 0x01);
	duty = FT800_Mem_Read8(REG_INT_FLAGS);
	
	return;
}

void FT800_CMD_Text(uint16_t x, uint16_t y, uint16_t font, uint16_t options, char* s) {
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), CMD_TEXT);
	increase_cmd_offset(4);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), x);
	increase_cmd_offset(2);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), y);
	increase_cmd_offset(2);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), font);
	increase_cmd_offset(2);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), options);
	increase_cmd_offset(2);
	
	while(strlen(s) > 0)
	{
		FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), *s);
		increase_cmd_offset(1);
		s++;
	}
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), 0);
	increase_cmd_offset(1);
	
	if(get_cmd_offset() % 4 == 1) {
		increase_cmd_offset(3);
	}
	else if(get_cmd_offset() % 4 == 2) {
		increase_cmd_offset(2);
	}		
	else if(get_cmd_offset() % 4 == 3)	{
		increase_cmd_offset(1);
	}	
}

void FT800_CMD_Keys(int16_t x, int16_t y, int16_t w, int16_t h,int16_t font, uint16_t options, char* s) {
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), CMD_KEYS);
	increase_cmd_offset(4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), x);
	increase_cmd_offset(2);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), y);
	increase_cmd_offset(2);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), w);
	increase_cmd_offset(2);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), h);
	increase_cmd_offset(2);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), font);
	increase_cmd_offset(2);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), options);
	increase_cmd_offset(2);
	
	while(strlen(s) > 0)
	{
		FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), *s);
		increase_cmd_offset(1);
		s++;
	}
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), 0);
	increase_cmd_offset(1);
	
	if(get_cmd_offset() % 4 == 1) {
		increase_cmd_offset(3);
	}
	else if(get_cmd_offset() % 4 == 2) {
		increase_cmd_offset(2);
	}		
	else if(get_cmd_offset() % 4 == 3)	{
		increase_cmd_offset(1);
	}	
}

void FT800_CMD_BgColor(uint32_t color) {
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), CMD_BGCOLOR);
	increase_cmd_offset(4);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), color);
	increase_cmd_offset(4);
}

void FT800_CMD_GradColor(uint32_t color) {
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), CMD_GRADCOLOR);
	increase_cmd_offset(4);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), color);
	increase_cmd_offset(4);
}

void FT800_CMD_FgColor(uint32_t color) {
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), CMD_FGCOLOR);
	increase_cmd_offset(4);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), color);
	increase_cmd_offset(4);
}

void FT800_CMD_Button(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options, char *s) {
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), CMD_BUTTON);
	increase_cmd_offset(4);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), x);
	increase_cmd_offset(2);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), y);
	increase_cmd_offset(2);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), w);
	increase_cmd_offset(2);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), h);
	increase_cmd_offset(2);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), font);
	increase_cmd_offset(2);
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), options);
	increase_cmd_offset(2);
	
	while(strlen(s) > 0)
	{
		FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), *s);
		increase_cmd_offset(1);
		s++;
	}
	
	FT800_Mem_Write16(RAM_CMD + get_cmd_offset(), 0);
	increase_cmd_offset(1);
	
	if(get_cmd_offset() % 4 == 1)
	{
		increase_cmd_offset(3);
	}
	else if(get_cmd_offset() % 4 == 2)
	{
		increase_cmd_offset(2);
	}		
	else if(get_cmd_offset() % 4 == 3)
	{
		increase_cmd_offset(1);
	}		
}

//void FT800_MainScreen(void) {
//	uint32_t tagval = 0;
//	uint16_t x = 0;
//	uint16_t y = 0;
//	
//	while(1)
//	{		
//		do
//		{
//			cmdBufferRd = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
//			cmdBufferWr = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
//		}while (cmdBufferWr != cmdBufferRd);									// Wait until the two registers match
//  
//		set_cmd_offset(cmdBufferWr);													// The new starting point the first location after the last command
//				
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_DLSTART));// Start the display list
//		increase_cmd_offset(4);								// Update the command pointer
//  
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR_RGB | BLACK));
//																												// Set the default clear color to black
//		increase_cmd_offset(4);								// Update the command pointer


//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
//																												// Clear the screen - this and the previous prevent artifacts between lists
//																												// Attributes are the color, stencil and tag buffers
//		increase_cmd_offset(4);							// Update the command pointer

//		tagval = FT800_Mem_Read32(REG_INT_FLAGS);
//		
//		if((uint32_t)(tagval & (1UL << 31)) == 0)
//		{
//			tagval = FT800_Mem_Read32(REG_TOUCH_TAG_XY);
//			
//			y = tagval & 0x3ff;
//			x = (tagval >> 16) & 0x3ff;
//			
//			if(x > 566 && y > 906)
//			{
//				__NOP();//fault history
//				FT800_Init();
//			}
//		}
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | GREEN); // Indicate to draw a point (dot)
//		increase_cmd_offset(4);	
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_BEGIN | RECTS)); // Indicate to draw a point (dot)
//		increase_cmd_offset(4);	
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(0,0)); // Indicate to draw a point (dot)
//		increase_cmd_offset(4);	
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240*16,136*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset(4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
//		increase_cmd_offset(4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | WHITE); // Indicate to draw a point (dot)
//		increase_cmd_offset(4);	
//		
//		FT800_CMD_Button(266, 0, 214, 31, 29, 0, "               Log");
//				
//		/*************************************************************************/
//		/***************** Nacrtaj najvecu kruznicu ******************************/
//		/*************************************************************************/
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | 0xFF8000); // Indicate to draw a point (dot)
//		increase_cmd_offset(4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), POINT_SIZE(2240)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_BEGIN | FTPOINTS)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);	
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240*16,136*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);	
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | BLACK); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), POINT_SIZE(2160)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_BEGIN | FTPOINTS)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);	
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240*16,136*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);	
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//					
//		/*************************************************************************/
//		/***************** Nacrtaj linije  ******************************/
//		/*************************************************************************/			
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | 0xFF8000); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);

//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), LINE_WIDTH(20)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), BEGIN(LINES)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(0*16,136*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(100*16,136*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(0*16,204*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(120*16,204*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(359*16,204*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,204*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(378*16,136*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,136*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(328*16,31*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,31*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
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
//		
//		/* SPEEDOMETER */
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(255,255,255)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(135 *16,194 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(123 *16,201 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(131 *16,187 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(119 *16,193 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(128 *16,180 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(115 *16,185 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(126 *16,173 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(113 *16,177 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(124 *16,166 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(110 *16,169 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(122 *16,158 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(108 *16,161 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(121 *16,151 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(107 *16,153 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(120 *16,144 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(106 *16,144 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(120 *16,136 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(106 *16,136 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(120 *16,128 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(106 *16,128 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(121 *16,121 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(107 *16,119 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(122 *16,114 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(108 *16,111 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(124 *16,106 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(110 *16,103 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(126 *16,99 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(113 *16,95 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(128 *16,92 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(115 *16,87 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(131 *16,85 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(119 *16,79 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(135 *16,78 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(123 *16,71 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(139 *16,72 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(127 *16,64 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(143 *16,65 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(132 *16,57 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(148 *16,60 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(137 *16,51 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(153 *16,54 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(142 *16,44 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(158 *16,49 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(148 *16,38 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(164 *16,44 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(155 *16,33 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(169 *16,39 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(161 *16,28 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(176 *16,35 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(168 *16,23 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(182 *16,31 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(175 *16,19 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(189 *16,27 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(67,67,67)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(183 *16,15 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(196 *16,24 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(191 *16,11 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(203 *16,22 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(199 *16,9 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(210 *16,20 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(207 *16,6 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(218 *16,18 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(215 *16,4 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(225 *16,17 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(223 *16,3 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(232 *16,16 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(232 *16,2 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240 *16,16 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(240 *16,2 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(248 *16,16 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(248 *16,2 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(255 *16,17 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(257 *16,3 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(262 *16,18 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(265 *16,4 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(270 *16,20 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(273 *16,6 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(277 *16,22 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(281 *16,9 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(284 *16,24 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(289 *16,11 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(291 *16,27 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(297 *16,15 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(298 *16,31 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(305 *16,19 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(304 *16,35 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(312 *16,23 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(311 *16,39 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(319 *16,28 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(316 *16,44 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(325 *16,33 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(322 *16,49 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(332 *16,38 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(327 *16,54 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(338 *16,44 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(332 *16,60 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(343 *16,51 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(337 *16,65 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(348 *16,57 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(341 *16,72 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(353 *16,64 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(345 *16,78 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(357 *16,71 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(349 *16,85 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(361 *16,79 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(352 *16,92 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(365 *16,87 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(354 *16,99 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(367 *16,95 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(356 *16,106 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(370 *16,103 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(358 *16,114 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(372 *16,111 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(359 *16,121 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(373 *16,119 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(360 *16,128 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(374 *16,128 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(360 *16,136 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(374 *16,136 *16));
//		increase_cmd_offset( 4);		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(360 *16,136 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(374 *16,136 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(360 *16,144 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(374 *16,144 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(359 *16,151 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(373 *16,153 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(358 *16,158 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(372 *16,161 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(356 *16,166 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(370 *16,169 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(354 *16,173 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(367 *16,177 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(352 *16,180 *16));
//		increase_cmd_offset( 4);
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(365 *16,185 *16));
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//				
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | WHITE); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		
//		FT800_CMD_Text(1, 140, 26, 0, "Battery Current");	
//		FT800_CMD_Text(21, 157, 31, 0, "4 A");	

//		FT800_CMD_Text(13, 208, 26, 0, "Battery Volts");
//			
//		FT800_CMD_Text(5, 226, 31, 0, "78.4 V");
//		
//		FT800_CMD_Text(404, 140, 26, 0, "AC Motor");	
//		FT800_CMD_Text(410, 152, 26, 0, "Current");	
//		FT800_CMD_Text(409, 167, 30, 0, "7 A");

//		FT800_CMD_Text(335, 244, 26, 0, "26 February 14 15:17");		
//		
//		FT800_CMD_Text(194, 90, 31, 0, "1124");
//		
//		FT800_CMD_Text(25, 50, 29, 0, "100%");
//		
//		FT800_CMD_Text(194, 139, 31, 0, "D");
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(67,67,67)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_CMD_Text(229, 139, 31, 0, "N");
//		
//		FT800_CMD_Text(264, 139, 31, 0, "R");
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(255,80,0)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_CMD_Text(293, 117, 26, 0, "rpm");
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), COLOR_RGB(255,0,0)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		
//		FT800_CMD_Text(430, 73, 31, 0, "!");
//			

//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
//		increase_cmd_offset( 4);								// Update the command pointer
//	
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
//		increase_cmd_offset( 4);								// Update the command pointer

//		FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());					// Update the ring buffer pointer so the graphics processor starts executing	
//		
//		osDelay(10);
//	}
//}

//void FT800_LoginScreen(void) {
//	
//	//uint32_t tagval = 0;
//	//uint16_t x = 0;
//	//uint16_t y = 0;
//	
//	while(1)
//	{		
//		do
//		{
//			cmdBufferRd = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
//			cmdBufferWr = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
//		}while (cmdBufferWr != cmdBufferRd);									// Wait until the two registers match
//  
//		set_cmd_offset(cmdBufferWr);															// The new starting point the first location after the last command
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
//		FT800_CMD_Keys(1,82, 478,35,23,0, "qwertyuiop");
//		FT800_CMD_Keys(27,120,432,35,23, 0, "asdfghjkl");
//		FT800_CMD_Button(2,158,85,35,21,0,"Caps Lock");
//		FT800_CMD_Keys(91,158,308,35,23, 0, "zxcvbnm");
//		FT800_CMD_Button(401,158,78,35,21,0,"Delete");
//		FT800_CMD_Button(2,196,38,35,20,0,"&123");
//		FT800_CMD_Button(43,196,357,35,20,0,"");
//		FT800_CMD_Button(403,196,18,35,24,0,".");
//		FT800_CMD_Button(424,196,53,35,21,0,"Enter");
//		FT800_CMD_Button(2,236,237,35,21,0,"Main Screen");
//		FT800_CMD_Button(243,236,237,35,21,0,"LOGOUT");
//		
//		FT800_CMD_Text(129,10,31, 0,"Text|");
//		//FT800_CMD_Button(50,122,300,35,23,0,"");
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
//		increase_cmd_offset( 4);								// Update the command pointer
//	
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
//		increase_cmd_offset( 4);								// Update the command pointer

//		FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());
//		
//		osDelay(10);
//	}
//}

//void FT800_VehicleStatusScreen(void) {
//	
//	uint32_t tagval = 0;
//	uint16_t x, y;
//	
//	while(1)
//	{		
//		do
//		{
//			cmdBufferRd = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
//			cmdBufferWr = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
//		}while (cmdBufferWr != cmdBufferRd);									// Wait until the two registers match
//  
//		set_cmd_offset(cmdBufferWr);															// The new starting point the first location after the last command
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

//		tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
//		if((uint32_t)(tagval & (1UL << 31)) == 0)
//		{
//			y = tagval & 0x3ff;
//			x = (tagval >> 16) & 0x3ff;
//			
//			if(y > 0 && y < 210)
//			{
//				__NOP();//fault history
//				//ft800_init();
//				//MenuScreen();
//			}
//		}
//		
//		FT800_CMD_Text(0, 0, 29, 0, "Battery");
//		FT800_CMD_Text(248, 0, 29, 0, "48.8 V");
//		FT800_CMD_Text(480, 0, 29, OPT_RIGHTX, "4 A");
//		
//		FT800_CMD_Text(0, 22, 29, 0, "Traction drive state");
//		FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "DRIVE");
//		
//		FT800_CMD_Text(0, 48, 29, 0, "Cap. Volts");
//		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, "48.8 V");
//		
//		FT800_CMD_Text(0, 70, 29, 0, "Cont. Temp.");
//		FT800_CMD_Text(480, 70, 29, OPT_RIGHTX, "13 C");
//		
//		FT800_CMD_Text(0, 92, 29, 0, "Motor Volts");
//		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, "46.0 C");
//		
//		FT800_CMD_Text(0, 114, 29, 0, "Motor Amps");
//		FT800_CMD_Text(480, 114, 29, OPT_RIGHTX, "5 A");
//		
//		FT800_CMD_Text(0, 136, 29, 0, "Motor RPM");
//		FT800_CMD_Text(480, 136, 29, OPT_RIGHTX, "780 RPM");
//		
//		FT800_CMD_Text(0, 158, 29, 0, "Motor Temperature");
//		FT800_CMD_Text(480, 158, 29, OPT_RIGHTX, "22 C");
//		
//		FT800_CMD_Text(0, 180, 29, 0, "Motor Torque");
//		FT800_CMD_Text(480, 180, 29, OPT_RIGHTX, "0.0 Nm");
//		
//		FT800_CMD_Button(0, 210, 480, 62, 30, 0, "Back");
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | 0x00ffff); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);

//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), LINE_WIDTH(20)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), BEGIN(LINES)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(0*16,50*16)); // Indicate to draw a point (dot)
//		increase_cmd_offset( 4);
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,50*16)); // Indicate to draw a point (dot)
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

//		osDelay(10);
//	}
//}

//void FT800_AdminMenuScreen(void) {
//	
//	uint32_t tagval = 0;
//	uint16_t x, y;
//	
//	while(1)
//	{		
//		do
//		{
//			cmdBufferRd = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
//			cmdBufferWr = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
//		}while (cmdBufferWr != cmdBufferRd);									// Wait until the two registers match
//  
//		set_cmd_offset(cmdBufferWr);															
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

//		tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
//		if((uint32_t)(tagval & (1UL << 31)) == 0)
//		{
//			y = tagval & 0x3ff;
//			x = (tagval >> 16) & 0x3ff;
//			
//			if(y > 0 && y < 256)
//			{
//				__NOP();//fault log
//			}
//			else if(y > 256 && y < 512)
//			{
//				__NOP();//test
//			}
//			else if(y > 512 && y < 768)
//			{
//				__NOP();//vehicle status
//			}
//			else if(y > 768 && y < 1023)
//			{
//				__NOP();//main
//			}
//		}
//			
//		FT800_CMD_Button(0, 0, 480, 68, 31, 0, "Main");
//		
//		FT800_CMD_Button(0, 68, 480, 68, 31, 0, "Vehicle Status");
//	
//		FT800_CMD_Button(0, 136, 480, 68, 31, 0, "Test");
//		
//		FT800_CMD_Button(0, 204, 480, 68, 31, 0, "Fault Log");
//			
//		
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
//		increase_cmd_offset( 4);								// Update the command pointer
//	
//		FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
//		increase_cmd_offset( 4);								// Update the command pointer

//		FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());					// Update the ring buffer pointer so the graphics processor starts executing	

//		osDelay(10);
//  }
//}

static uint32_t cmd_offset = 0;

uint32_t get_cmd_offset(void) {
	return cmd_offset;
}

void set_cmd_offset(uint32_t new_cmd_offset_value) {
	 cmd_offset = new_cmd_offset_value;
}

uint32_t increase_cmd_offset(uint32_t command_size) {
	uint32_t new_offset;
	
	new_offset = cmd_offset + command_size;
	
	if(new_offset > 4095) {
		new_offset = new_offset - 4096;
	}
	
	cmd_offset = new_offset;
	
	return cmd_offset;
}

////uint32_t incCMDOffset(uint32_t currentOffset, uint8_t commandSize) {
////	
////	static uint32_t newOffset;															// Used to hold new offset
////	
////	newOffset = currentOffset + commandSize;						// Calculate new offset
////	
////	if(newOffset > 4095)																// If new offset past boundary...
////	{
////			newOffset = (newOffset - 4096);									// ... roll over pointer
////	}
////	
////	return newOffset;	
////	
////}



