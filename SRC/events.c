#include "events.h"
#include "_FT800.h"
#include "stm32f4xx_rtc.h"

static void 		TimingDelay_Decrement(void);
static uint32_t	TimingDelay;
uint32_t 				reg_int_flags;

void TIM2_IRQHandler(void) {

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        TimingDelay_Decrement();
  }
}

void EXTI9_5_IRQHandler(void) {
	
	if(EXTI_GetITStatus(EXTI_Line8) != RESET) {
		
		EXTI_ClearITPendingBit(EXTI_Line8);
		reg_int_flags = FT800_Mem_Read32(REG_INT_FLAGS);
	}
}

void TimingDelay_Decrement(void) {
	
	if(TimingDelay != 0UL) {
		TimingDelay--;
	}
}

void Delay(uint32_t time) {
	
	TimingDelay = time;
	
	while(TimingDelay != 0UL);
}

void Timer1ms_Init(void) {
		
	TIM_TimeBaseInitTypeDef timerInitStructure;
	NVIC_InitTypeDef 			  nvicStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	timerInitStructure.TIM_Prescaler 					= 0;
	timerInitStructure.TIM_CounterMode 	  		= TIM_CounterMode_Up;
	timerInitStructure.TIM_Period 						= 42000;
	timerInitStructure.TIM_ClockDivision 			= TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter  = 0;
	TIM_TimeBaseInit(TIM2, &timerInitStructure);
	TIM_Cmd(TIM2, ENABLE);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	nvicStructure.NVIC_IRQChannel 									= TIM2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority 			  = 1;
	nvicStructure.NVIC_IRQChannelCmd 								= ENABLE;
	NVIC_Init(&nvicStructure);
}




