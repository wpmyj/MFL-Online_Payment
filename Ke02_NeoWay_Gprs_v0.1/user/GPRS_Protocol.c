/*
** ===================================================================
**     File Name   :	GPRS_Protocol.c
**     Author			 :	Deric-Easepal
**     Versions  	 : 	v0.2  2016-10-29
**		 Port				 :	
**     Modification: 
**			2016-10-29 ������֮ǰд�ã��������з�GPRS��������
**						note ���澯ģ������д
										 ����������ҲҪ��д
										 ȥ�������жϽ��պ���
										 ���յ�����ʹ��ָ��ָ�򣬷��͵����ݣ�ʹ�����ݸ���
							note : ���������������޸�
							note : ipд��eeprom ��� ��δʵ��
** ===================================================================
*/

#include "GPRS_Protocol.h"

/********************�����ⲿ��������************************/
/*
			NeoWayExternalPar            ״̬��Ϣ
											ֻҪ��	NeoWayExternalPar.HardwareRebootState
															NeoWayExternalPar.SofewareRebootState ��λ���ͻ�ʹ�ö�Ӧ�ķ�ʽ����ģ��
															
			g_aTcpRecDate          ���յ����ݵ�����
			g_uRecTcpDateNum			 ���յ������ݳ��� 
			
			g_aTcpSendDate				 �������ݵ�����
			g_uSendTcpDateNum			 �������ݵ����ݳ���
			
			extern uint8 Gprs_Send_Date(void);
*/

/********************�ⲿ���ò�������************************/
/*
		void GprsRec_Date(uint8* Date,uint8 num); ��������
		void Gprs_1SEvent(void)										1S
		uint8  Gprs_ReceiveEvent(void)			50ms rtcʱ�亯���е���
*/

extern NeoWayExternalPar_ NeoWayExternalPar;
//���յ�TCP����
extern uint8 g_aTcpRecDate[TCP_REC_DATE_MAX];
extern uint8  g_uRecTcpDateNum;
//����TCP����
extern uint8 g_aTcpSendDate[NEOWAY_SEND_MAX];
extern uint8  g_uSendTcpDateNum ;
extern uint8 Gprs_Send_Date(void);



/********************�ڲ���������************************/

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
**     Description :	GPRSЭ���������ݣ�ͨ��ָ�븳ֵ
**     Parameters  : 	Date ���յ�������ָ�룬����֡ͷ֡β
**								 ��	num	 ���յ������ݳ��ȣ�����֡ͷ֡β
**     Returns     : 	Nothing
** ===================================================================
*/
void GprsRec_Date(uint8* Date,uint8 num)
{	
	if(0x5E==*(Date)&&(0x7E==*(Date+num-1)))
	{
		//�۳�֡ͷ��֡β
		ReceiveOriginal = Date+1;   
		Original_length = num-2;
		Gprs_flag.bits.Deal_data=ON;		
	}
  NeoWayExternalPar.RecTcpDateState=OFF;
}

/*
** ===================================================================
**     Method      : 	GprsSend_Date 
**     Description :	GPRSЭ��㷢������
**     Parameters  : 	Date ���͵�����ָ�룬������֡ͷ֡β
**								 ��	num	 ���͵����ݳ��ȣ�������֡ͷ֡β
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
**     Description :	�ⲿ����1S�¼�
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
**     Description :	�����ϱ����ر���������ÿ10S�ϱ���ֱ���ɹ�
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
**     Description :	�����ϱ��¼����澯������ÿ��1�����ϱ��������ϱ������ӡ�
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
  { //GPRS�澯�ϱ�
//    Gprs_flag.bits.AlarmFlag =ON;
//    Gprs_flag.bits.ReportAlarm=ON; //����ѭ���澯��־λ
//    Collect_Data.Chair_ServiceCondition =0x03;
     Send_GprsAlarm(SendOriginal);
//    flag =2;
    Old_GprsAlarm=Alarm_State.Gprs_Alarm;
  }else  if(Master_Inf.Alarm_Num!=Old_ChairAlarm)
  {  //���Ӹ澯�ϱ�
    Gprs_flag.bits.AlarmFlag =ON;
    Collect_Data.Chair_ServiceCondition =0x03;
    //Set_Param.Lock_Chair=0x01;//�豸����
    Gprs_flag.bits.ReportAlarm=ON; //����ѭ���澯��־λ
    Alarm_State.Chair_Alarm=Master_Inf.Alarm_Num; //д��澯��Ϣ
    Send_Alarm(SendOriginal);
    flag = 1;
    Old_ChairAlarm=Master_Inf.Alarm_Num;
  }
	if(ON==Gprs_flag.bits.AlarmFlag)
	{
		Gprs_flag.bits.AlarmTime++;
    //�澯���
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
**     Description :	�������¼���û�����ݽ���60S��������������
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
	if(Gprs_flag.bits.HeartbeatFaultNum>=3)//�����ۻ�
	{
		flag = 1;
		Gprs_flag.bits.HeartbeatTime = 0;
		Gprs_flag.bits.HeartbeatFaultNum = 0;
	}
	if((1==flag)&&(Gprs_flag.bits.HeartbeatTime>=3600)) //һ��Сʱ
	{
		flag=0;
		Gprs_flag.bits.HeartbeatTime=0;	
	}
}

/*
** ===================================================================
**     Method      : 	Gprs_StartCodeEvent 
**     Description :	������������ѡ��
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
**     Description :	���յ��������ݣ����д���
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
	if(ON == Gprs_flag.bits.Deal_data)			//���ݽ�����ϣ���ʼ��������
	{  
	length=ServerToGprs_ChangeMean( ReceiveOriginal, source, Original_length); 	
	count_crc=crc16(source, length-2);	
	if(count_crc==(source[length-2]<<8)		//Ҫע��length�������е�λ�ù�ϵ!
		+source[length-1])
	 {		//crcУ��ͨ��									
			if(	Check_Size(source,(length-2)))	//����У��ͨ��		
			{							
				if(TRUE==Read_AckCmd(source,SendOriginal,length))//length Ϊ����֡ͷ��֡β�ĳ���
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
**     Description :	����Ӧ��Ľ���֡
**     Parameters  : 	*source ��Դ����ָ�룻       
*  										*target ��Ŀ������ָ�룻 
* 										*length ��Դ���ݳ��ȣ� 
**     Returns     : 	0: ��ʾ����  1��ʾ��ȷ
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
							break;//�����ϱ��ɹ�
						}else{return FAULT;}        
        case ORDER_HEARTBEAT:  
					if((ON == Gprs_flag.bits.ReportHeartbeat)||(ACK_SUCCEED==*(source+1)))
						{
							Gprs_flag.bits.ReportHeartbeat = OFF;
							break; //�����ϱ��ɹ�
						}else{return FAULT;}    
        case ORDER_ALARM:  
					if((ON == Gprs_flag.bits.ReportAlarm)||(ACK_SUCCEED==*(source+1)))
						{
							Gprs_flag.bits.ReportAlarm = OFF;
							Gprs_flag.bits.AlarmFlag = OFF;
							Gprs_flag.bits.AlarmTime = 0;
							break;//�澯�ϱ��ɹ�
						}else{return FAULT;}          
		case ORDER_INQUIRE:
						if(ACK_ORDER_MUST_REPLY == *(source+1))
						{
							Send_InquireInfo(source,target,length);
							break; //�����ݴ洢��target����                                //��ѯ
						}else{return FAULT;}
		case ORDER_SET:
						if(ACK_ORDER_MUST_REPLY == *(source+1))
						{
							Send_SetInfo(source,target,length);
							break; 			//�����ݴ洢��target���� 
						}else{return FAULT;}		
    case ORDER_SendInfo:
            if(ACK_ORDER_MUST_REPLY == *(source+1))
						{							
							break; 			//�����ݴ洢��target���� 
						}else{return FAULT;}
		default: return FAULT ;
	}
    return TRUE;    
}

/*
** ===================================================================
**     Method      : 	crc16 
**     Description :	CCITT�Ƽ���16λ��x16+x12+x5+1��0x11021������
**     Parameters  : 	*ptr ����ҪУ���������ַ
**       								len��ҪУ������ݸ���
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
**     Description :	�����󳤶��Լ���ض��󳤶�����ɵ��ܳ���
**     Parameters  : 	*source ��Դ����ָ�룻 ������֡ͷ     
**		  								length  ������֡ͷ��֡β��CRC
**     Returns     : 	 0: ��ʾ���� 
**										 1: ��ȷ
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
		if(i>length)return FAULT; //У���������ݳ���
	}
	return TRUE;
}

/*
** ===================================================================
**     Method      : 	Init_Gprs_Device 
**     Description :	��ʼ��GPRSЭ�������
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
		Web_Param.Server_Port_1[i] = temp_port_1[i];//�������˿ں�
	}
  NeoWaySysPar.NetWork.IpSelectNum = 1;
	Alarm_State.Chair_Alarm = 0x00;
  Alarm_State.Gprs_Alarm  =0x00;
	Set_Param.Lock_Chair=0x00;    //0x00��ʾ����
	Collect_Data.Chair_ServiceCondition = 0x01; 	
	Gprs_flag.flag = 0x0000;
	Gprs_flag.bits.ReportFirstOpen = ON; 
	Collect_Data.Start_ProgramState = 0x01;
}

/*
** ===================================================================
**     Method      : 	ServerToGprs_ChangeMean 
**     Description :	���շ���ת���ַ�
**     Parameters  : 	*source ��Դ����ָ�룻��ȥ��֡ͷ��֡β��
**       							*target ��Ŀ������ָ�룻
**       		 					*length ��Դ�������ݳ��ȣ�
**     Returns     : 	ת�����Ŀ�����ݳ���,���Ϊ0��˵�����ȳ�����󳤶�
** 										ת�������0x1E��0x5D������0x5E����0x1E��0x7D������0x7E����0x1E��0x00������0x1E
**										֡����Ӧ����������󳤶��ڡ�
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
**     Description :	����ת��ת���ַ�
**     Parameters  : 	*source ��Դ����ָ�룻
**        						*target ��Ŀ������ָ�룻������֡ͷ֡β
**        						*length ��Դ�������ݳ��ȣ�
**     Returns     : 	 ת�����Ŀ�����ݳ���,���Ϊ1��˵�����ȳ�����󳤶�
**										 ת�������0x1E��0x5D������0x5E����0x1E��0x7D������0x7E����0x1E��0x00������0x1E
**          					 ֡����Ӧ����������󳤶��ڡ�
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
**     Description :	ͳ�Ƽ�ض���ĸ���
**     Parameters  : 	*source ��Դ����ָ�룻
**        						*total_length ��Դ�������ݳ��ȣ�
**     Returns     : 	��ض������
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
	//temp=total_length-offset-2;  //ȥ�� �����־ ��Ӧ���־ //�۳�CRC
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
**     Description :	���������ѯ������
**     Parameters  : 	*source ��Դ����ָ�룻
**        						*target ��Ŀ������ָ�룻������֡ͷ��֡β
**        						*length ��Դ�������ݳ��ȣ�
**     Returns     : 	Ŀ������ָ��ĳ���
** ===================================================================
*/
static uint8 Send_InquireInfo(const uint8 * source,uint8 *target,const uint8 total_length)
{
	 volatile uint8 length = 0;
	 volatile static  uint16 Cnt=0;
	 uint8 i = 0,m = 0;
	 uint8 quantity = 0;
	 uint8 immobilization = 2; //�̶�ƫ��2����λ�������־��Ӧ���־
	 uint8 immobilization_3 = 3; //�̶�ƫ��3����λ����س��ȵ�Ԫ���ַ��Ԫ
	 uint8 temp_offset = 0; 
	 uint8 *p; 	 
	 uint16 temp_crc = 0;
	 *(target+length) = ORDER_INQUIRE; length++;
	 *(target+length) = ACK_SUCCEED; length++; 
	 quantity = Monitor_Quantity(source,total_length);  //��ü�ض��� ����
	 for(i=0;i<quantity;i++)
	 {	 	
	 	*(target+length) = *(source+temp_offset+2); length++;  //��ض������
		*(target+length) = *(source+temp_offset+3); length++;  //��ض����Ÿ�λ��ַ
		*(target+length) = *(source+temp_offset+4); length++;  //��ض����ŵ�λ��ַ	
		p=Head_SearchAddr((source+temp_offset+3));  //������ݵ��׵�ַ	
		  for(m=0;m<(*(source+temp_offset+2)-immobilization_3);m++)
		  {   //�۳�3�������������ݳ���
			*(target+length) = *(p+m); length++;    //���ݶ���
		  }
			temp_offset+=*(source+temp_offset+immobilization); //temp_offset->ƫ�Ƶ�λ��(��ض������)
	 }
	temp_crc = crc16(target,length); //װ��CRC
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	Cnt++;
	return TRUE;
}

/*
** ===================================================================
**     Method      : 	Send_SetInfo 
**     Description :	�����������õ����ݣ�����д������
**     Parameters  : 	*source ��Դ����ָ�룻
**        						*target ��Ŀ������ָ�룻������֡ͷ��֡β
**       							*length ��Դ�������ݳ��ȣ
**     Returns     : 	Ŀ������ָ��ĳ���,
** ===================================================================
*/
static uint8 Send_SetInfo(const uint8 * source,uint8 *target,const uint8 total_length)
{
	uint8 length = 0;
	uint8 i = 0,m = 0;
	uint8 quantity = 0;
	uint8 immobilization_2 = 2; //�̶�ƫ��2����λ�������־��Ӧ���־
	uint8 immobilization_3 = 3; //�̶�ƫ��3����λ����س��ȵ�Ԫ���ַ��Ԫ
	uint8 temp_offset = 0; 
	uint8 *p; 	 
	uint16 temp_crc = 0;
	Gprs_flag.bits.ReceiveSet = ON;
	*(target+length) = ORDER_SET; length++;
	*(target+length) = ACK_SUCCEED; length++; 
	quantity = Monitor_Quantity(source,total_length);  //��ü�ض��� ����
	for(i=0;i<quantity;i++)
	{
		*(target+length) = *(source+temp_offset+2); length++;  //��ض������
		*(target+length) = *(source+temp_offset+3); length++;  //��ض����Ÿ�λ��ַ
		*(target+length) = *(source+temp_offset+4); length++;  //��ض����ŵ�λ��ַ	
		p=Head_SearchAddr((source+temp_offset+3));  //������ݵ��׵�ַ
		for(m=0;m<(*(source+temp_offset+2)-immobilization_3);m++)
		{  //�۳�3�������������ݳ���
			*(p+m)=*(source+temp_offset+2+immobilization_3+m);  //����д��
			*(target+length) = *(p+m); length++;  
		}
		temp_offset+=*(source+temp_offset+immobilization_2); //temp_offset->ƫ�Ƶ�λ��(��ض������)
	}	 
  if(ON==Gprs_flag.bits.Start_Program)
  {
    if((Master_Inf.State.Massage>0)
      ||(ON == Set_Param.Lock_Chair)
      ||(0x01 != Collect_Data.Chair_ServiceCondition)
      ||(0!=Alarm_State.Chair_Alarm)
      ||(0!=Alarm_State.Gprs_Alarm))
    { //��Ħ��ʱ�䲻δ0���豸������gprs�޹��ϡ��豸�޹���
        *(target+length-1) =0xff;
        Gprs_flag.bits.Start_Program=OFF;
    }
  }
	temp_crc = crc16(target,length); //װ��CRC
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	Gprs_flag.bits.ReceiveSet=OFF;	
	return TRUE;
}

/*
** ===================================================================
**     Method      : 	Send_FirstOpen 
**     Description :	�����ϱ�-�����豸���к�   ��CRC�Լ�֡β
**     Parameters  : 	*target ��Ŀ������ָ�룻  
**     Returns     : 	Ŀ�����ݳ���
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
**     Description :	������-�����豸���к�
**     Parameters  : 	*target ��Ŀ������ָ�룻  
**     Returns     : 	Ŀ�����ݳ���
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
**     Description :	�ϱ��豸�澯
**     Parameters  : 	*target ��Ŀ������ָ�룻 
**     Returns     : 	Ŀ�����ݳ���
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
**     Description :	�ϱ�GPRS�澯
**     Parameters  : 	*target ��Ŀ������ָ�룻 
**     Returns     : 	Ŀ�����ݳ���
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
**     Description :	�ü�ض����ŵĵ�һ����ַѰ�ң���ض����Ӧ�����ݴ洢��ַ
**     Parameters  : 	*monitor_head  ��ض����ŵ��׵�ַ  
**     Returns     : 	���෵�ؼ�ض����Ӧ�Ĵ洢���ݵ�Ԫ��ַ
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
**     Description :	����ָ���ַ
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
**     Description :	�������ݣ������ڽ���ת��
**     Parameters  : 	*monitor_head  ��ض����ŵ��׵�ַ
**     Returns     : 	NULL : ��ʾ����
**									���෵�ؼ�ض����Ӧ�Ĵ洢���ݵ�Ԫ��ַ
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

