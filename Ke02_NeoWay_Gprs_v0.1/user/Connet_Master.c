#include "Connet_Master.h"
#include "GPRS_Protocol.h"



#define UART2_BUFFLENGTH   10
uint8 Uart2_ReceiveBuff[UART2_BUFFLENGTH]={0};


uint8 g_aRecMasterDate[21] = {0};
uint8 g_aSendMasterDate[5] = {0};
Master_Inf_ Master_Inf;
Ext_Inf_ Ext_Inf;

void ConnetBoard_Init(void)
{
	/********************UART************************/
	UART_ConfigType sUARTConfig;
	sUARTConfig.u32SysClkHz = BUS_CLK_HZ;
	sUARTConfig.u32Baudrate = 19200;
	UART_Init(UART2,&sUARTConfig);
	UART_SetCallback(UART_HandleInt);
	UART_ReceiveInt(UART2, Uart2_ReceiveBuff, UART2_BUFFLENGTH);
	NVIC_EnableIRQ(UART2_IRQn); 
	NVIC_SetPriority(UART2_IRQn,3);		
}

void RecMaster_Uart(uint8 date)
{
	static uint8 RecList = 0;
	uint8 Deal_Date = 0;
	static uint8 i = 0; 
	uint8 Checkout=0,n=0;
	static uint8 TempDate[2] = {0};
	switch(RecList)
	{
		case 0: if((0x03 == date)||(0x06 == date))
						{
							TempDate[0] = date;
							RecList = 1;
						}else if('$' == date)
						{
							TempDate[0] = date;
							RecList = 3;
						}
						break;
		case 1: if(0x01 == date)
						{
							TempDate[1] = date;
							RecList = 2;							
						}else
						{
							TempDate[0] = 0;
							TempDate[1] = 0;
							RecList = 0;							
						}
						break;
		case 2: if(i<7)
						{
							i++;
							g_aRecMasterDate[i+1] = date;
							
						}else
						{
							g_aRecMasterDate[0] = TempDate[0];
							g_aRecMasterDate[1] = TempDate[1];
							for(n=0;n<8;n++)
							{
								Checkout+=g_aRecMasterDate[n];
							}
							if(Checkout == g_aRecMasterDate[8])
							{
								Deal_Date = 1;	
							}
							TempDate[0] = 0;
							TempDate[1] = 0;
							RecList = 0;							
							i=0;
						}break;
		case 3: if('R' == date)
						{
							TempDate[1] = date;
							RecList = 4;
						}else
						{
							TempDate[0] = 0;
							TempDate[1] = 0;
							RecList = 0;
						}break;
		case 4: if(i<19)
						{
							g_aRecMasterDate[i+2] = date;
							i++;
						}else
						{
							g_aRecMasterDate[0] = TempDate[0];
							g_aRecMasterDate[1] = TempDate[1];
							if(('@'==g_aRecMasterDate[19])&&('@'==g_aRecMasterDate[20]))
							{
								Deal_Date = 2;
							}
							TempDate[0] = 0;
							TempDate[1] = 0;
							RecList = 0;							
							i=0;
						}break;
		default: break;
	}
	if(1 == Deal_Date)
	{
		if(0x03 == g_aRecMasterDate[0])
		{
			Master_Inf.State.SysPower = g_aRecMasterDate[2]&0x01;
			Master_Inf.State.Pause = g_aRecMasterDate[2]&0x02;
			Master_Inf.State.Error = g_aRecMasterDate[2]&0x10;
			Master_Inf.State.NeckMassage = g_aRecMasterDate[4]&0x01;
			Master_Inf.State.WaistMassage = g_aRecMasterDate[4]&0x02;
			Master_Inf.State.WholeMassage = g_aRecMasterDate[4]&0x04;
			Master_Inf.RunningTime=(uint16)(g_aRecMasterDate[5]*60);
			Master_Inf.RunningTime+=g_aRecMasterDate[6];
			Master_Inf.Alarm_Num = g_aRecMasterDate[7];	
/******************************************************/
			SendMaster_Date();
		}else if(0x06 == g_aRecMasterDate[0])
		{
			Ext_Inf.NoRespond_State=0;
			Ext_Inf.KeyValue = 0;
			SendMaster_Date();
		}
	}else if(2 == Deal_Date)
	{
		for(n=0;n<17;n++)
		{
			Device_Info.Serial_Id[n] = g_aRecMasterDate[n+2];
		}
		Ext_Inf.DownloadID_State = 1;
	}	
}

void SendMaster_Date(void)
{
	uint8 i = 0,Checkout = 0;
	
	g_aSendMasterDate[0] = 0x03;
	g_aSendMasterDate[1] = 0x01;
	g_aSendMasterDate[2] = 0x00;
	g_aSendMasterDate[3] = 0x00;
	
	if(1 == Ext_Inf.NoRespond_State)
	{
		g_aSendMasterDate[0] = 0x06;
		g_aSendMasterDate[3] = Ext_Inf.KeyValue;
	}
	for(i=0;i<4;i++)
	{
		Checkout+=g_aSendMasterDate[i];
	}
	g_aSendMasterDate[4] = Checkout;	
	UART_SendWait(UART_CONNET_MASTER,g_aSendMasterDate,5);	
}

void SendMaster_KeyValue(uint8 KeyValue)
{
	Ext_Inf.NoRespond_State = 1;
	Ext_Inf.KeyValue = KeyValue;
}

