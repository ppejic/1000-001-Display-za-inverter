#include "_init.h"

void RTX_Init(void const *arg) {
	
	//osThreadDef(RTX_CANReceive, 		  osPriorityNormal, 1, 0);
	//osThreadDef(RTX_CANSend, 				  osPriorityNormal, 1, 0);
	//osThreadDef(RTX_TouchController,  osPriorityNormal, 1, 0);
	osThreadDef(RTX_ScreenController, osPriorityNormal, 1, 0);
	//osThreadDef(RTX_Watchdog, 			  osPriorityNormal, 1, 0);
	
	Timer1ms_Init();	
	Periph_GPIO_Init();
	Periph_SPI_Init();
	FT800_Init();
	//Periph_EXTI_Init();
	//Periph_RTC_Init();
	
	/************************ TURN ONBOARD LED ON************************/
	GPIO_WriteBit(GPIOD, GPIO_Pin_12, Bit_SET);
	GPIO_WriteBit(GPIOD, GPIO_Pin_13, Bit_SET);
	GPIO_WriteBit(GPIOD, GPIO_Pin_14, Bit_SET);
	GPIO_WriteBit(GPIOD, GPIO_Pin_15, Bit_SET);
	
	//osThreadCreate(osThread(RTX_CANReceive), 			 NULL);
	//osThreadCreate(osThread(RTX_CANSend), 				 NULL);
	//osThreadCreate(osThread(RTX_TouchController),  NULL);
	osThreadCreate(osThread(RTX_ScreenController), NULL);
	//osThreadCreate(osThread(RTX_Watchdog), 				 NULL);
	
	osThreadTerminate(osThreadGetId());
}

void Periph_GPIO_Init(void) {
	
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

void Periph_SPI_Init(void) {
	
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

void Periph_EXTI_Init(void) {
	
	EXTI_InitTypeDef EXTI_InitStructure;		
	GPIO_InitTypeDef GPIOA_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIOA_InitStruct.GPIO_Pin 	= GPIO_Pin_8;
	GPIOA_InitStruct.GPIO_Mode  = GPIO_Mode_IN; 		
	GPIOA_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 	
	GPIOA_InitStruct.GPIO_OType = GPIO_OType_PP; 	
	GPIOA_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP; 	
	GPIO_Init(GPIOD, &GPIOA_InitStruct);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);
	
	EXTI_InitStructure.EXTI_Line    = EXTI_Line8;
  EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel 								   = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd 							 = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void Periph_RTC_Init(void) {
	
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_InitTypeDef RTC_InitStruct;
	RTC_DateTypeDef RTC_DateStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	PWR_BackupAccessCmd(ENABLE);
	
	/* Config RTC */
	RCC_LSICmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
	
	RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div25);
	RCC_RTCCLKCmd(ENABLE);
	RTC_WaitForSynchro();
	
	RTC_TimeStruct.RTC_Hours = 1;
	RTC_TimeStruct.RTC_Minutes = 1;
	RTC_TimeStruct.RTC_Seconds = 1;
	/* Fill date */
	RTC_DateStruct.RTC_Date = 2;
	RTC_DateStruct.RTC_Month = 2;
	RTC_DateStruct.RTC_Year = 0;
	RTC_DateStruct.RTC_WeekDay = 2;
	
	/* Set the RTC time base to 1s and hours format to 24h */
	RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;
	RTC_InitStruct.RTC_AsynchPrediv = 124;
	RTC_InitStruct.RTC_SynchPrediv = 2559;
	RTC_Init(&RTC_InitStruct);
	
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStruct);
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStruct);	
}


