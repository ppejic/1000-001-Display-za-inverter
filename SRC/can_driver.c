#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "can_driver.h"
#include "util.h"

void __rtx_can_task(void const *arg) {
	
//	//TODO: Add init
//	q_can_data_item *data;
	
	while(1) {
//		data = osPoolAlloc(q_can_pool);
//		data->admin_vehicle_status_data.battery_current = 10;
//		data->admin_vehicle_status_data.battery_voltage = 11;
//		data->admin_vehicle_status_data.cap_voltage = 12;
//				
//		osMessagePut(q_can, (uint32_t)(data), osWaitForever);
//		
		
		
		osDelay(500);
	}
	
}
