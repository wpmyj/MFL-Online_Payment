/*
** ===================================================================
**     File Name   :	GPRS_Protocol.c
**     Author			 :	Deric-Easepal
**     Versions  	 : 	v0.2  2016-10-29
**		 Port				 :	
**     Modification: 
**			2016-10-29 £º³õ¸åÖ®Ç°Ð´ºÃ£¬ÕûºÏÓëÓÐ·½GPRSÁ¬½ÓÄÚÈÝ
**						note £º¸æ¾¯Ä£¿éÐèÖØÐ´
										 Æô¶¯³ÌÐòº¯ÊýÒ²ÒªÖØÐ´
										 È¥µô´®¿ÚÖÐ¶Ï½ÓÊÕº¯Êý
										 ½ÓÊÕµÄÊý¾ÝÊ¹ÓÃÖ¸ÕëÖ¸Ïò£¬·¢ËÍµÄÊý¾Ý£¬Ê¹ÓÃÊý¾Ý¸´ÖÆ
							note : ÐÄÌø°ü±¨¾¯£¬ÐèÐÞ¸Ä
							note : ipÐ´Èëeeprom ±êºÅ »¹Î´ÊµÏÖ
** ===================================================================
*/

#include "GPRS_Protocol.h"

/********************µ÷ÓÃÍâ²¿²ÎÊý¼¯ºÏ************************/
/*
			NeoWayExternalPar            ×´Ì¬ÐÅÏ¢
											Ö»Òª½«	NeoWayExternalPar.HardwareRebootState
															NeoWayExternalPar.SofewareRebootState ÖÃÎ»£¬¾Í»áÊ¹ÓÃ¶ÔÓ¦µÄ·½Ê½ÖØÆôÄ£¿é
															
			g_aTcpRecDate          ½ÓÊÕµ½Êý¾ÝµÄÊý×é
			g_uRecTcpDateNum			 ½ÓÊÕµ½µÄÊý¾Ý³¤¶È 
			
			g_aTcpSendDate				 ·¢ËÍÊý¾ÝµÄÊý×é
			g_uSendTcpDateNum			 ·¢ËÍÊý¾ÝµÄÊý¾Ý³¤¶È
			
			extern uint8 Gprs_Send_Date(void);
*/

/********************Íâ²¿µ÷ÓÃ²ÎÊý¼¯ºÏ************************/
/*
		void GprsRec_Date(uint8* Date,uint8 num); ½ÓÊÕÊý¾Ý
		void Gprs_1SEvent(void)										1S
		uint8  Gprs_ReceiveEvent(void)			50ms rtcÊ±¼äº¯ÊýÖÐµ÷ÓÃ
*/

extern NeoWayExternalPar_ NeoWayExternalPar;
//½ÓÊÕµ½TCPÊý¾Ý
extern uint8 g_aTcpRecDate[TCP_REC_DATE_MAX];
extern uint8  g_uRecTcpDateNum;
//·¢ËÍTCPÊý¾Ý
extern uint8 g_aTcpSendDate[NEOWAY_SEND_MAX];
extern uint8  g_uSendTcpDateNum ;
extern uint8 Gprs_Send_Date(void);



/********************ÄÚ²¿²ÎÊý¼¯ºÏ************************/

uint8*   ReceiveOriginal;
uint8   ReceiveDebugData = 0;
uint8 	Original_length = 0;
uint8   SendOriginal[FRAME_MAX_LENGTH] ={0};

Monitor_Target_01 Device_Info;
Monitor_Target_02 Web_Param;
Monitor_Target_03 Alarm_State;
Monitor_Target_04 Set_Param;
Monitor_Target_05 Collect_Data;
GprsFlag Gprs_flag;

unsigned short crc_ta[256]=  
	{0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	 0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	 0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 
	 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 
	 0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 
	 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 
	 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 
	 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067, 
	 0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	 0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	 0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 
	 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	 0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
}; 

/*
** ===================================================================
**     Method      : 	GprsRec_Date 
**     Description :	GPRSÐ­Òé²ã½ÓÊÕÊý¾Ý£¬Í¨¹ýÖ¸Õë¸³Öµ
**     Parameters  : 	Date ½ÓÊÕµ½µÄÊý¾ÝÖ¸Õë£¬°üÀ¨Ö¡Í·Ö¡Î²
**								 £º	num	 ½ÓÊÕµ½ÊÇÊý¾Ý³¤¶È£¬°üÀ¨Ö¡Í·Ö¡Î²
**     Returns     : 	Nothing
** ===================================================================
*/
void GprsRec_Date(uint8* Date,uint8 num)
{	
	if(0x5E==*(Date)&&(0x7E==*(Date+num-1)))
	{
		//¿Û³ýÖ¡Í·¡¢Ö¡Î²
		ReceiveOriginal = Date+1;   
		Original_length = num-2;
		Gprs_flag.bits.Deal_data=ON;		
	}
  NeoWayExternalPar.RecTcpDateState=OFF;
}

/*
** ===================================================================
**     Method      : 	GprsSend_Date 
**     Description :	GPRSÐ­Òé²ã·¢ËÍÊý¾Ý
**     Parameters  : 	Date ·¢ËÍµÄÊý¾ÝÖ¸Õë£¬²»°üÀ¨Ö¡Í·Ö¡Î²
**								 £º	num	 ·¢ËÍµÄÊý¾Ý³¤¶È£¬²»°üÀ¨Ö¡Í·Ö¡Î²
**     Returns     : 	Nothing
** ===================================================================
*/
static void GprsSend_Date(uint8* Date,uint8 num)
{	
	uint8 i=0;
	*(g_aTcpSendDate)=0x5E;
	for(i=0;i<num;i++)
	{
		*(g_aTcpSendDate+i+1)=*(Date+i);
	}
	g_uSendTcpDateNum=2+num;
	*(g_aTcpSendDate+num+1)=0x7E;
	Gprs_Send_Date();
}

/*
** ===================================================================
**     Method      : 	Gprs_1SEvent 
**     Description :	Íâ²¿µ÷ÓÃ1SÊÂ¼þ
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
void Gprs_1SEvent(void)
{	
	if(ON==NeoWayExternalPar.NetWorkConnetState)
	{
		Gprs_FirstOpenEvent();
		Gprs_HeartbeatEvent();
		Gprs_AlarmEvent();
	}
  if(ON == Gprs_flag.bits.StartProgram)
  {
    Gprs_flag.bits.ProtectOpenProgramTime++; 
  }
}

/*
** ===================================================================
**     Method      : 	Gprs_FirstOpenEvent 
**     Description :	¿ª»úÉÏ±¨£¬¹Ø±ÕÐÄÌø°ü£¬Ã¿10SÉÏ±¨£¬Ö±µ½³É¹¦
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
static void Gprs_FirstOpenEvent(void)
{
	static uint8 time = 0; 
	if(Gprs_flag.bits.ReportFirstOpen==ON)
	{
		time++;
		if((2 == time)||(5 == time)||(10 == time))
		{
			Send_FirstOpen(SendOriginal);
		}
    if(time>=11)
    {
        time=11;
    }
	}else
	{
    time=0;
  }
}

/*
** ===================================================================
**     Method      : 	Gprs_AlarmEvent 
**     Description :	¹ÊÕÏÉÏ±¨ÊÂ¼þ£¬¸æ¾¯´¥·¢ºó£¬Ã¿¸ô1·ÖÖÓÉÏ±¨£¬Á¬ÐøÉÏ±¨Èý·ÖÖÓ¡£
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/

static void Gprs_AlarmEvent(void)
{
	static uint8  flag = 0;
  static uint8 Old_ChairAlarm = 0;
  static uint8 Old_GprsAlarm = 0;

  if(Alarm_State.Gprs_Alarm!=Old_GprsAlarm)
  { //GPRS¸æ¾¯ÉÏ±¨
//    Gprs_flag.bits.AlarmFlag =ON;
//    Gprs_flag.bits.ReportAlarm=ON; //¿ªÆôÑ­»·¸æ¾¯±êÖ¾Î»
//    Collect_Data.Chair_ServiceCondition =0x03;
     Send_GprsAlarm(SendOriginal);
//    flag =2;
    Old_GprsAlarm=Alarm_State.Gprs_Alarm;
  }else  if(Master_Inf.Alarm_Num!=Old_ChairAlarm)
  {  //ÒÎ×Ó¸æ¾¯ÉÏ±¨
    Gprs_flag.bits.AlarmFlag =ON;
    Collect_Data.Chair_ServiceCondition =0x03;
    //Set_Param.Lock_Chair=0x01;//Éè±¸ÉÏËø
    Gprs_flag.bits.ReportAlarm=ON; //¿ªÆôÑ­»·¸æ¾¯±êÖ¾Î»
    Alarm_State.Chair_Alarm=Master_Inf.Alarm_Num; //Ð´Èë¸æ¾¯ÐÅÏ¢
    Send_Alarm(SendOriginal);
    flag = 1;
    Old_ChairAlarm=Master_Inf.Alarm_Num;
  }
	if(ON==Gprs_flag.bits.AlarmFlag)
	{
		Gprs_flag.bits.AlarmTime++;
    //¸æ¾¯½â³ý
    if(0 == Master_Inf.Alarm_Num)
    {  
        Alarm_State.Chair_Alarm=0;        
        Collect_Data.Chair_ServiceCondition =0x01;
        flag =0;
        Gprs_flag.bits.AlarmFlag = OFF;
        Gprs_flag.bits.AlarmTime = 0;
        Gprs_flag.bits.ReportAlarm =OFF;
        Old_ChairAlarm = 0;
        Old_GprsAlarm  = 0;
    }    
	}
	if(	(ON==Gprs_flag.bits.AlarmFlag)&&
		((Gprs_flag.bits.AlarmTime==60)||
		(Gprs_flag.bits.AlarmTime==120)||
		(Gprs_flag.bits.AlarmTime==180)||
		(Gprs_flag.bits.AlarmTime==3600)))
	{
		if(Gprs_flag.bits.AlarmTime>=3600)
		{
			Gprs_flag.bits.AlarmFlag=OFF;
			Gprs_flag.bits.AlarmTime=0;
			return;
		}
		Alarm_State.Chair_Alarm=Master_Inf.Alarm_Num;
    if(1 == flag)
    {
    		Send_Alarm(SendOriginal);
    }else if(2 == flag)
    {
        Send_GprsAlarm(SendOriginal);
    }    
	}

}

/*
** ===================================================================
**     Method      : 	Gprs_HeartbeatEvent 
**     Description :	ÐÄÌø°üÊÂ¼þ£¬Ã»ÓÐÊý¾Ý½»»¥60Sºó£¬ÉÏËÍÐÄÌø°ü¡£
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
static void Gprs_HeartbeatEvent(void)
{
	static uint8 flag = 0; 
	Gprs_flag.bits.HeartbeatTime++;	
	if((OFF==Gprs_flag.bits.ReportHeartbeat))
	{
		Gprs_flag.bits.HeartbeatFaultNum = 0;
	}
	if((Gprs_flag.bits.HeartbeatTime>=60)&&(flag==0))// 60S
	{
		Send_Heartbeat(SendOriginal);
	 // Gprs_flag.bits.HeartbeatFaultNum++;		
		Gprs_flag.bits.HeartbeatTime=0;		
	}
	if(Gprs_flag.bits.HeartbeatFaultNum>=3)//´íÎóÀÛ»ý
	{
		flag = 1;
		Gprs_flag.bits.HeartbeatTime = 0;
		Gprs_flag.bits.HeartbeatFaultNum = 0;
	}
	if((1==flag)&&(Gprs_flag.bits.HeartbeatTime>=3600)) //Ò»¸öÐ¡Ê±
	{
		flag=0;
		Gprs_flag.bits.HeartbeatTime=0;	
	}
}

/*
** ===================================================================
**     Method      : 	Gprs_StartCodeEvent 
**     Description :	¿ªÆôÆô¶¯³ÌÐòÑ¡Ôñ¡£
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
uint8 Gprs_StartCodeEvent(void)
{	
  if(Gprs_flag.bits.ProtectOpenProgramTime > PROTECT_START_TIME)
  {
    if((0 == Master_Inf.State.Massage)||(Gprs_flag.bits.ProtectOpenProgramTime >PROTECT_PROGRAM_TIME))
    {
        //Collect_Data.Start_ProgramState = 0x01;
        Gprs_flag.bits.ProtectOpenProgramTime = 0;
        Collect_Data.Chair_ServiceCondition = 0x01;
        Gprs_flag.bits.StartProgram = OFF;
        if(ON==Master_Inf.State.SysPower)
        {
        	SendMaster_KeyValue(1);
        }		
    }
  }	
	if((ON == Gprs_flag.bits.Start_Program)&&(0x01==Collect_Data.Chair_ServiceCondition)
		&&(0x00==Set_Param.Lock_Chair))
	{			
		Collect_Data.Chair_ServiceCondition=0x02;	
    Gprs_flag.bits.StartProgram=ON;
    Gprs_flag.bits.ProtectOpenProgramTime=0;
    if((Set_Param.Start_Program>=5)&&(Set_Param.Start_Program<=25))
    {
        SendMaster_KeyValue(Set_Param.Start_Program);
    }
    Set_Param.Start_Program = 0;
	}
  Gprs_flag.bits.Start_Program=OFF;
	if((ON==Gprs_flag.bits.Stop_Chair))
	{	
		if(0x01==Set_Param.Stop_Chair)
		{
		  if(Master_Inf.State.SysPower)
      {
        SendMaster_KeyValue(1);	
      }			
			Gprs_flag.bits.Stop_Chair=OFF;
			Collect_Data.Chair_ServiceCondition=0x01;
			Collect_Data.Start_ProgramState = 0x01;
		}
	}

	return 1;
}

/*
** ===================================================================
**     Method      : 	Gprs_ReceiveEvent 
**     Description :	½ÓÊÕµ½´®¿ÚÊý¾Ý£¬½øÐÐ´¦Àí¡£
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
uint8  Gprs_ReceiveEvent(void)
{
	volatile uint16 count_crc=0;
	volatile uint8  length=0;
	volatile uint16 temp_crc = 0;
	uint8  source[FRAME_MAX_LENGTH]={0};	
	if(ON == Gprs_flag.bits.Deal_data)			//Êý¾Ý½ÓÊÕÍê±Ï£¬¿ªÊ¼´¦ÀíÊý¾Ý
	{  
	length=ServerToGprs_ChangeMean( ReceiveOriginal, source, Original_length); 	
	count_crc=crc16(source, length-2);	
	if(count_crc==(source[length-2]<<8)		//Òª×¢ÒâlengthÔÚÊý×éÖÐµÄÎ»ÖÃ¹ØÏµ!
		+source[length-1])
	 {		//crcÐ£ÑéÍ¨¹ý									
			if(	Check_Size(source,(length-2)))	//³¤¶ÈÐ£ÑéÍ¨¹ý		
			{							
				if(TRUE==Read_AckCmd(source,SendOriginal,length))//length Îª³ýÁËÖ¡Í·¡¢Ö¡Î²µÄ³¤¶È
				{
					Gprs_flag.bits.Deal_data = 0;
					Original_length = 0;
					Gprs_flag.bits.HeartbeatTime=0;
					return TRUE;
				}
			}
	}	
	Gprs_flag.bits.Deal_data = 0;
	Original_length = 0;
	Gprs_flag.bits.HeartbeatTime=0;
	}
	return FAULT;
}

/*
** ===================================================================
**     Method      : 	Read_AckCmd 
**     Description :	´¦ÀíÓ¦´ðµÄ½ÓÊÕÖ¡
**     Parameters  : 	*source £­Ô´Êý¾ÝÖ¸Õë£»       
*  										*target £­Ä¿±êÊý¾ÝÖ¸Õë£» 
* 										*length £­Ô´Êý¾Ý³¤¶È£» 
**     Returns     : 	0: ±íÊ¾´íÎó  1±íÊ¾ÕýÈ·
** ===================================================================
*/
static uint8 Read_AckCmd(const uint8 *source , uint8 *target ,const uint8 length)
{
    switch(*source)
	{        
      case ORDER_FIRST_OPEN: 
           if((ON == Gprs_flag.bits.ReportFirstOpen)||(ACK_SUCCEED==*(source+1)))
						{
							Gprs_flag.bits.ReportFirstOpen = OFF;
							break;//¿ª»úÉÏ±¨³É¹¦
						}else{return FAULT;}        
        case ORDER_HEARTBEAT:  
					if((ON == Gprs_flag.bits.ReportHeartbeat)||(ACK_SUCCEED==*(source+1)))
						{
							Gprs_flag.bits.ReportHeartbeat = OFF;
							break; //ÐÄÌøÉÏ±¨³É¹¦
						}else{return FAULT;}    
        case ORDER_ALARM:  
					if((ON == Gprs_flag.bits.ReportAlarm)||(ACK_SUCCEED==*(source+1)))
						{
							Gprs_flag.bits.ReportAlarm = OFF;
							Gprs_flag.bits.AlarmFlag = OFF;
							Gprs_flag.bits.AlarmTime = 0;
							break;//¸æ¾¯ÉÏ±¨³É¹¦
						}else{return FAULT;}          
		case ORDER_INQUIRE:
						if(ACK_ORDER_MUST_REPLY == *(source+1))
						{
							Send_InquireInfo(source,target,length);
							break; //½«Êý¾Ý´æ´¢ÔÚtargetÖÐÁË                                //²éÑ¯
						}else{return FAULT;}
		case ORDER_SET:
						if(ACK_ORDER_MUST_REPLY == *(source+1))
						{
							Send_SetInfo(source,target,length);
							break; 			//½«Êý¾Ý´æ´¢ÔÚtargetÖÐÁË 
						}else{return FAULT;}		
    case ORDER_SendInfo:
            if(ACK_ORDER_MUST_REPLY == *(source+1))
						{							
							break; 			//½«Êý¾Ý´æ´¢ÔÚtargetÖÐÁË 
						}else{return FAULT;}
		default: return FAULT ;
	}
    return TRUE;    
}

/*
** ===================================================================
**     Method      : 	crc16 
**     Description :	CCITTÍÆ¼öµÄ16Î»µÄx16+x12+x5+1£¨0x11021£©³ÌÐò
**     Parameters  : 	*ptr £ºÐèÒªÐ£ÑéµÄÊý×éÊ×Ö·
**       								len£ºÒªÐ£ÑéµÄÊý¾Ý¸öÊý
**     Returns     : 	Nothing
** ===================================================================
*/
static unsigned short crc16(uint8 *ptr, int len)   
{
	unsigned short crc=0;
	unsigned char da;
	unsigned short *ConstPtr;
	ConstPtr = crc_ta;
	while(len--!=0) 
	{
		da=(unsigned char)(crc/256);
		crc<<=8;
		crc^=*(ConstPtr+(da^(*ptr)));
		ptr++;
	}
	return(crc);
}

/*
** ===================================================================
**     Method      : 	Check_Size 
**     Description :	¼ì²é×î´ó³¤¶ÈÒÔ¼°¼à¿Ø¶ÔÏó³¤¶ÈËù×é³ÉµÄ×Ü³¤¶È
**     Parameters  : 	*source £­Ô´Êý¾ÝÖ¸Õë£» ²»°üº¬Ö¡Í·     
**		  								length  ²»°üº¬Ö¡Í·¡¢Ö¡Î²¡¢CRC
**     Returns     : 	 0: ±íÊ¾´íÎó 
**										 1: ÕýÈ·
** ===================================================================
*/
static uint8 Check_Size(const uint8 *source, uint8 length)
{
	uint8 i = 0;
	uint8 offset = 2; 
	if(length>FRAME_MAX_LENGTH)
	{
		return FAULT;
	}
	while((length-2)!=i)
	{
		i+=*(source+offset+i);
		if(i>length)return FAULT; //Ð£ÑéÊý¾ÝÄÚÈÝ³¤¶È
	}
	return TRUE;
}

/*
** ===================================================================
**     Method      : 	Init_Gprs_Device 
**     Description :	³õÊ¼»¯GPRSÐ­Òé²ãÄÚÈÝ
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
void Init_Gprs_Device(void)
{
	uint8 i = 0;
//	uint8 temp_ip_1[4]  =SERVER_IP_1;
	uint8 temp_port_1[2] =SERVER_PORT_1; 
	for(i=0;i<SERVER_PORT_LENGTH;i++)
	{
		Web_Param.Server_Port_1[i] = temp_port_1[i];//·þÎñÆ÷¶Ë¿ÚºÅ
	}
  NeoWaySysPar.NetWork.IpSelectNum = 1;
	Alarm_State.Chair_Alarm = 0x00;
  Alarm_State.Gprs_Alarm  =0x00;
	Set_Param.Lock_Chair=0x00;    //0x00±íÊ¾½âËø
	Collect_Data.Chair_ServiceCondition = 0x01; 	
	Gprs_flag.flag = 0x0000;
	Gprs_flag.bits.ReportFirstOpen = ON; 
	Collect_Data.Start_ProgramState = 0x01;
}

/*
** ===================================================================
**     Method      : 	ServerToGprs_ChangeMean 
**     Description :	½ÓÊÕ·­Òë×ªÒå×Ö·û
**     Parameters  : 	*source £­Ô´Êý¾ÝÖ¸Õë£»ÒÑÈ¥³ýÖ¡Í·¡¢Ö¡Î²¡£
**       							*target £­Ä¿µÄÊý¾ÝÖ¸Õë£»
**       		 					*length £­Ô´Êý¾ÝÊý¾Ý³¤¶È£»
**     Returns     : 	×ª»»ºóµÄÄ¿µÄÊý¾Ý³¤¶È,Èç¹ûÎª0£¬ËµÃ÷³¤¶È³¬¹ý×î´ó³¤¶È
** 										×ªÒå¹æÔò£ºÓÃ0x1E£¬0x5DÀ´´úÌæ0x5E£»ÓÃ0x1E£¬0x7DÀ´´úÌæ0x7E£»ÓÃ0x1E£¬0x00À´´úÌæ0x1E
**										Ö¡³¤¶ÈÓ¦¸ÃÏÞÖÆÔÚ×î´ó³¤¶ÈÄÚ¡£
** ===================================================================
*/
static  uint8 ServerToGprs_ChangeMean(  uint8 *source, uint8 *target,uint8 source_length )
{
	uint8 i = 0;
	uint8 target_length=0;
	if((source_length >=FRAME_MAX_LENGTH))
	{
		return FAULT;
	}else
	{
		while(i < source_length)
		{
			if(0x1E==*(source+i))
			{
					if (0x5D==*(source+i+1))
				{            
				 *(target+target_length)=0x5E;
				 i+=2;
					}else if(0x7D==*(source+i+1))
					{
				 *(target+target_length)=0x7E;
				 i+=2;
					}else if(0x00==*(source+i+1))
					{
				 *(target+target_length)=0x1E;
				 i+=2;
					}else
					{
				 *(target+target_length)=*(source+i);
				 i++;                
					}
			}else 
			{
			 *(target+target_length)=*(source+i);
			 i++;                     
			}
			target_length++;
		}    
		return target_length;
	}
}

/*
** ===================================================================
**     Method      : 	GprsToServer_ChangeMean 
**     Description :	·¢ËÍ×ª»»×ªÒå×Ö·û
**     Parameters  : 	*source £­Ô´Êý¾ÝÖ¸Õë£»
**        						*target £­Ä¿µÄÊý¾ÝÖ¸Õë£»²»°üº¬Ö¡Í·Ö¡Î²
**        						*length £­Ô´Êý¾ÝÊý¾Ý³¤¶È£»
**     Returns     : 	 ×ª»»ºóµÄÄ¿µÄÊý¾Ý³¤¶È,Èç¹ûÎª1£¬ËµÃ÷³¤¶È³¬¹ý×î´ó³¤¶È
**										 ×ªÒå¹æÔò£ºÓÃ0x1E£¬0x5DÀ´´úÌæ0x5E£»ÓÃ0x1E£¬0x7DÀ´´úÌæ0x7E£»ÓÃ0x1E£¬0x00À´´úÌæ0x1E
**          					 Ö¡³¤¶ÈÓ¦¸ÃÏÞÖÆÔÚ×î´ó³¤¶ÈÄÚ¡£
** ===================================================================
*/
static uint8 GprsToServer_ChangeMean(  uint8 *source, uint8 *target,uint8 source_length )
{
	uint8 i=0;
	uint8 target_length =0 ;
	while(i < source_length)
	{
		if(0x1E == *(source+i))
		{
			*(target+target_length)=0x1E;            
			target_length++;
			*(target+target_length)=0x00; 
		}else if(0x5E == *(source+i))
		{
			*(target+target_length)=0x1E;            
			target_length++;
			*(target+target_length)=0x5D; 
		}else if(0x7E == *(source+i))
		{
			*(target+target_length)=0x1E;            
			target_length++;
			*(target+target_length)=0x7D; 
		}else
		{
			*(target+target_length)=*(source+i);            
		}
		target_length++;
		i++;
	}    
	if(target_length>FRAME_MAX_LENGTH)
	{
		return FAULT;
	}else
	{
		return target_length;
	}    
}

/*
** ===================================================================
**     Method      : 	Monitor_Quantity 
**     Description :	Í³¼Æ¼à¿Ø¶ÔÏóµÄ¸öÊý
**     Parameters  : 	*source £­Ô´Êý¾ÝÖ¸Õë£»
**        						*total_length £­Ô´Êý¾ÝÊý¾Ý³¤¶È£»
**     Returns     : 	¼à¿Ø¶ÔÏó¸öÊý
** ===================================================================
*/
static uint8 Monitor_Quantity( const uint8 * source,const uint8 total_length)
{
	int16 temp = 0 ;
	uint8 offset =2;
	uint8 i=0;
	uint8 quantity=0;
	const uint8 *p;
	p=source;
	//temp=total_length-offset-2;  //È¥³ý ÃüÁî±êÖ¾ ºÍÓ¦´ð±êÖ¾ //¿Û³ýCRC
	temp=0;
	while( temp!=(total_length-4) )
	{
		temp+= *(p+offset+i);
		i+=*(p+offset+i);
		quantity++;
	}
	return quantity;
}

/*
** ===================================================================
**     Method      : 	Send_InquireInfo 
**     Description :	·¢ËÍÇëÇó²éÑ¯µÄÊý¾Ý
**     Parameters  : 	*source £­Ô´Êý¾ÝÖ¸Õë£»
**        						*target £­Ä¿µÄÊý¾ÝÖ¸Õë£»²»°üº¬Ö¡Í·¡¢Ö¡Î²
**        						*length £­Ô´Êý¾ÝÊý¾Ý³¤¶È£»
**     Returns     : 	Ä¿±êÊý¾ÝÖ¸ÕëµÄ³¤¶È
** ===================================================================
*/
static uint8 Send_InquireInfo(const uint8 * source,uint8 *target,const uint8 total_length)
{
	 volatile uint8 length = 0;
	 volatile static  uint16 Cnt=0;
	 uint8 i = 0,m = 0;
	 uint8 quantity = 0;
	 uint8 immobilization = 2; //¹Ì¶¨Æ«ÒÆ2¸öµ¥Î»¡£ÃüÁî±êÖ¾ÓëÓ¦´ð±êÖ¾
	 uint8 immobilization_3 = 3; //¹Ì¶¨Æ«ÒÆ3¸öµ¥Î»¡£¼à¿Ø³¤¶Èµ¥ÔªÓëµØÖ·µ¥Ôª
	 uint8 temp_offset = 0; 
	 uint8 *p; 	 
	 uint16 temp_crc = 0;
	 *(target+length) = ORDER_INQUIRE; length++;
	 *(target+length) = ACK_SUCCEED; length++; 
	 quantity = Monitor_Quantity(source,total_length);  //»ñµÃ¼à¿Ø¶ÔÏó ÊýÁ¿
	 for(i=0;i<quantity;i++)
	 {	 	
	 	*(target+length) = *(source+temp_offset+2); length++;  //¼à¿Ø¶ÔÏó¸öÊý
		*(target+length) = *(source+temp_offset+3); length++;  //¼à¿Ø¶ÔÏó±êºÅ¸ßÎ»µØÖ·
		*(target+length) = *(source+temp_offset+4); length++;  //¼à¿Ø¶ÔÏó±êºÅµÍÎ»µØÖ·	
		p=Head_SearchAddr((source+temp_offset+3));  //»ñµÃÊý¾ÝµÄÊ×µØÖ·	
		  for(m=0;m<(*(source+temp_offset+2)-immobilization_3);m++)
		  {   //¿Û³ý3²ÅÊÇÕæÕýµÄÊý¾Ý³¤¶È
			*(target+length) = *(p+m); length++;    //Êý¾Ý¶Á³ö
		  }
			temp_offset+=*(source+temp_offset+immobilization); //temp_offset->Æ«ÒÆµÄÎ»ÖÃ(¼à¿Ø¶ÔÏó¸öÊý)
	 }
	temp_crc = crc16(target,length); //×°ÔØCRC
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	Cnt++;
	return TRUE;
}

/*
** ===================================================================
**     Method      : 	Send_SetInfo 
**     Description :	·¢ËÍÇëÇóÉèÖÃµÄÊý¾Ý£¬²¢ÇÒÐ´ÈëÊý¾Ý
**     Parameters  : 	*source £­Ô´Êý¾ÝÖ¸Õë£»
**        						*target £­Ä¿µÄÊý¾ÝÖ¸Õë£»²»°üº¬Ö¡Í·¡¢Ö¡Î²
**       							*length £­Ô´Êý¾ÝÊý¾Ý³¤¶È£
**     Returns     : 	Ä¿±êÊý¾ÝÖ¸ÕëµÄ³¤¶È,
** ===================================================================
*/
static uint8 Send_SetInfo(const uint8 * source,uint8 *target,const uint8 total_length)
{
	uint8 length = 0;
	uint8 i = 0,m = 0;
	uint8 quantity = 0;
	uint8 immobilization_2 = 2; //¹Ì¶¨Æ«ÒÆ2¸öµ¥Î»¡£ÃüÁî±êÖ¾ÓëÓ¦´ð±êÖ¾
	uint8 immobilization_3 = 3; //¹Ì¶¨Æ«ÒÆ3¸öµ¥Î»¡£¼à¿Ø³¤¶Èµ¥ÔªÓëµØÖ·µ¥Ôª
	uint8 temp_offset = 0; 
	uint8 *p; 	 
	uint16 temp_crc = 0;
	Gprs_flag.bits.ReceiveSet = ON;
	*(target+length) = ORDER_SET; length++;
	*(target+length) = ACK_SUCCEED; length++; 
	quantity = Monitor_Quantity(source,total_length);  //»ñµÃ¼à¿Ø¶ÔÏó ÊýÁ¿
	for(i=0;i<quantity;i++)
	{
		*(target+length) = *(source+temp_offset+2); length++;  //¼à¿Ø¶ÔÏó¸öÊý
		*(target+length) = *(source+temp_offset+3); length++;  //¼à¿Ø¶ÔÏó±êºÅ¸ßÎ»µØÖ·
		*(target+length) = *(source+temp_offset+4); length++;  //¼à¿Ø¶ÔÏó±êºÅµÍÎ»µØÖ·	
		p=Head_SearchAddr((source+temp_offset+3));  //»ñµÃÊý¾ÝµÄÊ×µØÖ·
		for(m=0;m<(*(source+temp_offset+2)-immobilization_3);m++)
		{  //¿Û³ý3²ÅÊÇÕæÕýµÄÊý¾Ý³¤¶È
			*(p+m)=*(source+temp_offset+2+immobilization_3+m);  //Êý¾ÝÐ´Èë
			*(target+length) = *(p+m); length++;  
		}
		temp_offset+=*(source+temp_offset+immobilization_2); //temp_offset->Æ«ÒÆµÄÎ»ÖÃ(¼à¿Ø¶ÔÏó¸öÊý)
	}	 
  if(ON==Gprs_flag.bits.Start_Program)
  {
    if((Master_Inf.State.Massage>0)
      ||(ON == Set_Param.Lock_Chair)
      ||(0x01 != Collect_Data.Chair_ServiceCondition)
      ||(0!=Alarm_State.Chair_Alarm)
      ||(0!=Alarm_State.Gprs_Alarm))
    { //°´Ä¦ÒÎÊ±¼ä²»Î´0£¬Éè±¸ÉÏËø¡¢gprsÎÞ¹ÊÕÏ¡¢Éè±¸ÎÞ¹ÊÕÏ
        *(target+length-1) =0xff;
        Gprs_flag.bits.Start_Program=OFF;
    }
  }
	temp_crc = crc16(target,length); //×°ÔØCRC
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	Gprs_flag.bits.ReceiveSet=OFF;	
	return TRUE;
}

/*
** ===================================================================
**     Method      : 	Send_FirstOpen 
**     Description :	¿ª»úÉÏ±¨-´«ÊäÉè±¸ÐòÁÐºÅ   ÎÞCRCÒÔ¼°Ö¡Î²
**     Parameters  : 	*target £­Ä¿±êÊý¾ÝÖ¸Õë£»  
**     Returns     : 	Ä¿±êÊý¾Ý³¤¶È
** ===================================================================
*/
static uint8 Send_FirstOpen(uint8 *target)
{
	uint8 i = 0;
	uint8 length = 0;
	uint16 temp_crc = 0;
	*(target+length) = ORDER_FIRST_OPEN; length++;
	*(target+length) = ACK_ORDER_MUST_REPLY; length++;  
	*(target+length) = (1+2+SERIAL_ID_LENGTH);length++;
	*(target+length) = Head_DEV;length++;
	*(target+length) = Serial_Id;length++;
	for(i=0;i<SERIAL_ID_LENGTH;i++)
	{
		*(target+length) = Device_Info.Serial_Id[i];length++;        
  }
	temp_crc = crc16(target,length);
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	return length ;
}

/*
** ===================================================================
**     Method      : 	Send_Heartbeat 
**     Description :	ÐÄÌø°ü-´«ÊäÉè±¸ÐòÁÐºÅ
**     Parameters  : 	*target £­Ä¿±êÊý¾ÝÖ¸Õë£»  
**     Returns     : 	Ä¿±êÊý¾Ý³¤¶È
** ===================================================================
*/
static uint8 Send_Heartbeat(uint8 *target)
{
	uint8 i = 0;
	uint8 length = 0;
	uint16 temp_crc = 0;	
	Gprs_flag.bits.ReportHeartbeat = ON;      
	*(target+length) = ORDER_HEARTBEAT; length++;
	*(target+length) = ACK_ORDER_MUST_REPLY; length++;  
	*(target+length) = (1+2+SERIAL_ID_LENGTH);length++;
	*(target+length) = Head_DEV;length++;
	*(target+length) = Serial_Id;length++;
	for(i=0;i<SERIAL_ID_LENGTH;i++)
	{
		*(target+length) = Device_Info.Serial_Id[i];length++;        
	}	
	temp_crc = crc16(target,length);
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	Gprs_flag.bits.ReportHeartbeat = ON;
	return length ;
}

/*
** ===================================================================
**     Method      : 	Send_Alarm 
**     Description :	ÉÏ±¨Éè±¸¸æ¾¯
**     Parameters  : 	*target £­Ä¿±êÊý¾ÝÖ¸Õë£» 
**     Returns     : 	Ä¿±êÊý¾Ý³¤¶È
** ===================================================================
*/
static uint8 Send_Alarm(uint8 *target)
{    
	uint8 length = 0;
	uint16 temp_crc = 0;
	Gprs_flag.bits.ReportAlarm = ON;   
	*(target+length) = ORDER_ALARM; length++;
	*(target+length) = ACK_ORDER_MUST_REPLY; length++;  
	*(target+length) = (1+2+CHAIR_ALARM);length++;
	*(target+length) = Head_ALA;length++;
	*(target+length) = Chair_Alarm;length++;
	*(target+length) = Alarm_State.Chair_Alarm;length++;
	temp_crc = crc16(target,length);
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	return length ;
}
/*
** ===================================================================
**     Method      : 	Send_GprsAlarm 
**     Description :	ÉÏ±¨GPRS¸æ¾¯
**     Parameters  : 	*target £­Ä¿±êÊý¾ÝÖ¸Õë£» 
**     Returns     : 	Ä¿±êÊý¾Ý³¤¶È
** ===================================================================
*/
static uint8 Send_GprsAlarm(uint8 *target)
{    
	uint8 length = 0;
	uint16 temp_crc = 0;
	Gprs_flag.bits.ReportAlarm = ON;   
	*(target+length) = ORDER_ALARM; length++;
	*(target+length) = ACK_ORDER_MUST_REPLY; length++;  
	*(target+length) = (1+2+CHAIR_ALARM);length++;
	*(target+length) = Head_ALA;length++;
	*(target+length) = Gprs_Alarm;length++;
	*(target+length) = Alarm_State.Gprs_Alarm;length++;
	temp_crc = crc16(target,length);
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	return length ;
}
/*
** ===================================================================
**     Method      : 	Head_SearchAddr 
**     Description :	ÓÃ¼à¿Ø¶ÔÏó±êºÅµÄµÚÒ»¸öµØÖ·Ñ°ÕÒ£¬¼à¿Ø¶ÔÏó¶ÔÓ¦µÄÊý¾Ý´æ´¢µØÖ·
**     Parameters  : 	*monitor_head  ¼à¿Ø¶ÔÏó±êºÅµÄÊ×µØÖ·  
**     Returns     : 	ÆäÓà·µ»Ø¼à¿Ø¶ÔÏó¶ÔÓ¦µÄ´æ´¢Êý¾Ýµ¥ÔªµØÖ·
** ===================================================================
*/
static uint8* Head_SearchAddr(const uint8 *monitor_head)
{
	switch(*monitor_head)
	{
		case Head_DEV : return(Matching_DEV(monitor_head+1));
		case Head_WEB : return(Matching_WEB(monitor_head+1));
		case Head_ALA : return(Matching_ALA(monitor_head+1));
		case Head_SET : return(Matching_SET(monitor_head+1));
		case Head_Coll: return(Matching_Coll(monitor_head+1));
		default  	  : return  NULL;
	}
}

/*
** ===================================================================
**     Method      : 	Matching_DEV 
**     Description :	·µ»ØÖ¸ÕëµØÖ·
**     Parameters  : 	
**     Returns     : 	
** ===================================================================
*/

static uint8* Matching_DEV(const uint8 *dev)
{
	switch(*dev)
	{
		case Serial_Id 		: return (Device_Info.Serial_Id);
    case Cell_Id 		  : return (Device_Info.Cell_Id);
    case Local_Id 		: return (Device_Info.Local_Id);
		default  	   		: return  NULL;
	}
}
static uint8* Matching_WEB(const uint8 *web)
{
	
	switch(*web)
	{
		case Server_Ip_1   	: return (Web_Param.Server_Ip_1);
		case Server_Port_1 	: return (Web_Param.Server_Port_1);
		case Server_Ip_2   	: return (Web_Param.Server_Ip_2);
		case Server_Port_2 	: return (Web_Param.Server_Port_2);
    case Server_Ip_3   	: return (Web_Param.Server_Ip_3);
		case Server_Port_3 	: return (Web_Param.Server_Port_3);
		default  	   		: return  NULL;
	}
}
static uint8* Matching_ALA(const uint8 *ala)
{
	switch(*ala)
	{
		case Chair_Alarm 	: return (&Alarm_State.Chair_Alarm);
    case Gprs_Alarm 	: return (&Alarm_State.Gprs_Alarm);
		default  	   		: return  NULL;
	}
}

static uint8* Matching_SET(const uint8 *set)
{
	switch(*set)
	{
		case Start_Program:if(Gprs_flag.bits.ReceiveSet==ON){Gprs_flag.bits.Start_Program=ON;} return (&Set_Param.Start_Program);
		case Stop_Chair		:if(Gprs_flag.bits.ReceiveSet==ON){Gprs_flag.bits.Stop_Chair=ON;} return (&Set_Param.Stop_Chair);
		case Lock_Chair		:if(Gprs_flag.bits.ReceiveSet==ON){Gprs_flag.bits.Lock_Chair=ON;} return (&Set_Param.Lock_Chair);
		default  	   		  : return  NULL;
	}
}

static uint8* Matching_Coll(const uint8 *coll)
{
	switch(*coll)
	{
		case Chair_ServiceCondition :
				return (&Collect_Data.Chair_ServiceCondition);
    case Start_ProgramState  :
        return (&Collect_Data.Start_ProgramState);
    case Chair_State  :
      return(&Collect_Data.Chair_State);
    case Program_State:
      return(&Collect_Data.Program_State);
		default: return  NULL;
	}
}
/*
** ===================================================================
**     Method      : 	Group_SendData 
**     Description :	·¢ËÍÊý¾Ý£¬º¯ÊýÄÚ½øÐÐ×ªÒâ
**     Parameters  : 	*monitor_head  ¼à¿Ø¶ÔÏó±êºÅµÄÊ×µØÖ·
**     Returns     : 	NULL : ±íÊ¾´íÎó
**									ÆäÓà·µ»Ø¼à¿Ø¶ÔÏó¶ÔÓ¦µÄ´æ´¢Êý¾Ýµ¥ÔªµØÖ·
** ===================================================================
*/
static void Group_SendData(uint8 *source,uint8 length)
{
	static uint8 target[FRAME_MAX_LENGTH]={0};
	uint8 temp_length = 0;
	temp_length=GprsToServer_ChangeMean(source,target,length);
  GprsSend_Date(target,temp_length); 
}


void Send_ChangeState(uint8 *target,uint16 Monitor_Target)
{
	uint8 length = 0;
	uint16 temp_crc = 0;
	*(target+length) = ORDER_SendInfo; length++;
	*(target+length) = ACK_ORDER_MUST_REPLY; length++;  
	*(target+length) = (1+2+1);length++;

	*(target+length) = (uint8)(Monitor_Target>>8);length++;
	*(target+length) = (uint8)Monitor_Target;length++;
  if(0x0502 == Monitor_Target)
  {
    *(target+length) =Collect_Data.Start_ProgramState;length++;
  }else if(0x0503 == Monitor_Target)
  {
    *(target+length) =Collect_Data.Chair_State;length++;
  }else 
  {
    return;
  }
	temp_crc = crc16(target,length);
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length); 
}

void ChairStateChange(void)
{
    if((0!=Gprs_flag.bits.StateChange)&&(ON==NeoWayExternalPar.NetWorkConnetState))
    {
        if(0x01 == (Gprs_flag.bits.StateChange&0x01))
        {
           // Send_ChangeState(SendOriginal,0x0502);
            Gprs_flag.bits.StateChange&=0xFE;
        }else if(0x02 == (Gprs_flag.bits.StateChange&0x02))
        {
            Send_ChangeState(SendOriginal,0x0503);
            Gprs_flag.bits.StateChange&=0xFD;            
        }
    }

}

