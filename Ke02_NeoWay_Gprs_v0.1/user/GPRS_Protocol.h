#ifndef _GPRS_PROTOCOL_H_
#define _GPRS_PROTOCOL_H_
#include "common.h"
#include "crc.h"
#include "NeoWay_Gprs.h"
#include "Connet_Master.h"
#include "Init.h"
#include "gpio.h"

#define SERIAL_ID_LENGTH        20
#define IP_NUMBER               3
#define SERVER_IP_LENGTH        4
#define SERVER_PORT_LENGTH      2
#define CHAIR_ALARM             1  
#define FAULT             0

#define FRAME_MAX_LENGTH  128

#define FRAME_HEADER            0X5E
#define FRAME_END               0x7E

#define ORDER_FIRST_OPEN        0x01     //开机上报
#define ORDER_HEARTBEAT         0x02     //心跳包 
#define ORDER_ALARM             0x03     //告警
#define ORDER_INQUIRE           0x04     //查询
#define ORDER_SET               0x05     //设置
#define ORDER_SendInfo			0x06     //上送信息

#define ACK_SUCCEED             0x01     //成功
#define ACK_CRC_FAULT           0x02     //CRC校验错误
#define ACK_ORDER_NO_REPLY      0xFD     //命令，无需应答
#define ACK_ORDER_MUST_REPLY    0xFE    //命令，必须应答


#define SERIAL_ID       {'3','3','0','C'}
//#define SERIAL_ID       {0}

#define SERVER_IP_1       {121,43,115,207}
#define SERVER_PORT_1     {0x17,0x71}   //端口号:6001
#define SERVER_IP_2       {121,43,115,207}
#define SERVER_PORT_2     {0x17,0x71}   //端口号:6001
#define SERVER_IP_3       {121,43,115,207}
#define SERVER_PORT_3     {0x17,0x71}   //端口号:6001
typedef struct { 
    uint16 flag;
    struct{
	uint8 ActiveReportFlag;
	uint8 ReportFirstOpen;  
	uint8 ReportHeartbeat;
	uint8 ReportAlarm;
	uint8 SlaveReceiveFlag; 
	uint8  ReceiveInquire;
	uint8  ReceiveSet; 
	uint8  SetProgram;
  uint8 Start_Program;
  uint8 Stop_Chair;
  uint8 Lock_Chair;
  uint8 StartReceive;
  uint8 SucceedReceive;
  uint8 Deal_data;
  uint16 HeartbeatTime;
  uint8  HeartbeatFaultNum;
  uint16 AlarmTime;
  uint8  AlarmFlag;
  uint8 BodyTestFlag;
  uint8 GprsConnectFlag;
  uint8 ReceiceDebugData;
	}bits;	
}GprsFlag;


typedef struct{
    uint8  Serial_Id[SERIAL_ID_LENGTH];
}Monitor_Target_01 ;

typedef struct{
    uint8  Server_Ip_1[SERVER_IP_LENGTH];    
    uint8  Server_Port_1[SERVER_PORT_LENGTH];
    uint8  Server_Ip_2[SERVER_IP_LENGTH];    
    uint8  Server_Port_2[SERVER_PORT_LENGTH];
    uint8  Server_Ip_3[SERVER_IP_LENGTH];    
    uint8  Server_Port_3[SERVER_PORT_LENGTH];
}Monitor_Target_02;

typedef struct {
    uint8  Chair_Alarm;     
}Monitor_Target_03;

typedef struct {
    uint8   Start_Program;
    uint8   Stop_Chair;
    uint8   Lock_Chair;
}Monitor_Target_04;

typedef struct {
    uint8   Chair_ServiceCondition;
}Monitor_Target_05 ;

enum  Device_InfoNum{
  Head_DEV      = 0x00,         
    Serial_Id   = 0x03

};

enum Web_ParamNum{
  Head_WEB      = 0x01,
    Server_Ip_1   = 0x01,
    Server_Port_1 = 0x02,
    Server_Ip_2   = 0x03,
    Server_Port_2 = 0x04,
    Server_Ip_3   = 0x05,
    Server_Port_3 = 0x06
};

enum Alarm_StateNum{
  Head_ALA      = 0x03,
    Chair_Alarm = 0x01    
};

enum Set_ParamNum{
  Head_SET        = 0x04,
    Start_Program = 0x01,   
    Stop_Chair  = 0x02,    
    Lock_Chair  = 0x03
};

enum Collect_DataNum{
  Head_Coll         = 0x05,
    Chair_ServiceCondition = 0x01    
};

extern Monitor_Target_01 Device_Info;
extern Monitor_Target_02 Web_Param;
void GprsRec_Date(uint8* Date,uint8 num);
static void GprsSend_Date(uint8* Date,uint8 num);
void Gprs_1SEvent(void);
static void Gprs_FirstOpenEvent(void);
static void Gprs_AlarmEvent(void);
static void Gprs_HeartbeatEvent(void);
uint8 Gprs_StartCodeEvent(void);
uint8  Gprs_ReceiveEvent(void);
static uint8 Read_AckCmd(const uint8 *source , uint8 *target ,const uint8 length);
static void Gprs_CRC_Init(void);
static unsigned short crc16(uint8 *ptr, int len);
static uint8 Check_Size(const uint8 *source, uint8 length);
void Init_Gprs_Device(void);
static  uint8 ServerToGprs_ChangeMean(  uint8 *source, uint8 *target,uint8 source_length );
static uint8 GprsToServer_ChangeMean(  uint8 *source, uint8 *target,uint8 source_length );
static uint8 Monitor_Quantity( const uint8 * source,const uint8 total_length);
static uint8 Send_InquireInfo(const uint8 * source,uint8 *target,const uint8 total_length);
static uint8 Send_SetInfo(const uint8 * source,uint8 *target,const uint8 total_length);
static uint8 Send_FirstOpen(uint8 *target);
static uint8 Send_Heartbeat(uint8 *target);
static uint8 Send_Alarm(uint8 *target);
static uint8* Head_SearchAddr(const uint8 *monitor_head);
static uint8* Matching_DEV(const uint8 *dev);
static uint8* Matching_WEB(const uint8 *web);
static uint8* Matching_ALA(const uint8 *ala);
static uint8* Matching_SET(const uint8 *set);
static uint8* Matching_Coll(const uint8 *coll);
static void Group_SendData(uint8 *source,uint8 length);


#endif
