#include "AdcApp.h"

static unsigned int ADC_ConversionCount=0;
unsigned int ADC_ConversionBuff[16]={0};
unsigned int ADC_SenserTemperature[16]={0};	
void Adc_FifoIsr(void)
{

		while( !ADC_IsFIFOEmptyFlag(ADC) )
		{
				if( ADC_ConversionCount < 16 )
				{
						ADC_ConversionBuff[ADC_ConversionCount++] = ADC_ReadResultReg(ADC);
				}
				else
				{
						ADC_ReadResultReg(ADC);
				}
		}
}

void Read_AdcBuff(void)
{
		static unsigned char i,TemperatureCount=0;
		
		TemperatureCount++;
	
		if(TemperatureCount<=16)
		{
			ADC_SetChannel(ADC,ADC_CHANNEL_AD22_TEMPSENSOR);
		}else
		{
			TemperatureCount=0;
			for(i=0;i<16;i++)
			{
				ADC_SenserTemperature[i]=ADC_ConversionBuff[i];
				UART_PutChar(UART0,0xfd);
				UART_PutChar(UART0,(unsigned char)(ADC_SenserTemperature[i]>>8));
				UART_PutChar(UART0,(unsigned char)(ADC_SenserTemperature[i]));
				UART_PutChar(UART0,0xfc);
				ADC_ConversionCount=0;
			}
		}
}
