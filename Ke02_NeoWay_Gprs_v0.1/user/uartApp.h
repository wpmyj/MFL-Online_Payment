#ifndef __UART__APP__H__
#define __UART__APP__H__
#include "common.h"
#include "sysinit.h"
#include "uart.h"
void UART_SendInt(UART_Type *pUART, uint8_t *pSendBuff, uint32_t u32Length);
void UART_ReceiveInt(UART_Type *pUART, uint8 *pReceiveBuff, uint32 u32Length);
void UART_HandleInt(UART_Type *pUART);
#endif
