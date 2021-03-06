#ifndef __NEO_WAY__GPRS__H__
#define __NEO_WAY__GPRS__H__

#include "string.h"
#include "common.h"
#include "sysinit.h"
#include "math.h"
#include "uart.h"
#include "uartApp.h"
#include "gpio.h"
//#define DEBUG
#ifdef  DEBUG
//关键信息-重要错误
#define DEBUG_RANK_DIAMOND     
//初始化以及函数调用信息
#define DEBUG_RANK_PLATINUM
//数据的发送与接收
#define DEBUG_RANK_BRONZE
#endif


#define ERROR    0
#define SUCCEED  1


#define NEOWAY_REC_MAX   128
#define NEOWAY_SEND_MAX  128
#define TCP_REC_DATE_MAX 128

#define UART1_BUFFLENGTH   10

#define PORT_EMERGOFF  	GPIO_PTD6
#define PORT_ON_OFF   	GPIO_PTD7 

//帧间隔时间
#define NEOWAY_REC_INTERVAL_TIME  20
#define NEOWAY_REC_REBOOT_TIME 40

#define NEOWAY_UART   UART1
//发送等待200ms
#define NEOWAY_WAIT_TIME 200
#define NEOWAY_SEND_WAIT_TIME 70
//使用的是什么网络（移动、联通）,只能定义其中一种
#define CMNET
//#define UNINET

//链路，服务器IP，端口号
#define SEND_TCP_IP ("AT+TCPSETUP=0,121.43.115.207,6001\r")



//接收用到的参数
typedef struct{
	uint8 	Receiving;
	uint16  Num;
	uint16  Time;
}NeoWayRec_;

//产品信息
typedef struct{
	//产品序列号
	uint8 Cgsn[15];
	//移动用户识别码
	uint8 Cimi[15];
	//网络的IP，不是后台的IP！！！！
	int8  Ip[20];
}NeoWayInf_;

typedef struct{
	struct{
		uint8 ModulePowerState;
		uint8 StartNum;
		uint8 FindSimState;
		uint8 FindSimNum;
		uint8 StartInitState;
		uint8 GprsSendState;
	}Init;
	struct{
		uint8 ConnetPPPState;
		uint8 ConnetTCPState;
		uint8 ConnetServerState;		
	}NetWork;
}NeoWaySysPar_;

typedef struct{
	uint8 RecTcpDateState;
	uint8 NetWorkConnetState;
	uint8 HardwareRebootState;
	uint8 SofewareRebootState;
	uint16 LoseTime;
}NeoWayExternalPar_;

extern NeoWayExternalPar_ NeoWayExternalPar;
extern NeoWaySysPar_ NeoWaySysPar;

void NeoWayBoard_Init(void);
void NeoWay_Rtc1s(void);
uint8 NeoWay_Init(void);
static uint8 Get_Cgsn(void);
static uint8 Get_Cimi(void);
static uint8 NetWork_Login(void);
static uint8 Signal_Strength(void);
static uint8 SetProtocol_Stack(void);
static void SetPDP_Format(void);
static void User_Authentication(void);
static uint8 BuildPPP_Connet(void);
static uint8 BuildTCP_Connet(void);
uint8 TcpSend_Date(void);
uint8 Gprs_Send_Date(void);
static uint16 ReceiveTCP_Date(void);
void ModuleBack_Code(void);
static void Empty_Par(void);
static void ReBuild_NetWork(void);
static void ReBootHardware_Module(void);
static void ReBoot_Module(void);
static void PowerOff_Module(void);
static void PowerOn_Module(void);
static void Protect_Connet(void);
#endif
