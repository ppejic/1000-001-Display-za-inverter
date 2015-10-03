#ifndef __INIT_H
#define __INIT_H

/************************ HEADERS ************************/
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"
#include "cmsis_os.h"

#include "_FT800.h"
#include "events.h"
/************************ FUNCTIONS ************************/
void Init(void);
void Periph_SPI_Init(void);
void Periph_GPIO_Init(void);
void Periph_EXTI_Init(void);
void Periph_RTC_Init(void);

extern void RTX_CANReceive(void const *arg);
extern void RTX_CANSend(void const *arg);
extern void RTX_TouchController(void const *arg);
extern void RTX_ScreenController(void const *arg);
extern void RTX_Watchdog(void const *arg);

/************************ VARIABLES ************************/

#endif
