/*
** ===================================================================
**     File Name   :	GPRS_Protocol.c
**     Author			 :	Deric-Easepal
**     Versions  	 : 	v0.2  2016-10-29
**		 Port				 :	
**     Modification: 
**			2016-10-29 ：初稿之前写好，整合与有方GPRS连接内容
**						note ：告警模块需重写
										 启动程序函数也要重写
										 去掉串口中断接收函数
										 接收的数据使用指针指向，发送的数据，使用数据复制
** ===================================================================
*/

#include "GPRS_Protocol.h"
/********************调用外部参数集合************************/
/*
			NeoWayExternalPar            状态信息
											只要将	NeoWayExternalPar.HardwareRebootState
															NeoWayExternalPar.SofewareRebootState 置位，就会使用对应的方式重启模块
															
			g_aTcpRecDate          接收到数据的数组
			g_uRecTcpDateNum			 接收到的数据长度 
			
			g_aTcpSendDate				 发送数据的数组
			g_uSendTcpDateNum			 发送数据的数据长度
			
			extern uint8 Gprs_Send_Date(void);
*/

/********************外部调用参数集合************************/
/*
		void GprsRec_Date(uint8* Date,uint8 num); 接收数据
		void Gprs_1SEvent(void)										1S
		uint8  Gprs_ReceiveEvent(void)			50ms rtc时间函数中调用
*/

extern NeoWayExternalPar_ NeoWayExternalPar;
//接收到TCP数据
extern uint8 g_aTcpRecDate[TCP_REC_DATE_MAX];
extern uint8  g_uRecTcpDateNum;
//发送TCP数据
extern uint8 g_aTcpSendDate[NEOWAY_SEND_MAX];
extern uint8  g_uSendTcpDateNum ;
extern uint8 Gprs_Send_Date(void);



/********************内部参数集合************************/

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
**     Description :	GPRS协议层接收数据，通过指针赋值
**     Parameters  : 	Date 接收到的数据指针，包括帧头帧尾
**								 ：	num	 接收到是数据长度，包括帧头帧尾
**     Returns     : 	Nothing
** ===================================================================
*/
void GprsRec_Date(uint8* Date,uint8 num)
{	
	if(0x5E==*(Date)&&(0x7E==*(Date+num-1)))
	{
		//扣除帧头、帧尾
		ReceiveOriginal = Date+1;   
		Original_length = num-2;
		Gprs_flag.bits.Deal_data=ON;
		NeoWayExternalPar.RecTcpDateState=OFF;
	}
}

/*
** ===================================================================
**     Method      : 	GprsSend_Date 
**     Description :	GPRS协议层发送数据
**     Parameters  : 	Date 发送的数据指针，不包括帧头帧尾
**								 ：	num	 发送的数据长度，不包括帧头帧尾
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
**     Description :	外部调用1S事件
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
void Gprs_1SEvent(void)
{	
	if(1==NeoWayExternalPar.NetWorkConnetState)
	{
		Gprs_FirstOpenEvent();
		Gprs_HeartbeatEvent();
		Gprs_AlarmEvent();
	}
}

/*
** ===================================================================
**     Method      : 	Gprs_FirstOpenEvent 
**     Description :	开机上报，关闭心跳包，每10S上报，直到成功
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
		if(time>=2)
		{
			Send_FirstOpen(SendOriginal);
			time=0;
		}
		Gprs_flag.bits.HeartbeatTime=0;		
	}
}

/*
** ===================================================================
**     Method      : 	Gprs_AlarmEvent 
**     Description :	故障上报事件，告警触发后，每隔1分钟上报，连续上报三分钟。
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/

static void Gprs_AlarmEvent(void)
{
	static uint8  flag = OFF;
	if(0==Master_Inf.Alarm_Num)
	{
		flag=OFF;						//	非告警状态，清除标志、状态
		Gprs_flag.bits.AlarmFlag=OFF;
		Gprs_flag.bits.AlarmTime=0;
	}
	if((0!=Master_Inf.Alarm_Num)&&(OFF==Gprs_flag.bits.AlarmFlag)&&(OFF==flag))
	{
		flag=ON; //仅进一次
		Set_Param.Lock_Chair=0x01;//设备上锁
	 	Gprs_flag.bits.ReportAlarm=ON; //开启循环告警标志位
		Alarm_State.Chair_Alarm=Master_Inf.Alarm_Num; //写入告警信息
		Send_Alarm(SendOriginal);
		Gprs_flag.bits.AlarmFlag=ON;
	}
	if(ON==Gprs_flag.bits.AlarmFlag)
	{
		Gprs_flag.bits.AlarmTime++;
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
		Send_Alarm(SendOriginal);
	}
}

/*
** ===================================================================
**     Method      : 	Gprs_HeartbeatEvent 
**     Description :	心跳包事件，没有数据交互60S后，上送心跳包。
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
		//Gprs_flag.bits.HeartbeatFaultNum++;		
		Gprs_flag.bits.HeartbeatTime=0;		
	}
	if(Gprs_flag.bits.HeartbeatFaultNum>=3)//错误累积
	{
		flag = 1;
		Gprs_flag.bits.HeartbeatTime = 0;
		Gprs_flag.bits.HeartbeatFaultNum=0;
	}
	if((1==flag)&&(Gprs_flag.bits.HeartbeatTime>=3600)) //一个小时
	{
		flag=0;
		Gprs_flag.bits.HeartbeatTime=0;	
	}
}

/*
** ===================================================================
**     Method      : 	Gprs_StartCodeEvent 
**     Description :	开启启动程序选择。
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
//需重写
uint8 Gprs_StartCodeEvent(void)
{	
	volatile 	static uint8 Startflag = 0;
	static uint8 flag=0;
	
	if((1==flag)||(0!=Master_Inf.RunningTime))  //不在运行都为未使用
	{
		flag=1;
		if((0==Master_Inf.RunningTime)&&(ON==Startflag))
		{
			flag = 0;
			Collect_Data.Chair_ServiceCondition=0x01;
			Startflag=OFF;
			if(ON==Master_Inf.State.SysPower)
			{
				SendMaster_KeyValue(1);
				SendMaster_Date();
			}		
		}
	}
	
	if((ON == Gprs_flag.bits.Start_Program)&&(0x01==Collect_Data.Chair_ServiceCondition)
		&&(0x00==Set_Param.Lock_Chair))
	{
		Gprs_flag.bits.Start_Program=OFF;	
		Collect_Data.Chair_ServiceCondition=0x02;	
		Startflag = ON;	
		switch(Set_Param.Start_Program)
		{
		case  0x03 :SendMaster_KeyValue(5);
								SendMaster_Date();break;
		case  0x04 :SendMaster_KeyValue(5);
								SendMaster_Date();break;
		case  0x05 :SendMaster_KeyValue(5);
								SendMaster_Date();break;
		case  0x06 :SendMaster_KeyValue(5);
								SendMaster_Date();break;
		case  0x07 :SendMaster_KeyValue(5);
								SendMaster_Date();break;			
		default    :break;
		}
	
	}
	if((ON==Gprs_flag.bits.Stop_Chair))
	{	

		if(0x01==Set_Param.Stop_Chair)
		{
			SendMaster_KeyValue(1);
			SendMaster_Date();
			Gprs_flag.bits.Stop_Chair=OFF;
			Collect_Data.Chair_ServiceCondition=0x01;
		}
	}

	return 1;
}

/*
** ===================================================================
**     Method      : 	Gprs_ReceiveEvent 
**     Description :	接收到串口数据，进行处理。
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
uint8  Gprs_ReceiveEvent(void)
{
	volatile uint16 count_crc=0;
	volatile uint8  length=0;
	uint16 temp_crc = 0;
	volatile static uint16 ErrorDate=0;
	uint8  source[FRAME_MAX_LENGTH]={0};	
	if(1==Gprs_flag.bits.Deal_data)			//数据接收完毕，开始处理数据
	{  
	length=ServerToGprs_ChangeMean( ReceiveOriginal, source, Original_length); 	
	count_crc=crc16(source, length-2);	
	if(count_crc==(source[length-2]<<8)		//要注意length在数组中的位置关系!
		+source[length-1])
	 {		//crc校验通过									
			if(	Check_Size(source,(length-2)))	//长度校验通过		
			{							
				if(TRUE==Read_AckCmd(source,SendOriginal,length))//length 为除了帧头、帧尾的长度
				{
					Gprs_flag.bits.Deal_data = 0;
					Original_length = 0;
					Gprs_flag.bits.HeartbeatTime=0;
					return TRUE;
				}
			}
	}
	ErrorDate++;
	*(source+1)=ACK_CRC_FAULT;
	temp_crc = crc16(source,length-2);//-2为扣除原CRC的长度
	*(source+length-2) = temp_crc>>8;
	*(source+length-1) = temp_crc;		
	Group_SendData(source,length);	
	Gprs_flag.bits.Deal_data = 0;
	Original_length = 0;
	Gprs_flag.bits.HeartbeatTime=0;
	}
	return FAULT;
}

/*
** ===================================================================
**     Method      : 	Read_AckCmd 
**     Description :	处理应答的接收帧
**     Parameters  : 	*source －源数据指针；       
*  										*target －目标数据指针； 
* 										*length －源数据长度； 
**     Returns     : 	0: 表示错误  1表示正确
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
							Gprs_flag.bits.ActiveReportFlag = OFF;
							break;//开机上报成功
						}else{return FAULT;}        
        case ORDER_HEARTBEAT:  
					if((ON == Gprs_flag.bits.ReportHeartbeat)||(ACK_SUCCEED==*(source+1)))
						{
							Gprs_flag.bits.ReportHeartbeat = OFF;
							Gprs_flag.bits.ActiveReportFlag = OFF;
							break; //心跳上报成功
						}else{return FAULT;}    
        case ORDER_ALARM:  
					if((ON == Gprs_flag.bits.ReportAlarm)||(ACK_SUCCEED==*(source+1)))
						{
							Gprs_flag.bits.ReportAlarm = OFF;
							Gprs_flag.bits.ActiveReportFlag = OFF;
							Gprs_flag.bits.AlarmFlag = OFF;
							Gprs_flag.bits.AlarmTime = 0;
							break;//告警上报成功
						}else{return FAULT;}          
		case ORDER_INQUIRE:
						if(ACK_ORDER_MUST_REPLY == *(source+1))
						{
							Send_InquireInfo(source,target,length);
							break; //将数据存储在target中了                                //查询
						}else{return FAULT;}
		case ORDER_SET:
						if(ACK_ORDER_MUST_REPLY == *(source+1))
						{
							Send_SetInfo(source,target,length);
							break; 			//将数据存储在target中了 
						}else{return FAULT;}		
		default: return FAULT ;
	}
    return TRUE;    
}

/*
** ===================================================================
**     Method      : 	crc16 
**     Description :	CCITT推荐的16位的x16+x12+x5+1（0x11021）程序
**     Parameters  : 	*ptr ：需要校验的数组首址
**       								len：要校验的数据个数
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
**     Description :	检查最大长度以及监控对象长度所组成的总长度
**     Parameters  : 	*source －源数据指针； 不包含帧头     
**		  								length  不包含帧头、帧尾、CRC
**     Returns     : 	 0: 表示错误 
**										 1: 正确
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
		if(i>length)return FAULT; //校验数据内容长度
	}
	return TRUE;
}

/*
** ===================================================================
**     Method      : 	Init_Gprs_Device 
**     Description :	初始化GPRS协议层内容
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
void Init_Gprs_Device(void)
{
	uint8 i = 0;
	uint8 temp_id[SERIAL_ID_LENGTH] = SERIAL_ID;
	uint8 temp_ip[4]  =SERVER_IP;
	uint8 temp_port[2] =SERVER_PORT;        
	for(i=0;i<SERIAL_ID_LENGTH;i++)
	{
		Device_Info.Serial_Id[i] = temp_id[i];   //设备ID
	}    
	for(i=0;i<SERVER_IP_LENGTH;i++)
	{
		Web_Param.Server_Ip[i] = temp_ip[i];   //服务器IP
	}
	for(i=0;i<SERVER_PORT_LENGTH;i++)
	{
		Web_Param.Server_Port[i] = temp_port[i];//服务器端口号
	}
	Alarm_State.Chair_Alarm = 0x00;
	Set_Param.Lock_Chair=0x00;    //0x00表示解锁
	Collect_Data.Chair_ServiceCondition = 0x01; 	
	Gprs_flag.flag = 0x0000;
	//开机上报
	Gprs_flag.bits.ReportFirstOpen = ON; 
	//Send_FirstOpen(SendOriginal);
}

/*
** ===================================================================
**     Method      : 	ServerToGprs_ChangeMean 
**     Description :	接收翻译转义字符
**     Parameters  : 	*source －源数据指针；已去除帧头、帧尾。
**       							*target －目的数据指针；
**       		 					*length －源数据数据长度；
**     Returns     : 	转换后的目的数据长度,如果为0，说明长度超过最大长度
** 										转义规则：用0x1E，0x5D来代替0x5E；用0x1E，0x7D来代替0x7E；用0x1E，0x00来代替0x1E
**										帧长度应该限制在最大长度内。
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
**     Description :	发送转换转义字符
**     Parameters  : 	*source －源数据指针；
**        						*target －目的数据指针；不包含帧头帧尾
**        						*length －源数据数据长度；
**     Returns     : 	 转换后的目的数据长度,如果为1，说明长度超过最大长度
**										 转义规则：用0x1E，0x5D来代替0x5E；用0x1E，0x7D来代替0x7E；用0x1E，0x00来代替0x1E
**          					 帧长度应该限制在最大长度内。
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
**     Description :	统计监控对象的个数
**     Parameters  : 	*source －源数据指针；
**        						*total_length －源数据数据长度；
**     Returns     : 	监控对象个数
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
	//temp=total_length-offset-2;  //去除 命令标志 和应答标志 //扣除CRC
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
**     Description :	发送请求查询的数据
**     Parameters  : 	*source －源数据指针；
**        						*target －目的数据指针；不包含帧头、帧尾
**        						*length －源数据数据长度；
**     Returns     : 	目标数据指针的长度
** ===================================================================
*/
static uint8 Send_InquireInfo(const uint8 * source,uint8 *target,const uint8 total_length)
{
	 volatile uint8 length = 0;
	 volatile static  uint16 Cnt=0;
	 uint8 i = 0,m = 0;
	 uint8 quantity = 0;
	 uint8 immobilization = 2; //固定偏移2个单位。命令标志与应答标志
	 uint8 immobilization_3 = 3; //固定偏移3个单位。监控长度单元与地址单元
	 uint8 temp_offset = 0; 
	 uint8 *p; 	 
	 uint16 temp_crc = 0;
	 *(target+length) = ORDER_INQUIRE; length++;
	 *(target+length) = ACK_SUCCEED; length++; 
	 quantity = Monitor_Quantity(source,total_length);  //获得监控对象 数量
	 for(i=0;i<quantity;i++)
	 {	 	
	 	*(target+length) = *(source+temp_offset+2); length++;  //监控对象个数
		*(target+length) = *(source+temp_offset+3); length++;  //监控对象标号高位地址
		*(target+length) = *(source+temp_offset+4); length++;  //监控对象标号低位地址	
		p=Head_SearchAddr((source+temp_offset+3));  //获得数据的首地址	
		  for(m=0;m<(*(source+temp_offset+2)-immobilization_3);m++)
		  {   //扣除3才是真正的数据长度
			*(target+length) = *(p+m); length++;    //数据读出
		  }
			temp_offset+=*(source+temp_offset+immobilization); //temp_offset->偏移的位置(监控对象个数)
	 }
	temp_crc = crc16(target,length); //装载CRC
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	Cnt++;
	return TRUE;
}

/*
** ===================================================================
**     Method      : 	Send_SetInfo 
**     Description :	发送请求设置的数据，并且写入数据
**     Parameters  : 	*source －源数据指针；
**        						*target －目的数据指针；不包含帧头、帧尾
**       							*length －源数据数据长度�
**     Returns     : 	目标数据指针的长度,
** ===================================================================
*/
static uint8 Send_SetInfo(const uint8 * source,uint8 *target,const uint8 total_length)
{
	uint8 length = 0;
	uint8 i = 0,m = 0;
	uint8 quantity = 0;
	uint8 immobilization_2 = 2; //固定偏移2个单位。命令标志与应答标志
	uint8 immobilization_3 = 3; //固定偏移3个单位。监控长度单元与地址单元
	uint8 temp_offset = 0; 
	uint8 *p; 	 
	uint16 temp_crc = 0;
	Gprs_flag.bits.ReceiveSet = ON;
	*(target+length) = ORDER_SET; length++;
	*(target+length) = ACK_SUCCEED; length++; 
	quantity = Monitor_Quantity(source,total_length);  //获得监控对象 数量
	for(i=0;i<quantity;i++)
	{
		*(target+length) = *(source+temp_offset+2); length++;  //监控对象个数
		*(target+length) = *(source+temp_offset+3); length++;  //监控对象标号高位地址
		*(target+length) = *(source+temp_offset+4); length++;  //监控对象标号低位地址	
		p=Head_SearchAddr((source+temp_offset+3));  //获得数据的首地址
		for(m=0;m<(*(source+temp_offset+2)-immobilization_3);m++)
		{  //扣除3才是真正的数据长度
			*(p+m)=*(source+temp_offset+2+immobilization_3+m);  //数据写入
			*(target+length) = *(p+m); length++;  
		}
		temp_offset+=*(source+temp_offset+immobilization_2); //temp_offset->偏移的位置(监控对象个数)
	}	 
	temp_crc = crc16(target,length); //装载CRC
	*(target+length) = temp_crc>>8;length++;
	*(target+length) = temp_crc;length++;
	Group_SendData(target,length);
	Gprs_flag.bits.ReceiveSet=OFF;	
	return TRUE;
}

/*
** ===================================================================
**     Method      : 	Send_FirstOpen 
**     Description :	开机上报-传输设备序列号   无CRC以及帧尾
**     Parameters  : 	*target －目标数据指针；  
**     Returns     : 	目标数据长度
** ===================================================================
*/
static uint8 Send_FirstOpen(uint8 *target)
{
	uint8 i = 0;
	uint8 length = 0;
	uint16 temp_crc = 0;
	Gprs_flag.bits.ActiveReportFlag = ON;	
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
**     Description :	心跳包-传输设备序列号
**     Parameters  : 	*target －目标数据指针；  
**     Returns     : 	目标数据长度
** ===================================================================
*/
static uint8 Send_Heartbeat(uint8 *target)
{
	uint8 i = 0;
	uint8 length = 0;
	uint16 temp_crc = 0;	
	Gprs_flag.bits.ActiveReportFlag = ON;
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
**     Description :	上报设备告警
**     Parameters  : 	*target －目标数据指针； 
**     Returns     : 	目标数据长度
** ===================================================================
*/
static uint8 Send_Alarm(uint8 *target)
{    
	uint8 length = 0;
	uint16 temp_crc = 0;
	Gprs_flag.bits.ActiveReportFlag = ON;
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
**     Method      : 	Head_SearchAddr 
**     Description :	用监控对象标号的第一个地址寻找，监控对象对应的数据存储地址
**     Parameters  : 	*monitor_head  监控对象标号的首地址  
**     Returns     : 	其余返回监控对象对应的存储数据单元地址
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
**     Description :	返回指针地址
**     Parameters  : 	
**     Returns     : 	
** ===================================================================
*/

static uint8* Matching_DEV(const uint8 *dev)
{
	switch(*dev)
	{
		case Serial_Id 		: return (Device_Info.Serial_Id);
		default  	   		: return  NULL;
	}
}
static uint8* Matching_WEB(const uint8 *web)
{
	switch(*web)
	{
		case Server_Ip   	: return (Web_Param.Server_Ip);
		case Server_Port 	: return (Web_Param.Server_Port);
		default  	   		: return  NULL;
	}
}
static uint8* Matching_ALA(const uint8 *ala)
{
	switch(*ala)
	{
		case Chair_Alarm 	: return (&Alarm_State.Chair_Alarm);
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
		default: return  NULL;
	}
}
/*
** ===================================================================
**     Method      : 	Group_SendData 
**     Description :	发送数据，函数内进行转意
**     Parameters  : 	*monitor_head  监控对象标号的首地址
**     Returns     : 	NULL : 表示错误
**									其余返回监控对象对应的存储数据单元地址
** ===================================================================
*/
static void Group_SendData(uint8 *source,uint8 length)
{
	static uint8 target[FRAME_MAX_LENGTH]={0};
	uint8 temp_length = 0;
	temp_length=GprsToServer_ChangeMean(source,target,length);
  GprsSend_Date(target,temp_length); 
}
