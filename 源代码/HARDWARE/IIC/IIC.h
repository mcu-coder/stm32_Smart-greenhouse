#ifndef _IIC_H
#define _IIC_H
#include "stm32f10x.h"
//CCS811
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

//HDC1080
/////////计算公式///////////////////////////////////////////
//temper=[(d15:d0)/2^16]*165c-40c
//relative=[(d15:d0)/2^16]*100%
///////////HDC1080寄存器说明////////////////////////////////
#define HDC1080_Device_Adderss      0x80
#define HDC1080_Read_Temperature    0x00
#define HDC1080_Read_Humidity       0x01
#define HDC1080_Read_Config         0x02

#define HDC1080_DEVICE_ID           0xff
#define HDC1080_ID                  0x1000
//////////////////////////////////////////////////////////////
#define HDC1080_Rst                 15//0x8000软复位
#define HDC1080_Enht                13//0x2000//使能加热
#define HDC1080_Mode                12//0x1000//工作模式-为0时分开来读，为1时可以连续读温度在前
#define HDC1080_Trest               10//0x0000  0为温度14bit 1为11bit
#define HDC1080_Hres                8//0x0000 14 11 7bit 温度

//MS5611
#define MS5611_Device_Adderss      0xEE    //CSB is low  
#define MS5611_Reset_Cmd           0x1E
#define MS5611_PROM_RD_Cmd            0xA0 // Prom read command
#define MS5611_CMD_ADC_READ            0x00 // ADC read command
#define MS5611_CMD_ADC_CONV            0x40 // ADC conversion command
#define MS5611_CMD_ADC_D1              0x00 // ADC D1 conversion
#define MS5611_CMD_ADC_D2              0x10 // ADC D2 conversion
#define MS5611_CMD_ADC_256             0x00 // ADC OSR=256
#define MS5611_CMD_ADC_512             0x02 // ADC OSR=512
#define MS5611_CMD_ADC_1024            0x04 // ADC OSR=1024
#define MS5611_CMD_ADC_2048            0x06 // ADC OSR=2048
#define MS5611_CMD_ADC_4096            0x08 // ADC OSR=4096



//IO操作函数	 
#define IIC_SCL    PBout(6) 		//SCL
#define IIC_SDA    PBout(7) 		//SDA	 
#define SDA_read   PBin(7) 		//输入SDA 

#define SCL_H         IIC_SCL=1;
#define SCL_L         IIC_SCL=0;
   
#define SDA_H         IIC_SDA=1
#define SDA_L        IIC_SDA=0;

#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)8<<28;}	//PB7输入模式
#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)3<<28;} //PB7输出模式


//#define SDA_read      GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6)
//#define SDA_read      PBin(7)
void ON_CS(void);
void CS_EN(void);
void OFF_CS(void);

void I2C_GPIO_Config(void);
void I2C_Stop(void);
u8 Single_WriteI2C_byte(u8 Slave_Address,u8 REG_Address,u8 data);
u8 Single_MWriteI2C_byte(u8 Slave_Address,u8 REG_Address,u8 const *data,u8 length);
u8 Single_ReadI2C(u8 Slave_Address,u8 REG_Address,u8 *REG_data,u8 length);

//HDC1080
void HDC1080_Init(void);
u16 HDC1080_Read_Temper(void);
u16 HDC1080_Read_Humidi(void);
u16 HDC1080_Read_DeviceID(void);
#endif

