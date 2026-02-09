
//单片机头文件
#include "stm32f10x.h"

//网络设备
#include "esp8266.h"

//协议文件
#include "onenet.h"
#include "edpkit.h"

//硬件驱动
#include "usart2.h"
#include "delay.h"
#include "SWITCH.h"

//C库
#include <string.h>
#include <stdio.h>

extern u8 Water_pump_onenet,FAN_onenet,LED_onenet,control_mode;
#define DEVID	"689019494"                           //设备id   TGpUxQSNgO

#define APIKEY	"F4IswZTKx2qmzhVGslRIg1IV0Kw="        //密码    GzljBwXFOzPDQ8Ujc4ELycsdWTtfN5MpWzXXp831dS8=

//#define APIKEY	    "0ZNDTzHKX7P58pK=tyD6ugpIjWs="

 FENGSHAN_STATUS SUR_status;                     //结构体变量定义
 
//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};				//协议包

	unsigned char *dataPtr;
	
	unsigned char status = 1;
	
	UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
                        "DEVID: %s,     APIKEY: %s\r\n"
                        , DEVID, APIKEY);

	if(EDP_PacketConnect1(DEVID, APIKEY, 256, &edpPacket) == 0)		//根据devid 和 apikey封装协议包
	{
		 
		dataPtr = ESP8266_GetIPD(250);								//等待平台响应
		if(dataPtr != NULL)
		{
			if(EDP_UnPacketRecv(dataPtr) == CONNRESP)
			{
				switch(EDP_UnPacketConnectRsp(dataPtr))
				{
					case 0:UsartPrintf(USART_DEBUG, "Tips:	连接成功\r\n");status = 0;break;
					
					case 1:UsartPrintf(USART_DEBUG, "WARN:	连接失败：协议错误\r\n");break;
					case 2:UsartPrintf(USART_DEBUG, "WARN:	连接失败：设备ID鉴权失败\r\n");break;
					case 3:UsartPrintf(USART_DEBUG, "WARN:	连接失败：服务器失败\r\n");break;
					case 4:UsartPrintf(USART_DEBUG, "WARN:	连接失败：用户ID鉴权失败\r\n");break;
					case 5:UsartPrintf(USART_DEBUG, "WARN:	连接失败：未授权\r\n");break;
					case 6:UsartPrintf(USART_DEBUG, "WARN:	连接失败：授权码无效\r\n");break;
					case 7:UsartPrintf(USART_DEBUG, "WARN:	连接失败：激活码未分配\r\n");break;
					case 8:UsartPrintf(USART_DEBUG, "WARN:	连接失败：该设备已被激活\r\n");break;
					case 9:UsartPrintf(USART_DEBUG, "WARN:	连接失败：重复发送连接请求包\r\n");break;
					
					default:UsartPrintf(USART_DEBUG, "ERR:	连接失败：未知错误\r\n");break;
				}
			}
		}
		
		EDP_DeleteBuffer(&edpPacket);								//删包
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	EDP_PacketConnect Failed\r\n");
	
	return status;
	
}
u8 onenet_num=1;
unsigned char OneNet_FillBuf(char *buf)
{
	
	char text[16];
	memset(text, 0, sizeof(text));
	strcpy(buf, ",;");
	memset(text, 0, sizeof(text));
	switch(onenet_num)
	{
		case 1:sprintf(text, "AIR_TEM,%d;", SUR_status.air_tem);onenet_num++;break;   //发送温度
		case 2:sprintf(text, "AIR_HUM,%d;", SUR_status.air_hum);onenet_num++;break;//发送湿度
		case 3:sprintf(text, "LIGHT,%d;",SUR_status.light);onenet_num++;break;//发送光照
		case 4:sprintf(text, "CAR,%d;", SUR_status.car);onenet_num++;break;//发送CO
		case 5:sprintf(text, "SOIL_HUM,%d;", SUR_status.soil_hum);onenet_num++;break;//发送CO
		case 6:sprintf(text, "Water_FLAG,%d;", Water_pump_onenet);onenet_num++;break;//发送CO
		case 7:sprintf(text, "FAN_FLAG,%d;", FAN_onenet);onenet_num++;break;//发送CO
		case 8:sprintf(text, "LED_FLAG,%d;", LED_onenet);onenet_num++;break;//发送CO
		case 9:sprintf(text, "CONTROL,%d;", control_mode);onenet_num=1;break;//发送CO
		//default:sprintf(text, "CAR,%.2f;", SUR_status.car);break;
}
	strcat(buf, text);
printf("\r\n00:%s\r\n",buf);
	return strlen(buf);
}

//==========================================================
//	函数名称：	OneNet_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_SendData(void)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};												//协议包
	char buf[128];
	short body_len = 0, i = 0;

	memset(buf, 0, sizeof(buf));
	body_len = OneNet_FillBuf(buf);																	//获取当前需要发送的数据流的总长度
	if(body_len)
	{
		if(EDP_PacketSaveData(NULL, body_len, NULL, kTypeString, &edpPacket) == 0)					//封包
		{
			for(; i < body_len; i++)
				edpPacket._data[edpPacket._len++] = buf[i];
			
			ESP8266_SendData(edpPacket._data, edpPacket._len);										//上传数据到平台
			
			EDP_DeleteBuffer(&edpPacket);															//删包
		}
		else
			UsartPrintf(USART_DEBUG, "WARN:	EDP_NewBuffer Failed\r\n");
		//	ESP8266_Clear();
  }
	}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
//==========================================================
extern u8 Water_pump_KEY,FAN_KEY,LED_KEY;
void OneNet_RevPro(unsigned char *cmd)
{
	
	EDP_PACKET_STRUCTURE edpPacket = {NULL, 0, 0, 0};	//协议包
	
	char *cmdid_devid = NULL;
	unsigned short cmdid_len = 0;
	char *req = NULL;
	unsigned int req_len = 0;
	unsigned char type = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	
	type = EDP_UnPacketRecv(cmd);
	switch(type)										//判断是pushdata还是命令下发
	{
		case CMDREQ:									//解命令包
			
			result = EDP_UnPacketCmd(cmd, &cmdid_devid, &cmdid_len, &req, &req_len);
			
			if(result == 0)								//解包成功，则进行命令回复的组包
			{
				EDP_PacketCmdResp(cmdid_devid, cmdid_len, req, req_len, &edpPacket);
				UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_devid, req, req_len);
			}
			
		break;
			
		case SAVEACK:
			
			if(cmd[3] == MSG_ID_HIGH && cmd[4] == MSG_ID_LOW)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Send %s\r\n", cmd[5] ? "Err" : "Ok");
			}
			else
				UsartPrintf(USART_DEBUG, "Tips:	Message ID Err\r\n");
			
		break;
			
		default:
			result = -1;
		break;
	}
	
	ESP8266_Clear();										//清空缓存
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req, ':');							//搜索':'
	
	if(dataPtr != NULL)									//如果找到了
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//判断是否是下发的命令控制数据
		{
			numBuf[num++] = *dataPtr++;
		}
		numBuf[num] = 0;
		
		num = atoi((const char *)numBuf);				//转为数值形式
			if(strstr((char *)req, "Onenet"))
		{
			if(num == 1)
			{
				control_mode=2;		//云端控制
			}
			else if(num == 0)
			{			
				control_mode=0;    //	自动控制
			}
		}
	if(control_mode==2)
	{		
	if(strstr((char *)req, "FAN")) //风扇
		{
			if(num == 1)
			{
				FAN_onenet=1;
			}
			else if(num == 0)
			{			
				FAN_onenet=0;
			}
		}
			if(strstr((char *)req, "Water")) //水泵
		{
			if(num == 1)
			{
			Water_pump_onenet=1;
			}
			else if(num == 0)
			{			
       Water_pump_onenet=0;
			}
		}
   if(strstr((char *)req, "LED"))      //风扇开启
		{

			if(num == 1)
			{
				LED_onenet=1;					
			}
			else if(num == 0)
			{	
      LED_onenet=0;						
			}
		}
	}
	}
	
	if(type == CMDREQ && result == 0)						//如果是命令包 且 解包成功
	{
		EDP_FreeBuffer(cmdid_devid);						//释放内存
		EDP_FreeBuffer(req);
															//回复命令
		ESP8266_SendData(edpPacket._data, edpPacket._len);	//上传平台
		EDP_DeleteBuffer(&edpPacket);						//删包
	}

}

