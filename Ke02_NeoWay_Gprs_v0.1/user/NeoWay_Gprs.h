#ifndef __NEO_WAY__GPRS__H__
#define __NEO_WAY__GPRS__H__

#include "string.h"
#include "common.h"
#include "sysinit.h"
#include "math.h"
#include "uart.h"
#include "uartApp.h"
#include "gpio.h"
#define DEBUG
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

#define UART0_BUFFLENGTH   10

#define PORT_EMERGOFF  	GPIO_PTC1
#define PORT_ON_OFF   	GPIO_PTD5 
#define HARDWARE_POWR_CONTORL    GPIO_PTD1
//帧间隔时间
#define NEOWAY_REC_INTERVAL_TIME  20
#define NEOWAY_REC_REBOOT_TIME 120

#define NEOWAY_UART   UART0
#define NEOWAY_DEBUG_UART UART1
//发送等待200ms
#define NEOWAY_WAIT_TIME 200
#define NEOWAY_SEND_WAIT_TIME 70
//使用的是什么网络（移动、联通）,只能定义其中一种
#define CMNET
//#define UNINET

//链路，服务器IP，端口号
#define SEND_TCP_IP ("AT+TCPSETUP=0,121.43.115.207,6001\r")

#define DNS_WEB ("at+dns=\"www.ogawaplus.com\"\r")


//接收用到的参数
typedef struct{
	uint8 	Receiving;
	uint8   Dealing;
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
		uint8 ModuleRunning;
		uint8 FindSimState;
		uint8 StartInitState;
		uint8 GprsSendState;
	}Init;
	struct{
		uint8 ConnetPPPState;
		uint8 ConnetTCPState;
		uint8 ConnetServerState;
    uint8 IpSelectNum;
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
void Delay_ms(uint16 num);
void NeoWayBoard_Init(void);
static void NeoWayIp_Init(void);
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
static void DNS_AnalysisGetIp(void);
static uint8 BuildTCP_Connet(void);
uint8 TcpSend_Date(void);
uint8 Gprs_Send_Date(void);
static uint16 ReceiveTCP_Date(void);
static char* itoa(int num,char*str,int radix);
void ModuleBack_Code(void);
static void Empty_Par(void);
static void ReBuild_NetWork(void);
void ReBootHardware_Module(void);
static void ReBootSofeware_Module(void);
void ReBoot_Module(void);
static void PowerOff_Module(void);
static void PowerOn_Module(void);
static void Protect_Connet(void);
#endif
