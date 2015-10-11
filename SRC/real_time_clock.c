#include "stm32f4xx.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_pwr.h"
#include "real_time_clock.h"

void rtc_init() {
	
	RTC_InitTypeDef RTC_InitStructure;
  RTC_TimeTypeDef RTC_TimeStructure;
  RTC_DateTypeDef RTC_DateStructure;
	
	/***********************************RTC TEST*************************************************/
	//Enable PWR peripheral clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	//Enable access to backup registers
	PWR_BackupAccessCmd(ENABLE);
	
	//Enable RCC LSI clock and wait until clock is ready
	RCC_LSICmd(ENABLE);
	
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {
		__NOP();
	}
	
	//Set RTC clock source as LSI and enable it
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	RCC_RTCCLKCmd(ENABLE);
	
	//Wait until LSI, APB1 are synchronized
	RTC_WaitForSynchro();
	
	//Write custom data to backup register 0
	RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
	
	//Set the time as 08:00:00 
	RTC_TimeStructure.RTC_Hours   = 0x08;
	RTC_TimeStructure.RTC_Minutes = 0x00;
	RTC_TimeStructure.RTC_Seconds = 0x00;
	
	//Set the date
	RTC_DateStructure.RTC_Month = RTC_Month_July;
	RTC_DateStructure.RTC_Date = 0x01; 
	RTC_DateStructure.RTC_Year = 0x0;
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Monday;
	
	
	//Set clock prescalers: 32768Hz/(0x7F*0xFF)
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_SynchPrediv =  0xFF;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);
	
	
	//Write initial data to RTC registers
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
	
	if (RCC->BDCR & 0x1)
        //printf("LSE On\n");
	__NOP();
    else
        //printf("LSE Off\n");
		__NOP();
 
  if (RCC->BDCR & 0x2)
        //printf("LSE Ready\n");
	__NOP();
    else
        //printf("LSE Not Ready\n");
		__NOP();
 
  if (RCC->BDCR & 0x4)
        //printf("LSE ByPass On\n");
	__NOP();
    else
        //printf("LSE ByPass Off\n");
		__NOP();
 
  if (RCC->BDCR & 0x8000)
        //printf("RTC Clock Enabled\n");
	__NOP();
    else
        //printf("RTC Clock Disabled\n");
		__NOP();
         
  switch(RCC->BDCR & 0x300)
    {
        case 0x100 :
					//puts("RTC Clock Source LSE"); 
				__NOP();
				  break;
        case 0x200 : 
					//puts("RTC Clock Source LSI"); 
				__NOP();
				  break;
        case 0x300 : 
					//printf("RTC Clock Source HSE/%d", (RCC->CFGR >> 16) & 0x1F); 
				__NOP();
				  break;
        default : 
					//puts("RTC Clock Unknown");
				__NOP();
    }
	
		//TEST: Get data
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	
}