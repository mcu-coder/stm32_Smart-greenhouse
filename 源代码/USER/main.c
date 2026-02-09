#include "sys.h"	
#include "delay.h"	
#include "SWITCH.h" 
#include "esp8266.h"
#include "onenet.h"
#include "usart2.h"
#include "usart.h"
#include "bh1750.h"
#include "IIC.h"
#include "Lcd_Driver.h"
#include "LCD_Config.h"
#include "GUI.h"
#include "adc.h"
#include "dht11.h"
#include "KEY.h"


#define CCS811_Add  0x5A<<1
#define STATUS_REG 0x00
#define MEAS_MODE_REG 0x01
#define ALG_RESULT_DATA 0x02
#define ENV_DATA 0x05
#define NTC_REG 0x06
#define THRESHOLDS 0x10
#define BASELINE 0x11
#define HW_ID_REG 0x20
#define ERROR_ID_REG 0xE0
#define APP_START_REG 0xF4
#define SW_RESET 0xFF
#define CCS_811_ADDRESS 0x5A
#define GPIO_WAKE 0x5
#define DRIVE_MODE_IDLE 0x0
#define DRIVE_MODE_1SEC 0x10
#define DRIVE_MODE_10SEC 0x20
#define DRIVE_MODE_60SEC 0x30
#define INTERRUPT_DRIVEN 0x8
#define THRESHOLDS_ENABLED 0x4

u8 Water_pump_KEY=0,FAN_KEY=0,LED_KEY=0;
u8 Water_pump_onenet=0,FAN_onenet=0,LED_onenet=0;
u8 Onenet_control=0,Onenet_succes;
u8 BUUF[12];
u8 Information[10];
u8 temp=0x5a;
u8 key=0;
u8 MeasureMode,Status,Error_ID;
void threshold_value(void);
typedef struct {
u16 eco2;
u16 tvoc;
u8 status;
u8 error_id;
u16 raw_data;
} ccs811_measurement_t;
ccs811_measurement_t CCS;


extern int lux;
u8 flags=0;
u8 counts=0;
extern unsigned char esp8266_buf[128];

u8 threshold_sta=1,threshold_sta_control=1,threshold_flag=1,Gui_mode=0,control_mode=0,Gui_mode_last=25,control_mode_last=25;
void Shuju(void)
{
		 
		 OFF_CS(); 
	   CCS.eco2= (u16)BUUF[0]*256+BUUF[1];
	   CCS.tvoc= (u16)BUUF[2]*256+BUUF[3];
	   Information[0]=0;
	
	  SUR_status.soil_hum=100-(float)Get_Adc_Average(ADC_Channel_0,1)/40.96;  //土壤湿度
		Conversion();/// bh1750 光照模块数据采集
    DHT11_Read_Data(&SUR_status.air_tem,&SUR_status.air_hum);  //获取温湿度
		SUR_status.car=(float)CCS.eco2/10;  //二氧化碳
		 SUR_status.light=lux;  //灯光

}
void init_data(void)
{
	SUR_status.air_tem_num=25;
	SUR_status.soil_hum_num=40;
	SUR_status.car_num=250;
	SUR_status.light_num=500;
}
void control_set(void)
{
	switch(control_mode)
		{
		case 0://自动
		if(SUR_status.air_tem>SUR_status.air_tem_num) FAN=1;  //温度过高 继电器开启 风扇转
		else FAN=0;
		if(SUR_status.soil_hum<SUR_status.soil_hum_num)  Water_pump=1;  //土壤湿度不够，水泵开
		else Water_pump=0;
		if(SUR_status.light<SUR_status.light_num)  LED=1;  ///光照不够开灯
		else LED=0;
			break;
		 
		default:break;
	}	
		if(SUR_status.car>SUR_status.car_num)
			BEEP=1;
		else
			BEEP=0;
}
 void Num_Test(void)  //手动控制显示
{
	if(Onenet_succes)
	Gui_DrawFont_GBK16(60,144,BLACK,WHITE,"已连接");
	else
	Gui_DrawFont_GBK16(60,144,BLACK,WHITE,"未连接");
	if( Onenet_succes)
	Gui_DrawFont_GBK16(60,80,BLACK,WHITE,"开");
	Gui_DrawFont_GBK16(60,80,BLACK,WHITE,"关");
	 
}

void chuishi_1(void) //初始界面1显示
{

	Gui_DrawFont_GBK16(0,0,BLACK,WHITE,"空气温度:    ℃");
  Gui_DrawFont_GBK16(0,16,BLACK,WHITE,"空气湿度:    %");
	Gui_DrawFont_GBK16(0,32,BLACK,WHITE,"土壤湿度:    %");
	Gui_DrawFont_GBK16(0,48,BLACK,WHITE,"光照强度:    lux");
	Gui_DrawFont_GBK16(0,64,BLACK,WHITE,"二氧化碳:    ‰");
		Gui_DrawFont_GBK16(10,144,BLACK,WHITE,"WIFI:");
	if(Onenet_succes)
	Gui_DrawFont_GBK16(60,144,BLACK,WHITE,"已连接:");
	else
	Gui_DrawFont_GBK16(60,144,BLACK,WHITE,"未连接:");
}
void chuishi_2(void)
{
	Gui_DrawFont_GBK16(10,30,BLACK,WHITE,"温度阈值:");
	Gui_DrawFont_GBK16(10,46,BLACK,WHITE,"温度阈值:");
	Gui_DrawFont_GBK16(10,62,BLACK,WHITE,"土壤湿度阈值:");
	Gui_DrawFont_GBK16(10,88,BLACK,WHITE,"光照阈值:");
	Gui_DrawFont_GBK16(10,102,BLACK,WHITE,"二氧化碳阈值:");
}
void  control_gui(void)  //控制端口显示
{

		switch(control_mode)
		{
			case 0:
  Gui_DrawFont_GBK16(0,128,BLUE,WHITE,"自动");
	Gui_DrawFont_GBK16(45,128,BLACK,WHITE,"手动");
	Gui_DrawFont_GBK16(90,128,BLACK,WHITE,"云端");
			break;
	 
	}

	}
void GUI_Test1(void)  ///主界面控制和显示，环境数据显示
{

	if(Gui_mode_last!=Gui_mode)  //判断是否改变，更新固定显示
	{
		chuishi_1();
		Gui_mode_last=Gui_mode;  
	}

	LCD_ShowNum(70,0,SUR_status.air_tem,2,BLACK,WHITE);
	LCD_ShowNum(70,16,SUR_status.air_hum,2,BLACK,WHITE);
	LCD_ShowNum(70,32,SUR_status.soil_hum,2,BLACK,WHITE);
	LCD_ShowNum(70,48,SUR_status.light,4,BLACK,WHITE);
	LCD_ShowNum(70,64,SUR_status.car,2,BLACK,WHITE);  //数据显示

}
void GUI_Test2(void)  //参数设置显示
{
		if(Gui_mode_last!=Gui_mode)  //初始显示
	{
	
	Gui_DrawFont_GBK16(40,0,BLACK,WHITE,"参数设置");
	Gui_DrawFont_GBK16(10,16,BLACK,WHITE,"温度阈值:");
	Gui_DrawFont_GBK16(10,32,BLACK,WHITE,"土湿阈值:");
	Gui_DrawFont_GBK16(10,48,BLACK,WHITE,"光照阈值:");
	Gui_DrawFont_GBK16(10,64,BLACK,WHITE,"CO2阈值:");
				Gui_mode_last=Gui_mode;
	}  
	      Gui_DrawFont_GBK16(0,threshold_sta*16,BLACK,WHITE,"*");
	LCD_ShowNum(80,16,SUR_status.air_tem_num,2,BLACK,WHITE);
	LCD_ShowNum(80,32,SUR_status.soil_hum_num,2,BLACK,WHITE);
	LCD_ShowNum(80,48,SUR_status.light_num,4,BLACK,WHITE);
	LCD_ShowNum(80,64,SUR_status.car_num,3,BLACK,WHITE);   //参数显示

}

void GUI_Test3(void)  //手动控制界面
{		
	if(Gui_mode_last!=Gui_mode) // 界面显示
	{
	
	Gui_DrawFont_GBK16(40,0,BLACK,WHITE,"手动控制");
		Gui_DrawFont_GBK16(20,16,BLACK,WHITE,"风扇:");
		Gui_DrawFont_GBK16(20,32,BLACK,WHITE,"水泵:");
		Gui_DrawFont_GBK16(20,48,BLACK,WHITE,"灯光:");
				Gui_mode_last=Gui_mode;
	}
  Gui_DrawFont_GBK16(0,threshold_sta_control*16,BLACK,WHITE,"*");  //显示选择位置
	if(FAN_KEY)  //手动控制显示文字
  Gui_DrawFont_GBK16(80,16,BLACK,WHITE,"开");
	else
	Gui_DrawFont_GBK16(80,16,BLACK,WHITE,"关");
	
	if(Water_pump_KEY)
  Gui_DrawFont_GBK16(80,32,BLACK,WHITE,"开");
	else
	Gui_DrawFont_GBK16(80,32,BLACK,WHITE,"关");
	
  if(LED_KEY)
  Gui_DrawFont_GBK16(80,48,BLACK,WHITE,"开");
	else
	Gui_DrawFont_GBK16(80,48,BLACK,WHITE,"关");
}

void threshold_value(void)  //按键设置，按键进入界面和参数设置
{				
	if(key)
	{
		if(Gui_mode==1)  //参数设置界面，数据设置 第二个界面
		{
			
		if(SUR_status.air_tem_num>60)//  温度
			SUR_status.air_tem_num=20;
		
		if(SUR_status.air_hum_num>80)//湿度
			SUR_status.air_hum_num=30;
		
		if(SUR_status.soil_hum_num>80) //土壤湿度
			SUR_status.soil_hum_num=30;
		
		if(SUR_status.light_num>5000)   //光照
			SUR_status.light_num=1000;
		
		if(SUR_status.car_num>500)  //co2
			SUR_status.car_num=100;

			if(key==3)  //参数加
			{
		switch(threshold_sta){
			case 1:SUR_status.air_tem_num++;break;
			case 2:SUR_status.soil_hum_num++;break;
			case 3:SUR_status.light_num+=100;break;
			case 4:SUR_status.car_num+=5;break;
			default:break;
		}
	  }		
	 
		if(key==2)  //参数选择设置
		{
	 Gui_DrawFont_GBK16(0,threshold_sta*16,BLACK,WHITE," ");
	 threshold_sta++;
		if(threshold_sta==5)
			threshold_sta=1;	
	}
	
	}
		
		if(Gui_mode==2)  //手动控制界面
		{
			if(key==3)
			{
		switch(threshold_sta_control){
			case 1:FAN_KEY=~FAN_KEY;break;
			case 2:Water_pump_KEY=~Water_pump_KEY;break;
			case 3:LED_KEY=~LED_KEY;break;
			default:break;
		}
	  }		
	 
		if(key==2)  //参数设置位置选择
		{
			 Gui_DrawFont_GBK16(0,threshold_sta_control*16,BLACK,WHITE," ");
			threshold_sta_control++;
		if(threshold_sta_control==4)
			threshold_sta_control=1;	
	}	
	}
		
		if(key==1){   ///界面更换
		Lcd_Clear(WHITE);
				Gui_DrawFont_GBK16(10,144,BLACK,WHITE,"WIFI:");
	if(Onenet_succes)
	Gui_DrawFont_GBK16(60,144,BLACK,WHITE,"已连接:");
	else
	Gui_DrawFont_GBK16(60,144,BLACK,WHITE,"未连接:");
		Gui_mode++;
		if(Gui_mode==3) Gui_mode=0;
		}
		
	if(key==4){   //控制方式选择
		control_mode++;
			if(control_mode==3)
				control_mode=0;
		}

			key=0;
	}
		switch(Gui_mode)  //界面显示选择
		{
			case 0:GUI_Test1();break;
			case 1:GUI_Test2();break;
			case 2:GUI_Test3();break;
			default:break;
		}
		control_gui();
}

 int main(void)
 {
	unsigned char *dataPtr = 0;
	delay_init();	    	 //延时函数初始化	    
	uart_init(115200);  //串口1

  I2C_GPIO_Config();   //iic初始化  ccs811,模块初始化  检测CO2 
    CS_EN();  //对ccs811 配置
	  delay_ms(100);
	  ON_CS();
	  delay_ms(100);
	 
		
  KEY_Init();//按键初始化
	Lcd_Init();//st7735 LCD屏幕初始化
	LCD_LED_SET;//通过IO控制背光亮		
	Lcd_Clear(WHITE);  //清屏
	Adc_Init();  //ADc初始化，采集土壤湿度模块的模拟量
	init_data();  //初始阈值参数设置
   	while(DHT11_Init())	//DHT11初始化	温湿度
	{
 		delay_ms(200);
	}
 
	Gui_DrawFont_GBK16(10,60,BLACK,WHITE,"正在连接ONENET");
	ESP8266_Init();					//初始化ESP8266 wifi
	 
 
	Lcd_Clear(WHITE);//清屏
	Shuju();//获取环境数据
		Gui_DrawFont_GBK16(10,144,BLACK,WHITE,"WIFI:");
		if(Onenet_succes)
	Gui_DrawFont_GBK16(60,144,BLACK,WHITE,"已连接:");
	else
	Gui_DrawFont_GBK16(60,144,BLACK,WHITE,"未连接:");
	while(1)
	{		
   	key=KEY_Scan(0);//检测按键
			threshold_value();  //界面和按键处理
		counts++;//累加，用于onenet数据上传计时
		control_set();//控制处理
	 
	}
 }

/*			case 6:LED_FLAG=1;break;
			case 7:LED_FLAG=1;break;
			case 8:LED_FLAG=1;break;
			case 9:LED_FLAG=1;break;
			case 10:LED_FLAG=1;break;
 */