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
#define SERVER_IP_LENGTH        20
#define SERVER_PORT_LENGTH      2
#define CHAIR_ALARM             1  
#define FAULT             0

#define FRAME_MAX_LENGTH  128
#define PROTECT_START_TIME 4
#define PROTECT_PROGRAM_TIME 1500
#define FRAME_HEADER            0X5E
#define FRAME_END               0x7E

#define ORDER_FIRST_OPEN        0x01     //开机上报
#define ORDER_HEARTBEAT         0x02     //心跳包 
#define ORDER_ALARM             0x03     //告警
#define ORDER_INQUIRE           0x04     //查询
#define ORDER_SET               0x05     //设置
#define ORDER_SendInfo			    0x06     //上送信息

#define ACK_SUCCEED             0x01     //成功
#define ACK_CRC_FAULT           0x02     //CRC校验错误
#define ACK_ORDER_NO_REPLY      0xFD     //命令，无需应答
#define ACK_ORDER_MUST_REPLY    0xFE    //命令，必须应答



//#define SERIAL_ID       {0}

#define SERVER_IP_1       {122,43,115,207}
#define SERVER_PORT_1     {0x17,0x71}   //端口号:6001
#define SERVER_IP_2       {121,43,115,207}
#define SERVER_PORT_2     {0x17,0x71}   //端口号:6001
#define SERVER_IP_3       {121,43,115,207}
#define SERVER_PORT_3     {0x17,0x71}   //端口号:6001
typedef struct { 
    uint16 flag;
    struct{
	uint8 ReportFirstOpen;  
	uint8 ReportHeartbeat;
	uint8 ReportAlarm;
	uint8  ReceiveSet; 
  uint8 Start_Program;
  uint8 Stop_Chair;
  uint8 Lock_Chair;
  uint8 Deal_data;
  uint16 HeartbeatTime;
  uint8  HeartbeatFaultNum;
  uint16 AlarmTime;
  uint8  AlarmFlag;
  uint8 StartProgram;
  uint16 ProtectOpenProgramTime;
  uint8 StateChangeFlag;//bit0 对应0x0502，bit1 对应0x0503
  uint8 StateChange;
  }bits;  
}GprsFlag;


typedef struct{
    uint8  Serial_Id[SERIAL_ID_LENGTH];
    uint8  Cell_Id[4];
    uint8  Local_Id[8];
}Monitor_Target_01 ;

typedef struct{
    uint8  Server_Ip_1[SERVER_IP_LENGTH];    
    uint8  Server_Port_1[SERVER_PORT_LENGTH];
    uint8  Server_Ip_2[SERVER_IP_LENGTH];    
    uint8  Server_Port_2[SERVER_PORT_LENGTH];
    uint8  Server_Ip_3[SERVER_IP_LENGTH];    
    uint8  Server_Port_3[SERVER_PORT_LENGTH];
    int16  Gprs_Strength;
    int16  Gprs_ErrorRate;
}Monitor_Target_02;

typedef struct {
    uint8  Chair_Alarm;  
    uint8  Gprs_Alarm;
}Monitor_Target_03;

typedef struct {
    uint8   Start_Program;
    uint8   Stop_Chair;
    uint8   Lock_Chair;
}Monitor_Target_04;

typedef struct {
    uint8   Chair_ServiceCondition; //0x01 未使用、0x02正在使用、0x03设备故障、0x04设备检修
    uint8   Start_ProgramState; //0x01没有按摩程序启动、0x02按摩椅程序启动成功、0x03按摩椅程序启动不成功
    uint8   Chair_State; //bit0 开关机、bit1 暂停状态、bit4 错误标志
    uint8   Program_State;//按摩椅程序启动标志位 bit0颈间按摩状态、bit1腰部按摩状态、bit4全身按摩状态
}Monitor_Target_05 ;

enum  Device_InfoNum{
  Head_DEV      = 0x00,         
    Serial_Id   = 0x03,
    Cell_Id     = 0x04,
    Local_Id    = 0x05

};

enum Web_ParamNum{
  Head_WEB      = 0x01,
    Server_Ip_1   = 0x01,
    Server_Port_1 = 0x02,
    Server_Ip_2   = 0x03,
    Server_Port_2 = 0x04,
    Server_Ip_3   = 0x05,
    Server_Port_3 = 0x06,
    Gprs_Strength = 0x07,
    Gprs_ErrorRate= 0x08
};

enum Alarm_StateNum{
  Head_ALA      = 0x03,
    Chair_Alarm = 0x01,
    Gprs_Alarm = 0x02
};

enum Set_ParamNum{
  Head_SET        = 0x04,
    Start_Program = 0x01,   
    Stop_Chair  = 0x02,    
    Lock_Chair  = 0x03
};

enum Collect_DataNum{
  Head_Coll         = 0x05,
    Chair_ServiceCondition = 0x01,
    Start_ProgramState = 0x02 ,
    Chair_State = 0x03,
    Program_State =0x04
};

extern Monitor_Target_01 Device_Info;
extern Monitor_Target_02 Web_Param;
extern Monitor_Target_03 Alarm_State;
extern Monitor_Target_04 Set_Param;
extern Monitor_Target_05 Collect_Data;
extern GprsFlag Gprs_flag;

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
static uint8 Send_GprsAlarm(uint8 *target);
static uint8* Head_SearchAddr(const uint8 *monitor_head);
static uint8* Matching_DEV(const uint8 *dev);
static uint8* Matching_WEB(const uint8 *web);
static uint8* Matching_ALA(const uint8 *ala);
static uint8* Matching_SET(const uint8 *set);
static uint8* Matching_Coll(const uint8 *coll);
static void Group_SendData(uint8 *source,uint8 length);
void Send_ChangeState(uint8 *target,uint16 Monitor_Target);
void ChairStateChange(void);

#endif
