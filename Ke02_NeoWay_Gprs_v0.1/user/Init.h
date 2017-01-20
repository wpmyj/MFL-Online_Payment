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
#include "wdog.h"
#include "NeoWay_Gprs.h"
#define UART1_BUFFLENGTH   10
#define LIGHT			GPIO_PTD3//GPIO_PTD3//GPIO_PTB5
void Board_Init(void);
void Gpio_Init(void);
void Uart_Init(void);
void Rtc_Init(void);
void Adc_Init(void);
void Pit_Init(void);
void Wdog_Init(void);
#endif
