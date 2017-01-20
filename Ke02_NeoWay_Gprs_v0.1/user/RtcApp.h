#ifndef __RTC__APP_H__
#define __RTC__APP_H__
#include "common.h"
#include "rtc.h"
#include "AdcApp.h"
#include "init.h"
#include "NeoWay_Gprs.h"
extern uint8 g_uRTC10MsEvent;
extern uint8 g_uRTC50MsEvent;
extern uint8 g_uRTC1SEvent;


void Rtc_Isr(void);
void Light_Event(void);
#endif
