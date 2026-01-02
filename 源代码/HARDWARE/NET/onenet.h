#ifndef _ONENET_H_
#define _ONENET_H_
#include "sys.h"	

typedef struct
{
	u16 light;
	u8 fan_flag;
	u8 LED_flag;
	u8 water_pump_flag;
	u8 LED_num;
	u8 FAN_KEY;
	u8 LED_KEY;
	u8 Water_pump_num;
	u16 car;
  u8 air_hum;
  u8 air_tem;
  u8 soil_hum;
	u16 air_hum_num;
  u16 air_tem_num;
  u16 soil_hum_num;
	u16 car_num;
	u16 light_num;
} FENGSHAN_STATUS;

extern FENGSHAN_STATUS SUR_status;


_Bool OneNet_DevLink(void);

void OneNet_SendData(void);

void OneNet_RevPro(unsigned char *cmd);

#endif
