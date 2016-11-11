#ifndef __INIT_H__
#define __INIT_H__

#include "common.h"
#include "uart.h"
#include "gpio.h"
#include "rtc.h"
#include "adc.h"
#include "sysinit.h"
#include "flash.h"
#include "uartApp.h"
#include "AdcApp.h"
#include "RtcApp.h"
#include "pit.h"
#include "NeoWay_Gprs.h"
#define UART1_BUFFLENGTH   10

void Board_Init(void);
void Gpio_Init(void);
void Uart_Init(void);
void Rtc_Init(void);
void Pit_Init(void);
#endif
