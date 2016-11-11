#ifndef __ADC__APP_H__
#define __ADC__APP_H__

#include "common.h"
#include "adc.h"
#include "uart.h"

void Adc_FifoIsr(void);
void Read_AdcBuff(void);
#endif
