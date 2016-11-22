#include "AdcApp.h"

static unsigned int ADC_ConversionCount=0;
unsigned int ADC_ConversionBuff[16]={0};
volatile unsigned int g_uNeoWayVccio = 0;
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
		unsigned long long Temp=0;
		
		TemperatureCount++;
	
		if(TemperatureCount<=16)
		{
			ADC_SetChannel(ADC,ADC_CHANNEL_AD0);
		}else
		{
			TemperatureCount=0;
			g_uNeoWayVccio=ADC_ConversionBuff[15];
			for(i=0;i<16;i++)
			{
				Temp+=ADC_ConversionBuff[i];				
			}
			g_uNeoWayVccio=(uint16)(Temp/16);
			ADC_ConversionCount=0;
		}
}
