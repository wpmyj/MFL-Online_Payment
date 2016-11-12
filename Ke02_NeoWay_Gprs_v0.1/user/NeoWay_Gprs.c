/*
** ===================================================================
**     File Name   :	NeoWay_Gprs.c
**     Author			 :	Deric-Easepal
**     Versions  	 : 	v0.1  2016-10-25
**		 Port				 :	NeoWay Use:	Uart1
**										Debug Use	:	Uart0
**										EMERGOFF  :
**										ON/OFF		:
**     Modification: 
**			2016-10-25 :	����
**						Note ��	�����жϵȼ�Ҫ�ȶ�ʱ���ߣ���Ȼ�ᵼ����������
**			2016-10-29 ��	����GPRSЭ������
**			2016-11-10 �� ��ʼ�޸�ʱ������,��ͣ�İ�����δʵ��
** ===================================================================
*/

#include "NeoWay_Gprs.h"
/********************������������ͼ************************/
/*
Rec:	1				MODEM:STARTUP							�����ɹ�
			2				+PBREADY      						ʶ��SIM��
Send: 3				"AT+CGSN\r"								��ȡCGSN�ţ���Ʒ���к�
			4				"AT+CIMI\r"								��ȡCIMI�ţ������ƶ��û�ʶ����
			5				"AT+CREG?\r"							��ѯ����ע�����
			6				"AT+CSQ\r"								��ȡ�ź�ǿ�ȣ��ŵ�λ������
			7				"AT+XISP=0\r"							�����ڲ�Э��ջ
			8				"AT+CGDCONT=1,\"IP\",\"CMNET\"\r"
																				����PDP��ʽ
			9				"AT+XGAUTH=1,1,\"gsm\",\"1234\"\r"
																				����PDP��֤
			10			"AT+XIIC=1\r"							����PPP����
			11			"AT+TCPSETUP=0,121.43.115.207,6001\r"
																				����TCP����
			12			"AT+TCPSEND"							��tcp��������
*/

/********************GPRSģ�鱣������***********************/
/*
			1 		 ������ʼ�����̣�δ�յ���ȷ���ɹ�Ӧ��ʱ������ģ��
			2			 �ӿ�����ʼ����ʱ���ڹ涨ʱ����δ��ɳ�ʼ����ʱ������ģ��
			3			 ÿ��ȥʶ������״̬��־λ����PPP��TCP��������ʧȥ����ʱ������ģ��
			4			 GPRSЭ���У���125Sδ�յ���̨��Ӧ������
			��ʱ������ģ��
*/

/********************�����ļ����ò�������************************/
/*
void NeoWay_UartRec(uint8 Date)    uartapp.c
void NeoWay_Rtc1ms(void)           rtcapp.c
void NeoWay_Rtc1s(void);					 main.c
uint8 Gprs_Send_Date(void)				 �������ݺ��������ô˺���ʱ�������ȸ�ֵg_aTcpSendDate�ڵ����ݣ������ݳ���g_uSendTcpDateNum
			
			NeoWayExternalPar      ״̬��Ϣ
			g_aTcpRecDate          ���յ����ݵ�����
			g_uRecTcpDateNum			 ���յ������ݳ��� 
			
			g_aTcpSendDate				 �������ݵ�����
			g_uSendTcpDateNum			 �������ݵ����ݳ���
*/

/********************���ò�������************************/
/*
		void GprsRec_Date(uint8* Date,uint8 num);
*/

extern void GprsRec_Date(uint8* Date,uint8 num); 
/********************ȫ�ֱ���************************/
//����GPRS��������
uint8 g_aNeoWayRecBuff[NEOWAY_REC_MAX] = {0};
uint8 g_aNeoWayRecNum = 0;
uint8 g_aNeoWayRec[NEOWAY_REC_MAX]= {0};

//���յ�TCP����
uint8 g_aTcpRecDateBuff[TCP_REC_DATE_MAX]={0};
uint8  g_uRecTcpDateNum = 0;
//����TCP����
uint8 g_aTcpSendDate[NEOWAY_SEND_MAX]= {0};
uint8  g_uSendTcpDateNum = 0;

//�з�ģ������еı�������
NeoWayRec_ NeoWayRec;
NeoWayInf_ NeoWayInf;
NeoWaySysPar_ NeoWaySysPar;
NeoWayExternalPar_ NeoWayExternalPar;

uint16 Connet_ErrorCnt=0;
uint8 Uart1_ReceiveBuff[UART1_BUFFLENGTH]={0};
/********************����************************/


void NeoWayBoard_Init(void)
{
	/********************UART************************/
	UART_ConfigType sUARTConfig;
	sUARTConfig.u32SysClkHz = BUS_CLK_HZ;
	sUARTConfig.u32Baudrate = 115200;
	UART_Init(UART1,&sUARTConfig);
	UART_SetCallback(UART_HandleInt);
	UART_ReceiveInt(UART1, Uart1_ReceiveBuff, UART1_BUFFLENGTH);
	NVIC_EnableIRQ(UART1_IRQn); 
	NVIC_SetPriority(UART1_IRQn,2);

	/********************UART************************/
	GPIO_PinInit(PORT_EMERGOFF, GPIO_PinOutput); 
	GPIO_PinInit(PORT_ON_OFF, GPIO_PinOutput);	
	GPIO_PinClear(PORT_EMERGOFF);
	GPIO_PinClear(PORT_ON_OFF);		
}

// 115200/8=14400 14400/2=7200��  һ��char ֮��ļ��Ϊ7��us 
//���Կ����趨֮֡�������20ms�Ǳ��յ�

/*
** ===================================================================
**     Method      : NeoWay_UartRec
**     Description : ���ڽ��պ����������ڴ��ڽ����ж���
**     Parameters  : Date  ���յ�������
**     Returns     : Nothing
** ===================================================================
*/
void NeoWay_UartRec(uint8 Date)
{
	//���յ������ݣ���ռ��ʱ��
	NeoWayRec.Time = 0;	
	NeoWayRec.Receiving = ON;
	g_aNeoWayRecBuff[NeoWayRec.Num] = Date;
	NeoWayRec.Num++;	
}
/*
** ===================================================================
**     Method      : 	NeoWay_Rec1ms 
**     Description :	�з�������ʱ�䷢�������û������Լ���������֡
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
static void NeoWay_Rec1ms(void)
{
	uint16 i=0;
	if(ON==NeoWayRec.Receiving)
	{
		NeoWayRec.Time++;
		//�������
		if(NeoWayRec.Time>=NEOWAY_REC_INTERVAL_TIME)
		{			
			memset((uint8 *)&g_aNeoWayRec,(uint8)0,NEOWAY_REC_MAX);
			for(i=0;i<NeoWayRec.Num;i++)g_aNeoWayRec[i]=g_aNeoWayRecBuff[i];
#ifdef DEBUG_RANK_BRONZE	
printf("Receive Date:<-----");
printf("%s",g_aNeoWayRec);
printf("\r\n");		
#endif		
			g_aNeoWayRecNum = NeoWayRec.Num;
			NeoWayRec.Time = 0;
			NeoWayRec.Num = 0;
			NeoWayRec.Receiving = OFF;		
		}
	}

}
/*
** ===================================================================
**     Method      : 	NeoWay_Rec1s 
**     Description :	�ⲿ����1�������
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
void NeoWay_Rtc1s(void)
{
	Protect_Connet();
}
/*
** ===================================================================
**     Method      : 	NeoWay_Rtc1ms 
**     Description :	�ⲿ����1���������
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
void NeoWay_Rtc1ms(void)
{
	NeoWay_Rec1ms();	
}

/*
** ===================================================================
**     Method      : 	NeoWay_SendString
**     Description :	�����ַ�������
**     Parameters  : 	*string �ַ���ָ��
**     Returns     : 	���
** ===================================================================
*/
static uint8  NeoWay_SendString(uint8* string)
{
	uint16 i=0;
	while(*(string+i)!='\0')
	{
		i++;
		if(i>=NEOWAY_SEND_MAX)return ERROR;
	}	
	//+1����Ϊ\r
#ifdef DEBUG_RANK_BRONZE
printf("Send Date is:----->");
UART_SendWait(UART0,string,i+1);	
printf("\r\n");
#endif 
	UART_SendWait(NEOWAY_UART,string,i+1);	
	return SUCCEED;
}

/*
** ===================================================================
**     Method      : 	NeoWay_SendDate
**     Description :	���ڸ���̨��������
**     Parameters  : 	*Date ����ָ��
**								 :	num	���ݳ���
**     Returns     : 	���
** ===================================================================
*/
static uint8  NeoWay_SendDate(uint8* Date,uint8 num)
{
	volatile uint8 i=0;
	*(Date+num)='\r';	
	UART_SendInt(NEOWAY_UART,Date,num+1);
//	UART_SendWait(NEOWAY_UART,Date,num);
//	UART_PutChar(NEOWAY_UART, '\r');
#ifdef DEBUG_RANK_BRONZE
printf("Send Server Date is:----->>");
	for(i=0;i<num;i++)
	{
		UART_PutChar(UART0, '0');
		UART_PutChar(UART0, 'x');
		UART_PutChar(UART0, (*(Date+i)>>4)+'0');
		UART_PutChar(UART0, (((*(Date+i))&0x0f)+'0'));
		UART_PutChar(UART0, ' ');
	}
printf("\r\n");
#endif 
	return SUCCEED;
}
/*
** ===================================================================
**     Method      : 1ms��ʱ����
**     Description : ��׼���պ���
**     Parameters  : num ��ʱʱ��
**     Returns     : Nothing
** ===================================================================
*/
static void Delay_ms(uint16 num)
{
	uint16 m,k=0;
	for(m=0;m<num;m++)
	for(k=0;k<4000;k++);
}

/*
** ===================================================================
**     Method      : 	NeoWay_Init
**     Description :	ģ���ʼ������
**     Parameters  : 	None
**     Returns     : 	���
** ===================================================================
*/
uint8 NeoWay_Init(void)
{
	Delay_ms(NEOWAY_WAIT_TIME);
	Get_Cgsn();
	Get_Cimi();
	NetWork_Login();
	Signal_Strength();
	SetProtocol_Stack();
	SetPDP_Format();
	User_Authentication();
	BuildPPP_Connet();
	BuildTCP_Connet();
	return SUCCEED;
}

/*
** ===================================================================
**     Method      : 	Get_Cgsn
**     Description :	��ȡģ���Ʒ���кţ�Ҳ����IMEI��
**     Parameters  : 	None
**     Returns     : 	���
** ===================================================================
*/
static uint8 Get_Cgsn(void)
{
	uint8 i=0;
	int8* string;
	int8* TempPrintf="012345678912345";
	NeoWay_SendString((uint8*)"AT+CGSN\r");
	Delay_ms(NEOWAY_WAIT_TIME);
	if(strstr((char *)g_aNeoWayRec,"ERROR")>0)
	{//��ȡʧ��
#ifdef DEBUG_RANK_DIAMOND	
printf("Get CGSN ERROR\r");
#endif 
			return ERROR;
	}else
	{
		string=strrchr((char *)g_aNeoWayRec,'N');
		if(NULL == string)
		{
			return ERROR;
		}
		for(i=0;i<15;i++)
		{
			*(NeoWayInf.Cgsn+i)=*(string+4+i);
/*********************protect Num Rang******************************/			
			if((*(NeoWayInf.Cgsn+i)>'9')&&(*(NeoWayInf.Cgsn+i)<'0'))
			{
#ifdef DEBUG_RANK_DIAMOND	
printf("Get CGSN ERROR,Nunber is Over Range \r");
#endif 
				return ERROR;			
			}
		}
/*********************End protect***********************************/			
#ifdef DEBUG_RANK_PLATINUM	
TempPrintf=string+3;
printf("Get CGSN SUCCEED,Nunber is %s\r",TempPrintf);
#endif 
		return SUCCEED;
	}	
}

/*
** ===================================================================
**     Method      : 	Get_Cimi
**     Description :	��ȡ�����ƶ��û�ʶ����IMSI
**     Parameters  : 	None
**     Returns     : 	���
** ===================================================================
*/
static uint8 Get_Cimi(void)
{
	uint8 i=0;
	int8* string;
	int8* TempPrintf="012345678912345";
	NeoWay_SendString((uint8*)"AT+CIMI\r");
	Delay_ms(NEOWAY_WAIT_TIME);
	if(strstr((char *)g_aNeoWayRec,"ERROR")>0)
	{//��ȡʧ��
#ifdef DEBUG_RANK_DIAMOND	
printf("Get CIMI ERROR\r");
#endif 
			return ERROR;
	}else
	{
		string=strrchr((const char *)g_aNeoWayRec,'I');
		if(NULL == string)
		{
			return ERROR;
		}
		for(;i<15;i++)
		{
			*(NeoWayInf.Cgsn+i)=*(string+4+i);
/*********************protect Num Rang******************************/			
			if((*(NeoWayInf.Cgsn+i)>'9')&&(*(NeoWayInf.Cgsn+i)<'0'))
			{
#ifdef DEBUG_RANK_DIAMOND	
printf("Get Cimi ERROR,Nunber is Over Range \r");
#endif 
				return ERROR;			
			}
		}
/*********************End protect***********************************/			
#ifdef DEBUG_RANK_PLATINUM	
TempPrintf=string+3;
printf("Get CIMI SUCCEED,Nunber is %s\r",TempPrintf);
#endif 
		return SUCCEED;
	}	
}
/*
** ===================================================================
**     Method      : 	NetWork_Login
**     Description :	����ע�ᣬ���Ҳ�ѯע����
**     Parameters  : 	None
**     Returns     : 	���
** ===================================================================
*/
static uint8 NetWork_Login(void)
{
	volatile uint8 Mode=0;
	uint8 Stat=0;
	int8* string;
	NeoWay_SendString((uint8*)"AT+CREG?\r");
	Delay_ms(NEOWAY_WAIT_TIME);
	if(strstr((char *)g_aNeoWayRec,"ERROR")>0)
	{//��ȡʧ��		
#ifdef DEBUG_RANK_DIAMOND	
printf("Get NetWork Login ERROR\r");
#endif 
			return ERROR;
	}else
	{
		string=strrchr((const char *)g_aNeoWayRec,',');
		if(NULL == string)
		{
			return ERROR;
		}
		Mode=*(string-1);
		Stat=*(string+1);
#ifdef DEBUG_RANK_DIAMOND
		(('1' == Stat)||('5' == Stat))?printf("Net Work Login Succeed\r\n"):printf("Net Work Login ERROR\r\n");
#endif
	}
	return SUCCEED;
}
/*
** ===================================================================
**     Method      : 	Signal_Strength
**     Description :	��ѯ�ź�ǿ�ȣ��Լ��ŵ�λ������
**     Parameters  :	None
**     Returns     :	���
** ===================================================================
*/
static uint8 Signal_Strength(void)
{	
	volatile int16 Strength=0;
	volatile int16 ErrorRate=0;
	volatile int8* string;
	NeoWay_SendString((uint8*)"AT+CSQ\r");
	Delay_ms(NEOWAY_WAIT_TIME);
	if(strstr((char *)g_aNeoWayRec,"ERROR")>0)
	{//��ȡʧ��		
#ifdef DEBUG_RANK_DIAMOND	
printf("Get Signal Strength ERROR\r");
#endif 
			return ERROR;
	}else
	{
		string = strrchr((const char *)g_aNeoWayRec,',');
		if(NULL == string)
		{
			return ERROR;
		}
		Strength = (int16)(((*(string-2)-'0')*10)+((*(string-1)-'0')))*2-113;
		if((*(string+2)>='0')&&(*(string+2)<='9'))
		{
			ErrorRate =(*(string+1)-'0')*10+(*(string+2)-'0');
		}else
		{
			ErrorRate =(*(string+1)-'0');
		}
#ifdef DEBUG_RANK_PLATINUM	
printf("Signal Strength is %d\r\n",Strength);
printf("Signal ErrorRate is %d\r\n",ErrorRate);	
#endif 
	return SUCCEED;		
	}	
}
/*
** ===================================================================
**     Method      : 	SetProtocol_Stack
**     Description :	����Э��ջ������Ϊ�ڲ�Э��ջ
**     Parameters  :	None
**     Returns     : 	���
** ===================================================================
*/
static uint8 SetProtocol_Stack(void)
{
	NeoWay_SendString((uint8*)"AT+XISP=0\r");
	Delay_ms(NEOWAY_WAIT_TIME);
	NeoWay_SendString((uint8*)"AT+XISP?\r");
	Delay_ms(NEOWAY_WAIT_TIME);
	if(strstr((char *)g_aNeoWayRec,"+XISP:    0")>0)
	{
#ifdef DEBUG_RANK_PLATINUM		
printf("Set Internal Protocol Stack  Succeed \r\n");
#endif			
		return SUCCEED;
	}else
	{
		NeoWay_SendString((uint8*)"AT+XISP=0");
#ifdef DEBUG_RANK_DIAMOND		
printf("Set Internal Protocol Stack  ERROR \r\n");
#endif	
		return ERROR;
	}
}
/*
** ===================================================================
**     Method      : 	SetPDP_Format
**     Description :	����GPRS��PDP��ʽ
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
static void SetPDP_Format(void)
{
#ifdef CMNET
	NeoWay_SendString((uint8*)"AT+CGDCONT=1,\"IP\",\"CMNET\"\r");
#endif
#ifdef UNINET
	NeoWay_SendString((uint8*)"AT+CGDCONT=1,\"IP\",\"UNINET\"\r");
#endif
}
/*
** ===================================================================
**     Method      : 	User_Authentication
**     Description :	PDP��֤�����Ǳ��룬�����޷�
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
void User_Authentication(void)
{
	NeoWay_SendString((uint8*)"AT+XGAUTH=1,1,\"gsm\",\"1234\"\r");
	Delay_ms(NEOWAY_WAIT_TIME);
	NeoWay_SendString((uint8*)"AT+XGAUTH=?\r");
	Delay_ms(NEOWAY_WAIT_TIME);
	if(strstr((char *)g_aNeoWayRec,"+XGAUTH:")>0)
	{
#ifdef DEBUG_RANK_PLATINUM		
printf("Set User Authentication Succeed \r\n");
#endif					
	}else 
	{
#ifdef DEBUG_RANK_DIAMOND		
printf("Set User Authentication ERROR \r\n");
#endif	
	}	
}
/*
** ===================================================================
**     Method      : 	BuildPPP_Connet
**     Description :	����PPP����
**     Parameters  : 	None
**     Returns     : 	���
** ===================================================================
*/
static uint8 BuildPPP_Connet(void)
{
	int8* string;
	uint8 i=0,k=0,m=0;
	uint8 ErrorCnt=0;

	for(k=1;k<4;k++)
	{
#ifdef DEBUG_RANK_PLATINUM		
printf("Build PPP ing ~~~~~ Wait A Moment  \r\n");
#endif		
		NeoWay_SendString((uint8*)"AT+XIIC=1\r");
		for(m=0;m<k;m++)
		{//3��
			Delay_ms(3000);
		}		
		NeoWay_SendString((uint8*)"AT+XIIC?\r");
		Delay_ms(NEOWAY_WAIT_TIME);
		string=strrchr((const char *)g_aNeoWayRec,',');
		if(NULL == string)
		{
			return ERROR;
		}
		if(*(string-1)>='1')
		{
			for(i = 0;*(string+i) != '\r';i++)
			*(NeoWayInf.Ip+i) = *(string+1+i);
			i++;
			*(NeoWayInf.Ip+i)='\r';		
#ifdef DEBUG_RANK_PLATINUM		
printf("Build PPP SUCCEED,IP is %s \r\n",NeoWayInf.Ip);
#endif
			NeoWaySysPar.NetWork.ConnetPPPState=ON;
			return SUCCEED;
		}	
		ErrorCnt++;
		if(ErrorCnt>=4)
		{
#ifdef DEBUG_RANK_DIAMOND		
printf("Build PPP ERROR \r\n");
#endif		
		//ȱ����������
		return ERROR;
		}
	}
	return ERROR;
}
/*
** ===================================================================
**     Method      : 	BuildTCP_Connet
**     Description :	����TCP����
**     Parameters  : 	None
**     Returns     : 	���
** ===================================================================
*/
static uint8 BuildTCP_Connet(void)
{
	NeoWay_SendString((uint8*)SEND_TCP_IP);
	Delay_ms(NEOWAY_WAIT_TIME);
 if(strstr((char *)g_aNeoWayRec,"FAIL")>0)
	{
#ifdef DEBUG_RANK_DIAMOND		
printf("Build TCP FAIL \r\n");
#endif			
		return ERROR;
	}else if(strstr((char *)g_aNeoWayRec,":Error")>0)
	{
#ifdef DEBUG_RANK_DIAMOND		
printf("Build TCP ERROR \r");
#endif	
		return ERROR;
	}else	if(strstr((char *)g_aNeoWayRec,"OK")>0)
	{
#ifdef DEBUG_RANK_PLATINUM		
printf("Build TCP SUCCEED\r\n");
#endif	
		NeoWaySysPar.NetWork.ConnetTCPState=ON;
		Delay_ms(NEOWAY_WAIT_TIME*5);
		return SUCCEED;
	}
	return ERROR;
}

/*
** ===================================================================
**     Method      : 	itoa
**     Description :	�ú��������翽���ģ������ǽ�����ת�����ַ���
**     Parameters  : 	num		ת������
**										*str	�ַ���ָ��
**										radix  ���ٽ��Ƶ�
**     Returns     : Nothing
** ===================================================================
*/
static char* itoa(int num,char*str,int radix)
{
	char index[]="0123456789ABCDEF";
	unsigned unum;
	int i=0,j,k;
	char temp;
	if(radix==10&&num<0)
	{
		unum=(unsigned)-num;
		str[i++]='-';
	}else
	{
		unum=(unsigned)num;
	}
	do
	{
		str[i++]=index[unum%(unsigned)radix];
		unum/=radix;
	}while(unum);
	str[i]='\0';
	if(str[0]=='-')
	{
		k=1;
	}	else
	{	
		k=0;
	}
	for(j=k;j<=(i-1)/2;j++)
	{
		temp=str[j];
		str[j]=str[i-1+k-j];
		str[i-1+k-j]=temp;
	}
	return str;
}
/*
** ===================================================================
**     Method      : 	TcpSend_Date
**     Description :	TCP�������ݺ��������ô˺���ʱ�������ȸ�ֵg_aTcpSendDate�ڵ����ݣ������ݳ���g_uSendTcpDateNum
**     Parameters  : 	None
**     Returns     : 	Nothing
** ===================================================================
*/
uint8 TcpSend_Date(void)
{
	
	int8 Send[20]="AT+TCPSEND=0,";
	uint8 i=0;
	int8 SendDateNum[4] ;
	uint16 k=0;
    volatile static uint16 Cnt=0; 
	//ת����ֵΪ�ַ���	
	itoa(g_uSendTcpDateNum,SendDateNum,10);
	strcat(Send,SendDateNum);	
	 Cnt = 0;
	while(0 == Send[19-i])
	{
		i++;
	}
	Send[19-i+1]='\r';		
	NeoWay_SendString((uint8*)Send);
	for(k=0;k<5000;k++)
	{
	    Cnt++;
		Delay_ms(1);
		if((strstr((char *)g_aNeoWayRec,">")>0))
		{
			Delay_ms(NEOWAY_SEND_WAIT_TIME);
			NeoWay_SendDate(g_aTcpSendDate,g_uSendTcpDateNum);
      
			return SUCCEED;
		}
	}
	return ERROR;
}

/*
** ===================================================================
**     Method      : 	Gprs_Send_Date
**     Description :	�ú��������翽���ģ������ǽ�����ת�����ַ���
**     Parameters  : 	num		ת������
**										*str	�ַ���ָ��
**										radix  ���ٽ��Ƶ�
**     Returns     : Nothing
** ===================================================================
*/
uint8 Gprs_Send_Date(void)
{
	NeoWaySysPar.Init.GprsSendState=ON;
	return SUCCEED;
}

/*
** ===================================================================
**     Method      : 	ModuleBack_Code
**     Description :	ģ��Ļ��봦��
**     Parameters  : 	Nothing
**     Returns     : 	Nothing
** ===================================================================
*/
void ModuleBack_Code(void)
{
	if(strstr((char *)g_aNeoWayRec,"+TCPRECV:0")>0)
	{//���յ�TCP����
		NeoWayExternalPar.LoseTime=0;
		g_uRecTcpDateNum=ReceiveTCP_Date();
		GprsRec_Date(g_aTcpRecDateBuff,g_uRecTcpDateNum);
    memset(&g_aNeoWayRec,0,sizeof(g_aNeoWayRec));
	}else if(strstr((char *)g_aNeoWayRec,"MODEM:STARTUP")>0)
	{//�����ɹ�	
		NeoWaySysPar.Init.ModulePowerState = ON;
		NeoWaySysPar.Init.StartNum++;
    memset(&g_aNeoWayRec,0,g_aNeoWayRecNum);
	}else if(strstr((char *)g_aNeoWayRec,"+PBREADY")>0)
	{//��⵽SIM��
		NeoWaySysPar.Init.FindSimState = ON;
		NeoWaySysPar.Init.FindSimNum++;
		NeoWaySysPar.Init.StartInitState = ON;
    memset(&g_aNeoWayRec,0,g_uRecTcpDateNum);
	}else if(strstr((char *)g_aNeoWayRec,"SOCKETS:IPR")>0)
	{//��������
		NeoWayExternalPar.NetWorkConnetState = OFF;		
		NeoWaySysPar.NetWork.ConnetTCPState = OFF;
		NeoWaySysPar.NetWork.ConnetPPPState = OFF;
		NeoWayExternalPar.HardwareRebootState = ON;
    memset(&g_aNeoWayRec,0,g_uRecTcpDateNum);
	}else if(strstr((char *)g_aNeoWayRec,"Link Closed")>0)
	{//�������Ͽ��������쳣
		NeoWayExternalPar.NetWorkConnetState = OFF;
		NeoWaySysPar.NetWork.ConnetTCPState = OFF;
		NeoWaySysPar.NetWork.ConnetPPPState = OFF;
		NeoWayExternalPar.HardwareRebootState = ON;
    memset(&g_aNeoWayRec,0,g_uRecTcpDateNum);
	}else if(strstr((char *)g_aNeoWayRec,"+TCPSEND:Error")>0)
	{
		NeoWayExternalPar.NetWorkConnetState = OFF;
		NeoWaySysPar.NetWork.ConnetTCPState = OFF;
		NeoWaySysPar.NetWork.ConnetPPPState = OFF;
		NeoWayExternalPar.HardwareRebootState = ON;
    memset(&g_aNeoWayRec,0,g_uRecTcpDateNum);
	}
}	
/*
** ===================================================================
**     Method      : 	ReceiveTCP_Date
**     Description :	���յ����ݴ���
**     Parameters  : 	Nothing
**     Returns     : 	���ݵĳ���
** ===================================================================
*/
static uint16 ReceiveTCP_Date(void)
{
	int8* stringStart;	
	int8* stringEnd;
	volatile int16 DateLength=0,DateLengthTemp=0;
	uint8 ErrotNum=0,i=1;
	stringStart = strchr((const char *)g_aNeoWayRec,',');
	stringEnd = strrchr((const char *)g_aNeoWayRec,',');
	if((NULL == stringStart)||(NULL == stringEnd))
	{
		return ERROR;
	}
	for(i=1;(*(stringEnd-i)!=*(stringStart));i++)
	{//�ӵڶ������ſ�ʼ��ǰת��
		DateLengthTemp=*(stringEnd-i)-'0';
		DateLength+=(DateLengthTemp*(pow(10,i-1)));
		ErrotNum++;
		if(ErrotNum>=4)
		{
#ifdef DEBUG_RANK_DIAMOND		
printf("Rec TCP Date Over Rang\r\n");
#endif	
			return ERROR;
		}
	}
	for(i=0;i<DateLength;i++)
	{
		g_aTcpRecDateBuff[i]=*(stringEnd+1+i);
	}
	NeoWayExternalPar.RecTcpDateState=ON;
	return DateLength;
}


static void Empty_Par(void)
{
	memset(&NeoWayRec,0,sizeof(NeoWayRec));
	memset(&NeoWayInf,0,sizeof(NeoWayInf));
	memset(&NeoWaySysPar,0,sizeof(NeoWaySysPar));
	memset(&NeoWayExternalPar,0,sizeof(NeoWayExternalPar));	
	//���BuffAndDate
	memset(&g_aNeoWayRecBuff,0,sizeof(g_aNeoWayRecBuff));
	memset(&g_aNeoWayRec,0,sizeof(g_aNeoWayRec));
	memset(&g_aTcpRecDateBuff,0,sizeof(g_aTcpRecDateBuff));
	memset(&g_aTcpSendDate,0,sizeof(g_aTcpSendDate));
	g_uSendTcpDateNum = 0;
	
}
static void ReBuild_NetWork(void)
{
	//ȷ�����buff���ݺ���
	BuildPPP_Connet();
	BuildTCP_Connet();
}

static void ReBootHardware_Module(void)
{
	Empty_Par();
	//ȱ��Ӳ����������	
	PowerOff_Module();
	PowerOn_Module();
	NeoWaySysPar.Init.StartInitState = ON;
}	

static void ReBootSofeware_Module(void)
{
	
}	
static void PowerOff_Module(void)
{
	GPIO_PinSet(PORT_ON_OFF);	
	Delay_ms(600);
	GPIO_PinClear(PORT_ON_OFF);	
	Delay_ms(6000);	
}
static void PowerOn_Module(void)
{
	GPIO_PinClear(PORT_ON_OFF);	
}

static void Protect_Connet(void)
{
	static uint16 Temp = 0;
	if((OFF == NeoWaySysPar.NetWork.ConnetTCPState)||
		(OFF == NeoWaySysPar.NetWork.ConnetPPPState))
	{
		if(Temp>=NEOWAY_REC_REBOOT_TIME)
		{//�������������
			Temp=0;
			Connet_ErrorCnt++;
			ReBootHardware_Module();
		}else
		{
			Temp++;
		}
	}else
	{		
		Temp = 0;
		NeoWayExternalPar.NetWorkConnetState = ON;
	}
	
	if(ON == NeoWayExternalPar.HardwareRebootState)
	{//����Ӳ����λ
		Connet_ErrorCnt++;
		ReBootHardware_Module();		
	}
	if(ON == NeoWayExternalPar.SofewareRebootState)
	{//���������λ
	
	}
	if(ON == NeoWaySysPar.Init.StartInitState)
	{
		NeoWaySysPar.Init.StartInitState = OFF;
		NeoWay_Init();
	}

}
