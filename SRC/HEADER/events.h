#ifndef __EVENTS_H
#define __EVENTS_H

/************************ DEFINES ************************/

/************************ HEADERS ************************/
#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include "stm32f4xx_rcc.h"
/************************ FUNCTIONS ************************/
void Delay(uint32_t time);
void TIM2_IRQHandler(void);
void Timer1ms_Init(void);
#endif
