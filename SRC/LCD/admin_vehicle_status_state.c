#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "ft800_driver.h"
#include "stdio.h"

DEFINE_LCD_STATE(admin_vehicle_status);

lcd_state_header_t* admin_vehicle_status_process_input(lcd_state_header_t* lcd_state) {
	//TODO: add button handling
  uint32_t tagval;
	uint16_t y;
	
	tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
	if((uint32_t)(tagval & (1UL << 31)) == 0)
	{
		y = tagval & 0x3ff;		
		
		if(y <= CONVERTCOORDINATES_Y(270-210))
		{
			//event = EV_BACK;
			__NOP();
			return admin_menu_state();
		}
	}
	
	return lcd_state;
}

void admin_vehicle_status_update_frame(system_state_t* q_can_data) {
	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
	//SCREEN_EVENT event;		
	uint32_t tagval = 0;
	uint16_t y;
	char battery_voltage[6];
	char motorVoltage[6];
	char battery_current[5];
	char motorRPM[7];
	char temperature[5];
	char capVoltage[6];
	char num[2];
	char num2[1];
	
	//event = EV_NONE;
	
	do
	{
		cmd_buffer_read = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
		cmd_buffer_write = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
	}while (cmd_buffer_write != cmd_buffer_read);									// Wait until the two registers match

	set_cmd_offset(cmd_buffer_write);														// The new starting point the first location after the last command
			
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_DLSTART));// Start the display list
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR_RGB | BLACK));
																											// Set the default clear color to black
	increase_cmd_offset( 4);								// Update the command pointer


	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
																											// Clear the screen - this and the previous prevent artifacts between lists
																											// Attributes are the color, stencil and tag buffers
	increase_cmd_offset( 4);								// Update the command pointer


	
	FT800_CMD_Text(0, 0, 29, 0, "Battery");
	if(g_system_state.battery_voltage >= 100 && 
		 g_system_state.battery_voltage < 1000) {
		snprintf(num, 3, "%d", g_system_state.battery_voltage/10);
		strcpy(battery_voltage, num);
		strcat(battery_voltage, ".");
		snprintf(num2, 2, "%d", g_system_state.battery_voltage%10);
		strcat(battery_voltage, num2);
		strcat(battery_voltage, " V");
		
		FT800_CMD_Text(248, 0, 29, 0, battery_voltage);
	}
	else if(g_system_state.battery_voltage >= 10 && 
		g_system_state.battery_voltage){		
		snprintf(num, 2, "%d", g_system_state.battery_voltage/10);
		strcpy(battery_voltage, num);
		strcat(battery_voltage, ".");
		snprintf(num2, 2, "%d", g_system_state.battery_voltage%10);
		strcat(battery_voltage, num2);
		strcat(battery_voltage, " V");
		
		FT800_CMD_Text(248, 0, 29, 0, battery_voltage);
	}
	else if(g_system_state.battery_voltage > 0 && 
		g_system_state.battery_voltage < 10) {		
		strcpy(battery_voltage, "0.");
		snprintf(num2, 2, "%d", g_system_state.battery_voltage%10);
		strcat(battery_voltage, num2);
		strcat(battery_voltage, " V");
		
		FT800_CMD_Text(248, 0, 29, 0, battery_voltage);
	}
	else {
		FT800_CMD_Text(248, 0, 29, 0, "NA/N");
	}
	
	if(g_system_state.battery_current < 100 && g_system_state.battery_current >= 10) {
		snprintf(num, 2, "%d", g_system_state.battery_current);
		strcpy(battery_current, num);
		strcat(battery_current, ".");
		
		snprintf(num, 2, "%d", g_system_state.battery_current%10);
		strcat(battery_current, num);
		strcat(battery_current, " A\0");
		FT800_CMD_Text(480, 0, 29, OPT_RIGHTX, battery_current);
	}
	else if(g_system_state.battery_current > 0 && g_system_state.battery_current <= 10) {
		strcpy(battery_current, "0.");		
		snprintf(num, 2, "%d", g_system_state.battery_current%10);
		strcat(battery_current, num);
		
		strcat(battery_current, " A\0");
		FT800_CMD_Text(480, 0, 29, OPT_RIGHTX, battery_current);
	}
	else {
		FT800_CMD_Text(480, 0, 29, OPT_RIGHTX, "NA/N");
	}
	
	FT800_CMD_Text(0, 22, 29, 0, "Traction drive state");
	switch(g_system_state.gear_mode) {
		case 'D': {
			FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "DRIVE");		
		} break;
		case 'N': {	
			FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "NEUTRAL");		
		} break;
		case 'R': {
			FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "REVERSE");		
		} break;
		default: {
			FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "NA/N");
		}
	}
	
	FT800_CMD_Text(0, 48, 29, 0, "Cap. Volts");
	if(g_system_state.cap_voltage >= 100 && g_system_state.cap_voltage < 1000) {		
		snprintf(num, 3, "%d", g_system_state.cap_voltage/10);
		strcpy(capVoltage, num);
		strcat(capVoltage, ".");
		snprintf(num2, 2, "%d", g_system_state.cap_voltage%10);
		strcat(capVoltage, num2);
		strcat(capVoltage, " V");
		
		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, capVoltage);
	}
	else if(g_system_state.cap_voltage >= 10 && g_system_state.cap_voltage < 100){		
		snprintf(num, 2, "%d", g_system_state.cap_voltage/10);
		strcpy(capVoltage, num);
		strcat(capVoltage, ".");
		snprintf(num2, 2, "%d", g_system_state.cap_voltage%10);
		strcat(capVoltage, num2);
		strcat(capVoltage, " V");
		
		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, capVoltage);
	}
	else if(g_system_state.cap_voltage > 0 && g_system_state.cap_voltage < 10) {		
		strcpy(capVoltage, "0.");
		snprintf(num2, 2, "%d", g_system_state.cap_voltage%10);
		strcat(capVoltage, num2);
		strcat(capVoltage, " V");
		
		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, capVoltage);
	}
	else {
		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, "NA/N");
	}
	
	FT800_CMD_Text(0, 70, 29, 0, "Cont. Temp.");
	if(g_system_state.cont_temperature >= -100 && g_system_state.cont_temperature < 300) {
		snprintf(temperature, 4, "%d", g_system_state.cont_temperature);
		strcat(temperature, " C");
		FT800_CMD_Text(480, 70, 29, OPT_RIGHTX, temperature);
	}
	else {
		FT800_CMD_Text(480, 70, 29, OPT_RIGHTX, "ERROR");
	}	
	
	/*********************************** MOTOR VOLTAGE ******************************************/
	FT800_CMD_Text(0, 92, 29, 0, "Motor Voltage");
	if(g_system_state.motor_voltage >= 100 && g_system_state.motor_voltage < 1000) {
		snprintf(num, 3, "%d", g_system_state.motor_voltage/10);
		strcpy(motorVoltage, num);
		strcat(motorVoltage, ".");
		snprintf(num2, 2, "%d", g_system_state.motor_voltage%10);
		strcat(motorVoltage, num2);
		strcat(motorVoltage, " V");
		
		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, motorVoltage);
	}
	else if(g_system_state.motor_voltage >= 10 && g_system_state.motor_voltage < 100){		
		snprintf(num, 2, "%d", g_system_state.motor_voltage/10);
		strcpy(motorVoltage, num);
		strcat(motorVoltage, ".");
		snprintf(num2, 2, "%d", g_system_state.motor_voltage%10);
		strcat(motorVoltage, num2);
		strcat(motorVoltage, " V");
		
		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, motorVoltage);
	}
	else if(g_system_state.motor_voltage > 0 && g_system_state.motor_voltage < 10) {		
		strcpy(motorVoltage, "0.");
		snprintf(num2, 2, "%d", g_system_state.motor_voltage%10);
		strcat(motorVoltage, num2);
		strcat(motorVoltage, " V");
		
		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, motorVoltage);
	}
	else {
		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, "NA/N");
	}
	
	FT800_CMD_Text(0, 114, 29, 0, "Motor Current");
	if(g_system_state.motor_current < 100 && g_system_state.motor_current >= 10) {
		snprintf(num, 2, "%d", g_system_state.motor_current);
		strcpy(battery_current, num);
		strcat(battery_current, ".");
		
		snprintf(num, 2, "%d", g_system_state.motor_current%10);
		strcat(battery_current, num);
		strcat(battery_current, " A\0");
		FT800_CMD_Text(480, 114, 29, OPT_RIGHTX, battery_current);
	}
	else if(g_system_state.motor_current > 0 && g_system_state.motor_current <= 10) {
		strcpy(battery_current, "0.");		
		snprintf(num, 2, "%d", g_system_state.motor_current%10);
		strcat(battery_current, num);
		
		strcat(battery_current, " A\0");
		FT800_CMD_Text(480, 114, 29, OPT_RIGHTX, battery_current);
	}
	else {
		FT800_CMD_Text(480, 114, 29, OPT_RIGHTX, "NA/N A");
	}		
	
	FT800_CMD_Text(0, 136, 29, 0, "Motor RPM");
	if(g_system_state.motor_rpm > 0 && g_system_state.motor_rpm <= 3000) {
		sprintf(motorRPM, "%d", g_system_state.motor_rpm);
		strcat(motorRPM, " RPM");
		FT800_CMD_Text(480, 136, 29, OPT_RIGHTX, motorRPM);
	}
	else {
		FT800_CMD_Text(480, 136, 29, OPT_RIGHTX, "NA/N");
	}		
	
	FT800_CMD_Text(0, 158, 29, 0, "Motor Temperature");
	if(g_system_state.cont_temperature >= -100 && g_system_state.cont_temperature < 300) {
		char temperature[5];
		snprintf(temperature, 4, "%d", g_system_state.cont_temperature);
		strcat(temperature, " C");
		FT800_CMD_Text(480, 158, 29, OPT_RIGHTX, temperature);
	}
	else {
		FT800_CMD_Text(480, 158, 29, OPT_RIGHTX, "ERROR");
	}
	
	FT800_CMD_Text(0, 180, 29, 0, "Motor Torque");
	FT800_CMD_Text(480, 180, 29, OPT_RIGHTX, "0.0 Nm");	
	FT800_CMD_Button(0, 210, 480, 60, 30, 0, "Back");
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | 0x00ffff); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), LINE_WIDTH(20)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), BEGIN(LINES)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(0*16,50*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,50*16)); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
	increase_cmd_offset( 4);
	
	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
	increase_cmd_offset( 4);								// Update the command pointer

	FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());					// Update the ring buffer pointer so the graphics processor starts executing			
	
	//return event;
}

//void admin_vehicle_status_state(void) { 
//	uint32_t cmd_buffer_read = 0, cmd_buffer_write = 0;
//	//SCREEN_EVENT event;		
//	uint32_t tagval = 0;
//	uint16_t y;
//	char batteryVoltage[6];
//	char motorVoltage[6];
//	char batteryCurrent[5];
//	char motorRPM[7];
//	char temperature[5];
//	char capVoltage[6];
//	char num[2];
//	char num2[1];
//	
//	//event = EV_NONE;
//	
//	do
//	{
//		cmd_buffer_read = FT800_Mem_Read16(REG_CMD_READ);					// Read the graphics processor read pointer
//		cmd_buffer_write = FT800_Mem_Read16(REG_CMD_WRITE); 				// Read the graphics processor write pointer
//	}while (cmd_buffer_write != cmd_buffer_read);									// Wait until the two registers match

//	set_cmd_offset(cmd_buffer_write);														// The new starting point the first location after the last command
//			
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_DLSTART));// Start the display list
//	increase_cmd_offset( 4);								// Update the command pointer

//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR_RGB | BLACK));
//																											// Set the default clear color to black
//	increase_cmd_offset( 4);								// Update the command pointer


//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
//																											// Clear the screen - this and the previous prevent artifacts between lists
//																											// Attributes are the color, stencil and tag buffers
//	increase_cmd_offset( 4);								// Update the command pointer

//	tagval = FT800_Mem_Read32(REG_TOUCH_DIRECT_XY);
//	if((uint32_t)(tagval & (1UL << 31)) == 0)
//	{
//		y = tagval & 0x3ff;		
//		
////		if(y <= CONVERTCOORDINATES_Y(270-210))
////		{
////			event = EV_BACK;
////		}
//	}
//	
//	FT800_CMD_Text(0, 0, 29, 0, "Battery");
////	if(inputData.batteryVoltage >= 100 && inputData.batteryVoltage < 1000) {
////		snprintf(num, 3, "%d", inputData.batteryVoltage/10);
////		strcpy(batteryVoltage, num);
////		strcat(batteryVoltage, ".");
////		snprintf(num2, 2, "%d", inputData.batteryVoltage%10);
////		strcat(batteryVoltage, num2);
////		strcat(batteryVoltage, " V");
////		
////		FT800_CMD_Text(248, 0, 29, 0, batteryVoltage);
////	}
////	else if(inputData.batteryVoltage >= 10 && inputData.batteryVoltage < 100){		
////		snprintf(num, 2, "%d", inputData.batteryVoltage/10);
////		strcpy(batteryVoltage, num);
////		strcat(batteryVoltage, ".");
////		snprintf(num2, 2, "%d", inputData.batteryVoltage%10);
////		strcat(batteryVoltage, num2);
////		strcat(batteryVoltage, " V");
////		
////		FT800_CMD_Text(248, 0, 29, 0, batteryVoltage);
////	}
////	else if(inputData.batteryVoltage > 0 && inputData.batteryVoltage < 10) {		
////		strcpy(batteryVoltage, "0.");
////		snprintf(num2, 2, "%d", inputData.batteryVoltage%10);
////		strcat(batteryVoltage, num2);
////		strcat(batteryVoltage, " V");
////		
////		FT800_CMD_Text(248, 0, 29, 0, batteryVoltage);
////	}
////	else {
////		FT800_CMD_Text(248, 0, 29, 0, "99.9 V");
////	}
//	
////	if(inputData.batteryCurrent < 100 && inputData.batteryCurrent >= 10) {
////		snprintf(num, 2, "%d", inputData.batteryCurrent);
////		strcpy(batteryCurrent, num);
////		strcat(batteryCurrent, ".");
////		
////		snprintf(num, 2, "%d", inputData.batteryCurrent%10);
////		strcat(batteryCurrent, num);
////		strcat(batteryCurrent, " A\0");
////		FT800_CMD_Text(480, 0, 29, OPT_RIGHTX, batteryCurrent);
////	}
////	else if(inputData.batteryCurrent > 0 && inputData.batteryCurrent <= 10) {
////		strcpy(batteryCurrent, "0.");		
////		snprintf(num, 2, "%d", inputData.batteryCurrent%10);
////		strcat(batteryCurrent, num);
////		
////		strcat(batteryCurrent, " A\0");
////		FT800_CMD_Text(480, 0, 29, OPT_RIGHTX, batteryCurrent);
////	}
////	else {
////		FT800_CMD_Text(480, 0, 29, OPT_RIGHTX, "9.9 A");
////	}
//	
//	FT800_CMD_Text(0, 22, 29, 0, "Traction drive state");
////	switch(inputData.gearMode) {
////		case 'D': {
////			FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "DRIVE");		
////		} break;
////		case 'N': {	
////			FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "NEUTRAL");		
////		} break;
////		case 'R': {
////			FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "REVERSE");		
////		} break;
////		default: {
////			FT800_CMD_Text(480, 22, 29, OPT_RIGHTX, "NONE");
////		}
////	}
//	
//	FT800_CMD_Text(0, 48, 29, 0, "Cap. Volts");
////	if(inputData.capVoltage >= 100 && inputData.capVoltage < 1000) {		
////		snprintf(num, 3, "%d", inputData.capVoltage/10);
////		strcpy(capVoltage, num);
////		strcat(capVoltage, ".");
////		snprintf(num2, 2, "%d", inputData.capVoltage%10);
////		strcat(capVoltage, num2);
////		strcat(capVoltage, " V");
////		
////		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, capVoltage);
////	}
////	else if(inputData.capVoltage >= 10 && inputData.capVoltage < 100){		
////		snprintf(num, 2, "%d", inputData.capVoltage/10);
////		strcpy(capVoltage, num);
////		strcat(capVoltage, ".");
////		snprintf(num2, 2, "%d", inputData.capVoltage%10);
////		strcat(capVoltage, num2);
////		strcat(capVoltage, " V");
////		
////		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, capVoltage);
////	}
////	else if(inputData.capVoltage > 0 && inputData.capVoltage < 10) {		
////		strcpy(capVoltage, "0.");
////		snprintf(num2, 2, "%d", inputData.capVoltage%10);
////		strcat(capVoltage, num2);
////		strcat(capVoltage, " V");
////		
////		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, capVoltage);
////	}
////	else {
////		FT800_CMD_Text(480, 48, 29, OPT_RIGHTX, "99.9 V");
////	}
//	
//	FT800_CMD_Text(0, 70, 29, 0, "Cont. Temp.");
////	if(inputData.contTemperature >= -100 && inputData.contTemperature < 300) {
////		snprintf(temperature, 4, "%d", inputData.contTemperature);
////		strcat(temperature, " C");
////		FT800_CMD_Text(480, 70, 29, OPT_RIGHTX, temperature);
////	}
////	else {
////		FT800_CMD_Text(480, 70, 29, OPT_RIGHTX, "ERROR");
////	}	
//	
//	/*********************************** MOTOR VOLTAGE ******************************************/
//	FT800_CMD_Text(0, 92, 29, 0, "Motor Voltage");
////	if(inputData.motorVoltage >= 100 && inputData.motorVoltage < 1000) {
////		snprintf(num, 3, "%d", inputData.motorVoltage/10);
////		strcpy(motorVoltage, num);
////		strcat(motorVoltage, ".");
////		snprintf(num2, 2, "%d", inputData.motorVoltage%10);
////		strcat(motorVoltage, num2);
////		strcat(motorVoltage, " V");
////		
////		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, motorVoltage);
////	}
////	else if(inputData.motorVoltage >= 10 && inputData.motorVoltage < 100){		
////		snprintf(num, 2, "%d", inputData.motorVoltage/10);
////		strcpy(motorVoltage, num);
////		strcat(motorVoltage, ".");
////		snprintf(num2, 2, "%d", inputData.motorVoltage%10);
////		strcat(motorVoltage, num2);
////		strcat(motorVoltage, " V");
////		
////		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, motorVoltage);
////	}
////	else if(inputData.motorVoltage > 0 && inputData.motorVoltage < 10) {		
////		strcpy(motorVoltage, "0.");
////		snprintf(num2, 2, "%d", inputData.motorVoltage%10);
////		strcat(motorVoltage, num2);
////		strcat(motorVoltage, " V");
////		
////		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, motorVoltage);
////	}
////	else {
////		FT800_CMD_Text(480, 92, 29, OPT_RIGHTX, "99.9 V");
////	}
//	
//	FT800_CMD_Text(0, 114, 29, 0, "Motor Amps");
////	if(inputData.motorCurrent < 100 && inputData.motorCurrent >= 10) {
////		snprintf(num, 2, "%d", inputData.motorCurrent);
////		strcpy(batteryCurrent, num);
////		strcat(batteryCurrent, ".");
////		
////		snprintf(num, 2, "%d", inputData.motorCurrent%10);
////		strcat(batteryCurrent, num);
////		strcat(batteryCurrent, " A\0");
////		FT800_CMD_Text(480, 114, 29, OPT_RIGHTX, batteryCurrent);
////	}
////	else if(inputData.motorCurrent > 0 && inputData.motorCurrent <= 10) {
////		strcpy(batteryCurrent, "0.");		
////		snprintf(num, 2, "%d", inputData.motorCurrent%10);
////		strcat(batteryCurrent, num);
////		
////		strcat(batteryCurrent, " A\0");
////		FT800_CMD_Text(480, 114, 29, OPT_RIGHTX, batteryCurrent);
////	}
////	else {
////		FT800_CMD_Text(480, 114, 29, OPT_RIGHTX, "9.9 A");
////	}		
//	
//	FT800_CMD_Text(0, 136, 29, 0, "Motor RPM");
////	if(inputData.motorRPM > 0 && inputData.motorRPM <= 3000) {
////		sprintf(motorRPM, "%d", inputData.motorRPM);
////		strcat(motorRPM, " RPM");
////		FT800_CMD_Text(480, 136, 29, OPT_RIGHTX, motorRPM);
////	}
////	else {
////		FT800_CMD_Text(480, 136, 29, OPT_RIGHTX, "2000 RPM");
////	}		
//	
//	FT800_CMD_Text(0, 158, 29, 0, "Motor Temperature");
////	if(inputData.contTemperature >= -100 && inputData.contTemperature < 300) {
////		char temperature[5];
////		snprintf(temperature, 4, "%d", inputData.motorTemperature);
////		strcat(temperature, " C");
////		FT800_CMD_Text(480, 158, 29, OPT_RIGHTX, temperature);
////	}
////	else {
////		FT800_CMD_Text(480, 158, 29, OPT_RIGHTX, "ERROR");
////	}
//	
//	FT800_CMD_Text(0, 180, 29, 0, "Motor Torque");
//	FT800_CMD_Text(480, 180, 29, OPT_RIGHTX, "0.0 Nm");	
//	FT800_CMD_Button(0, 210, 480, 60, 30, 0, "Back");
//	
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_COLOR_RGB | 0x00ffff); // Indicate to draw a point (dot)
//	increase_cmd_offset( 4);

//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), LINE_WIDTH(20)); // Indicate to draw a point (dot)
//	increase_cmd_offset( 4);
//	
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), BEGIN(LINES)); // Indicate to draw a point (dot)
//	increase_cmd_offset( 4);
//	
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(0*16,50*16)); // Indicate to draw a point (dot)
//	increase_cmd_offset( 4);
//	
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), VERTEX2F(480*16,50*16)); // Indicate to draw a point (dot)
//	increase_cmd_offset( 4);
//	
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), DL_END); // Indicate to draw a point (dot)
//	increase_cmd_offset( 4);
//	
//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (DL_DISPLAY));		// Instruct the graphics processor to show the list
//	increase_cmd_offset( 4);								// Update the command pointer

//	FT800_Mem_Write32(RAM_CMD + get_cmd_offset(), (CMD_SWAP));			// Make this list active
//	increase_cmd_offset( 4);								// Update the command pointer

//	FT800_Mem_Write16(REG_CMD_WRITE, get_cmd_offset());					// Update the ring buffer pointer so the graphics processor starts executing			
//	
//	//return event;
//}
