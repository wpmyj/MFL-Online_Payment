#include "Connet_Master.h"
#include "GPRS_Protocol.h"



#define UART1_BUFFLENGTH   10
uint8 Uart1_ReceiveBuff[UART1_BUFFLENGTH]={0};


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
	UART_Init(UART1,&sUARTConfig);
	UART_SetCallback(UART_HandleInt);
	UART_ReceiveInt(UART1, Uart1_ReceiveBuff, UART1_BUFFLENGTH);
	NVIC_EnableIRQ(UART1_IRQn); 
	NVIC_SetPriority(UART1_IRQn,3);		
}

void RecMaster_Uart(uint8 date)
{
	static uint8 RecList = 0;
	uint8 Deal_Date = 0;
	static uint8 i = 0; 
	uint8 Checkout=0,n=0;
	static uint8 TempDate[2] = {0};
  volatile uint8 m=0;
  static uint8 Old_State=0;
  if(('$' == date)&&(ON==Ext_Inf.Start_ReadID))
  {
    m=1;
		RecList =3;
		TempDate[0] = date;
		return;
  }
	switch(RecList)
	{
		case 0: if((0x03 == date)||(0x06 == date))
						{
							TempDate[0] = date;
							RecList = 1;
						}else
						{
							TempDate[0] = 0;
							TempDate[1] = 0;
							RecList = 0;
              i=0;
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
							i=0;
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
						}
						if(19 == i)
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
		default:	TempDate[0] = 0;
							TempDate[1] = 0;
							RecList = 0;i=0; break;
	}
	if(1 == Deal_Date)
	{
	  Master_Inf.LostMasterTime = 0;
    Alarm_State.Gprs_Alarm = (Alarm_State.Gprs_Alarm & 0xFE);//Çå¸æ¾¯Î»
    
		if(0x03 == g_aRecMasterDate[0])
		{
       if((g_aRecMasterDate[2]&0x13)!=Old_State)
      {
        Gprs_flag.bits.StateChange |=0x02;
        Collect_Data.Chair_State=(g_aRecMasterDate[2]&0x13);
      }
			Master_Inf.State.SysPower = g_aRecMasterDate[2]&0x01;
			Master_Inf.State.Pause = g_aRecMasterDate[2]&0x02;
			Master_Inf.State.Error = g_aRecMasterDate[2]&0x10;
      Master_Inf.State.Massage = g_aRecMasterDate[4];
      Collect_Data.Program_State = g_aRecMasterDate[4];
			Master_Inf.RunningTime=(uint16)(g_aRecMasterDate[5]*60);
			Master_Inf.RunningTime+=g_aRecMasterDate[6];
			Master_Inf.Alarm_Num = g_aRecMasterDate[7];	
      
      Old_State =(g_aRecMasterDate[2]&0x13);

/******************************************************/
      Master_Inf.SendMasterDateFlag = ON;
			//SendMaster_Date();
		}else if(0x06 == g_aRecMasterDate[0])
		{
    			Ext_Inf.NoRespond_State=0;
          
          Master_Inf.SendMasterDateFlag = ON;
    			//SendMaster_Date();
		}
	}else if(2 == Deal_Date)
	{
		for(n=0;n<17;n++)
		{
			Device_Info.Serial_Id[n] = g_aRecMasterDate[n+2];
		}
		Ext_Inf.DownloadID_State = ON;
    Ext_Inf.Start_ReadID = OFF;
	}	
}

void SendMaster_Date(void)
{
	uint8 i = 0,Checkout = 0;
	if(!Master_Inf.SendMasterDateFlag)return; 
  Master_Inf.SendMasterDateFlag = OFF;
  
	g_aSendMasterDate[0] = 0x03;
	g_aSendMasterDate[1] = 0x01;
	g_aSendMasterDate[2] = 0x00;
	g_aSendMasterDate[3] = 0x00;
	
	if(ON == Ext_Inf.NoRespond_State)
	{
		g_aSendMasterDate[0] = 0x06;		
		g_aSendMasterDate[3] = Ext_Inf.KeyValue;	
		Ext_Inf.NoRespond_State		=OFF;
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
  Master_Inf.SendMasterDateFlag = ON;
	Ext_Inf.KeyValue = KeyValue;
}

void  Read_MasterID(void)
{
    Ext_Inf.Start_ReadID=ON;
    SendMaster_KeyValue(2);
}


void LostMasterCnt(void)
{
    if(ON==NeoWayExternalPar.NetWorkConnetState)
    {
        Master_Inf.LostMasterTime++;
        if(Master_Inf.LostMasterTime==5)
        {
            Master_Inf.LostMasterTime=0;
            SendMaster_KeyValue(0);
            Alarm_State.Gprs_Alarm =(Alarm_State.Gprs_Alarm | 0x01);
        }
    }
}

