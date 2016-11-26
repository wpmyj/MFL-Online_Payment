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

extern uint8 g_uRTC10MsEvent;
extern uint8 g_uRTC50MsEvent;
extern uint8 g_uRTC1SEvent;

void Rtc_Event(void);

volatile uint8 TempFlag=0;
volatile uint8 Key=0;

int main (void)
{
	Board_Init();
	ConnetBoard_Init();
	NeoWayBoard_Init();
	Init_Gprs_Device();
	Delay_ms(200);  //�ȴ��ɼ��ĵ�Դ��ѹֵ�ȶ�
	ReBootHardware_Module();	
  while(1)
	{	
		/********************ģ��������************************/		
		ModuleBack_Code();
		/********************ʱ��Ƭ����************************/	
		Rtc_Event();
		if(1==TempFlag)
		{
			TempFlag=0;
			SendMaster_KeyValue(Key);
			SendMaster_Date();
		}
		Gprs_ReceiveEvent();
    if(ON==NeoWaySysPar.Init.GprsSendState)
		{
			TcpSend_Date();
			NeoWaySysPar.Init.GprsSendState=0;
		}
		Gprs_StartCodeEvent();
		/********************ι��ʱ��************************/
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
		}
}
