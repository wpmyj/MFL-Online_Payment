#include "RtcApp.h"

uint8 g_uRTC10MsEvent=0;
uint8 g_uRTC50MsEvent=0;
uint8 g_uRTC1SEvent=0;
extern void NeoWay_Rtc1ms(void);
void Rtc_Isr(void)
{
	static uint16 Cnt_10Ms=0;
	static uint16 Cnt_50Ms=0;
	static uint16 Cnt_1S=0;
	Cnt_10Ms++;
	Cnt_50Ms++;
	Cnt_1S++;
	RTC_ClrFlags();   //定时中断，记得清!!	
	NeoWay_Rtc1ms();
	if(Cnt_10Ms>=10)
	{
		Cnt_10Ms=0;
		g_uRTC10MsEvent=1;
		Read_AdcBuff();
	}
	if(Cnt_50Ms>=50)
	{
		Cnt_50Ms=0;
		g_uRTC50MsEvent=1;
	}
	if(Cnt_1S>=1000)
	{
		Cnt_1S=0;
		g_uRTC1SEvent=1;
	}
	
		
}
