#include "uartApp.h"
#include "Connet_Master.h"
static  uint8 *pUART_TxBuff[MAX_UART_NO] = {NULL};           /* pointer to RxBuf */
static  uint8  *pUART_RxBuff[MAX_UART_NO] = {NULL};           /* pointer to TxBuf */
static  uint16 gu16UART_TxBuffPos[MAX_UART_NO] = {0};        /* write position to RxBuf */
static  uint16 gu16UART_RxBuffPos[MAX_UART_NO] = {0};        /* read position to TxBuf */
static  uint32 gu32UART_BuffSize[MAX_UART_NO] = {0};         /* buffer size*/

extern void NeoWay_UartRec(uint8 Date);
void UART_SendInt(UART_Type *pUART, uint8_t *pSendBuff, uint32_t u32Length)
{
    uint8_t u8Port = ((uint32_t)pUART-(uint32_t)UART0)>>12;	//计算哪一个Uart Port
    
    /* save user defined send buffer pointers and size */
    pUART_TxBuff[u8Port]        = pSendBuff;		//设置Uart发送buffer指针
    gu32UART_BuffSize[u8Port]   = u32Length;
    gu16UART_TxBuffPos[u8Port]  = 0;

    UART_EnableTxBuffEmptyInt(pUART);   /* enable tx interrupt */ 
}
void UART_ReceiveInt(UART_Type *pUART, uint8 *pReceiveBuff, uint32 u32Length)
{
    uint8_t u8Port = ((uint32_t)pUART-(uint32_t)UART0)>>12;
    
    /* save user defined read buffer pointers and size */
    pUART_RxBuff[u8Port]        = pReceiveBuff;	
    gu32UART_BuffSize[u8Port]   = u32Length;	 
    gu16UART_RxBuffPos[u8Port]  = 0;						

    UART_EnableRxBuffFullInt(pUART);  					
}

void UART_HandleInt(UART_Type *pUART)
{
    uint8   u8Port;
    //uint8   *pRdBuff;
    uint8   *pWrBuff;  
    uint8   read_data = 0;
    volatile uint8 read_temp = 0;
    
    u8Port = ((uint32)pUART-(uint32)UART0)>>12;
    
	if(u8Port == 1)
    {
    	if(UART_CheckFlag(pUART,UART_FlagOR))   //溢出
    	{
    	  read_data = UART_ReadDataReg(pUART);     
    	}else if(UART_IsRxBuffFull(pUART))  //缓冲区是否满
    	{     
    		 read_data = UART_ReadDataReg(pUART); //读到数据
    		 NeoWay_UartRec(read_data);
    	}else if(UART_IsTxBuffEmpty(pUART))
    	{ 		
    			if(gu16UART_TxBuffPos[u8Port] != gu32UART_BuffSize[u8Port])
    	        {             
    	            pWrBuff = pUART_TxBuff[u8Port];
    	            UART_WriteDataReg(pUART, pWrBuff[gu16UART_TxBuffPos[u8Port]++]);     
    	        }  
    	        else
    	        {
    				UART_DisableTxBuffEmptyInt(pUART);				
    	        }
    		}
	}
	if(u8Port == 0)
    {
    	if(UART_CheckFlag(pUART,UART_FlagOR))   //溢出
    	{
    	  read_data = UART_ReadDataReg(pUART);     
    	}else if(UART_IsRxBuffFull(pUART))  //缓冲区是否满
    	{     
    		 read_data = UART_ReadDataReg(pUART); //读到数据
    		 RecMaster_Uart(read_data);
    	}else if(UART_IsTxBuffEmpty(pUART))
    	{ 		
    			if(gu16UART_TxBuffPos[u8Port] != gu32UART_BuffSize[u8Port])
    	        {             
    	            pWrBuff = pUART_TxBuff[u8Port];
    	            UART_WriteDataReg(pUART, pWrBuff[gu16UART_TxBuffPos[u8Port]++]);     
    	        }  
    	        else
    	        {
    				UART_DisableTxBuffEmptyInt(pUART);				
    	        }
    		}
	}
}
void Deal_Receive_data(uint8 data)
{
    volatile uint8 temp=0;
    temp=data;
    if(0x01==temp)
    {
         printf("ok");
    }
}
