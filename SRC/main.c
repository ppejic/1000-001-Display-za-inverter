#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "ft800_driver.h"
#include "can_driver.h"
#include "util.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_pwr.h"
#include "real_time_clock.h"

osMessageQId q_can;
osMessageQDef(q_can, Q_CAN_SIZE, q_can_data_item_t);

osPoolId q_can_pool;
osPoolDef(q_can_pool, Q_CAN_SIZE, q_can_data_item_t);

int main() {
	uint32_t status;
	
	rtc_init();
	
	osThreadDef(__rtx_ft800_task, 	  osPriorityNormal, 1, 0);
	osThreadDef(__rtx_can_task, 	    osPriorityNormal, 1, 0);
		
	osKernelInitialize();
	
	q_can 		 = osMessageCreate(osMessageQ(q_can), NULL);
	q_can_pool = osPoolCreate(osPool(q_can_pool));
	osThreadCreate(osThread(__rtx_ft800_task), NULL);
	osThreadCreate(osThread(__rtx_can_task), NULL);
	
	osKernelStart();
	
	while(1) {
		/* Should never get here */
	}
}
