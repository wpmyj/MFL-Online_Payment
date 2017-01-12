#ifndef __CONNET_MASTER_H__
#define __CONNET_MASTER_H__
#include "string.h"
#include "common.h"
#include "sysinit.h"
#include "uart.h"
#include "uartApp.h"
#define UART_CONNET_MASTER  UART1
#define SERIAL_ID_LENGTH 20
typedef struct{
	struct{
		uint8 SysPower;
		uint8 Pause;
		uint8 Error;	
		uint8 NeckMassage;
		uint8 WaistMassage;
		uint8 WholeMassage;
	}State;
	
	uint8 Alarm_Num;
	uint8 Serial_Id[SERIAL_ID_LENGTH];
	uint16 RunningTime;
  uint16 LostMasterTime;
}Master_Inf_;
typedef struct{
	uint8 DownloadID_State;
	uint8 NoRespond_State;
	uint8 KeyValue;
  uint8 Start_ReadID;
}Ext_Inf_;

extern Master_Inf_ Master_Inf;
extern Ext_Inf_ Ext_Inf;

void ConnetBoard_Init(void);
void RecMaster_Uart(uint8 date);
void SendMaster_Date(void);
void SendMaster_KeyValue(uint8 KeyValue);
void Read_MasterID(void);
#endif
