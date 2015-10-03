#include "main.h"

int main() {
	
	osThreadDef(RTX_Init, osPriorityNormal, 1, 0);
	
	osKernelInitialize();
	
	osThreadCreate(osThread(RTX_Init), NULL);
	
	osKernelStart();
	
	while(1) {
		/* Should never get here */
	}
}
