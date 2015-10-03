#ifndef _FT_PLATFORM_H_
#define _FT_PLATFORM_H_

//#define ARDUINO_PLATFORM
#define MSVC_PLATFORM

//#define SAMAPP_DISPLAY_QVGA
#include <stdio.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <Windows.h>
#include "ftd2xx.h"
#include "LibMPSSE_spi.h"
#include "FT_DataTypes.h"
#include "FT_Gpu_Hal.h"
#include "FT_Gpu.h"
#include "FT_CoPro_Cmds.h"

#define BUFFER_OPTIMIZATION
#define MSVC_PLATFORM_SPI
#define SAMAPP_ENABLE_APIS_SET0
#define SAMAPP_ENABLE_APIS_SET1
#define SAMAPP_ENABLE_APIS_SET2
#define SAMAPP_ENABLE_APIS_SET3
#define SAMAPP_ENABLE_APIS_SET4
#define SAMAPP_ENABLE_APIS_SET5

/* Compile time switch for enabling sample app api sets - please cross check the same in SampleApp_RawData.cpp file as well */
/*
#define SAMAPP_ENABLE_APIS_SET0
#define SAMAPP_ENABLE_APIS_SET1
#define SAMAPP_ENABLE_APIS_SET2
#define SAMAPP_ENABLE_APIS_SET3
*/
#define SAMAPP_ENABLE_APIS_SET3

#endif
#endif /*_FT_PLATFORM_H_*/
/* Nothing beyond this*/




