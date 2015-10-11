#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "can_driver.h"
#include "util.h"

//TODO: implement CAN_RX updates

void __rtx_can_task(void const *arg) {
	
//	//TODO: Add init
	q_can_data_item_t *can_data;
	
	g_system_state.battery_soc     = 89;
	g_system_state.battery_voltage = 101;
	g_system_state.battery_current = 100;
	g_system_state.gear_mode = 'N';
	g_system_state.cap_voltage = 12;
	g_system_state.cont_temperature = 231;
	g_system_state.motor_voltage = 563;
	g_system_state.motor_current = 123;
	g_system_state.motor_rpm = 2133;
	g_system_state.motor_temperature = 121;
	g_system_state.motor_torque = 12;
	
	while(1) {
		
				
		//osMessagePut(q_can, (uint32_t)(can_data), 100);
		
		osDelay(1000);
	}
	
}
