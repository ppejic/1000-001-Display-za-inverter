#ifndef __UTIL_H
#define __UTIL_H

/*********************************INCLUDES******************************************/
#include "stm32f4xx.h"
#include "cmsis_os.h"
/**********************************DEFINES******************************************/
#define Q_CAN_SIZE											30

/*********************************STRUCTURES****************************************/
typedef struct {
	uint16_t battery_voltage;
	uint8_t  battery_current;
	char     gear_mode;
	uint16_t cap_voltage;
	int16_t  cont_temperature;
	uint16_t motor_voltage;
	uint16_t motor_current;
	uint16_t motor_rpm;
	int16_t  motor_temperature;
	uint16_t motor_torque;	
} admin_vehicle_status_data_t;


typedef union {
	admin_vehicle_status_data_t admin_vehicle_status_data;
} q_can_data_item_t;

/******************************GLOBAL VARIABLES*************************************/

/******************************EXTERN VARIABLES*************************************/
extern osMessageQId q_can;
extern osPoolId 	  q_can_pool;
/****************************FUNCTION PROTOTYPES************************************/

#endif
