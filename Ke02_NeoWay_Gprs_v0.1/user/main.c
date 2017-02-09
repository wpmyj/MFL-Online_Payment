/*!
*
* @file main.c
*
* @author Deric
*
* @version 1
*
* @date 2016-9-27
*
* @brief  
* @port :uart0 gprs  23-24
				 uart1 380W 36-37
				 uart2 Debug  16-17
				 PowerControl 1   PTD1
				 RstCpu      19		PTC1
				 On/Off Mcu  18		PTD5

*******************************************************************************/
#include "common.h"
#include "ics.h"
#include "rtc.h"
#include "uart.h"
#include "gpio.h"
#include "adc.h"
#include "sysinit.h"
#include "flash.h"
#include "ftm.h"
#include "Init.h"
#include "NeoWay_Gprs.h"
#include "GPRS_Protocol.h"
#include "Connet_Master.h"

void Rtc_Event(void);

volatile uint8 TempFlag=0;
volatile uint8 Key=0;

int main (void)
{

	Board_Init();
	ConnetBoard_Init();  
  //Init_Gprs_Device();
	NeoWayBoard_Init();
  Delay_ms(400);  //必须要有的延时，使底板初始化完成
/********************读取ID************************/	
    SendMaster_KeyValue(0);
		SendMaster_Date();
    Delay_ms(50);
    Read_MasterID();
    SendMaster_Date();
    Delay_ms(50);
/********************读取ID结束************************/
  NeoWayExternalPar.HardwareRebootState = ON;
  while(1)
	{	
		/********************模块回码解析************************/		
		ModuleBack_Code();
    /********************模块重启****************************/
    ReBoot_Module();
		/********************时间片解析**************************/	
		Rtc_Event();
    SendMaster_Date();
		if(1==TempFlag)
		{
			TempFlag=0;
			SendMaster_KeyValue(Key);
		}
		Gprs_ReceiveEvent();
    if(ON==NeoWaySysPar.Init.GprsSendState)
		{
        NeoWaySysPar.Init.GprsSendState=OFF;
        TcpSend_Date();			
		}
		Gprs_StartCodeEvent();
    ChairStateChange();
		/********************喂狗时间************************/
    WDOG_Feed();
	}   
}

void Rtc_Event(void)
{
		if(g_uRTC10MsEvent>=1)
		{
			g_uRTC10MsEvent=0;			
		}
		if(g_uRTC50MsEvent>=1)
		{
			g_uRTC50MsEvent=0;		
		}
		if(g_uRTC1SEvent>=1)
		{
			g_uRTC1SEvent=0;					
			Gprs_1SEvent();
			NeoWay_Rtc1s();
      LostMasterCnt();
		}
}


