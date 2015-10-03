#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "ft800_driver.h"

int main() {
	osThreadDef(__rtx_ft800_task, 	  osPriorityNormal, 1, 0);
		
	osKernelInitialize();
	
	osThreadCreate(osThread(__rtx_ft800_task), NULL);
	
	osKernelStart();
	
	while(1) {
		/* Should never get here */
	}
}
