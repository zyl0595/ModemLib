/* 
* 文件名称：MDPort.c
* 摘    要：实现移植ModemLib需要实现的函数，用于系统延时，与模块
*           通信的数据发送和接收缓存读取等
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#include <Windows.h>
#include <stdio.h>
#include "uart.h"
#include "fifo.h"
#include "ucos_ii.h"
#include "MDType.h"

/*************************** 私有定义 ******************************/
#define MD_COM_BAUD_RATE 115200 /*与模块通信串口波特率*/
#define MD_COM_FIFO_SIZE 2048   /*与模块通信数据接收FIFO大小（字节）*/

/*************************** 私有变量 ******************************/
static int s_handle;
static const unsigned char s_Port[] = "COM4";

static unsigned char s_fifoBuf[MD_COM_FIFO_SIZE];
static FIFO_BUF s_fifo;
OS_EVENT *pFifoLock = NULL;

sMDMsg s_UartRcvMsg = {
    MSG_UART_RCV,
    NULL,
};

/*************************** 外部变量 ******************************/
extern OS_EVENT *g_MsgQ;
extern uint8_t g_AtIsIdle;

/*************************** 函数声明 ******************************/
void MD_UartReadTask(void *arg);/*串口数据接收进程，模拟真实使用时的串口接收中断*/

/*************************** 函数实现 ******************************/
/*
* 函数说明：延时函数
*
* 参    数：
*   [in] ms:延时时间（单位：ms）
*   [out]无
* 返回值：无
*/
void MD_Delay(unsigned short ms)
{
    Sleep(ms);
}

int MD_TtysOpen(void)
{
	s_handle = uart_open(s_Port, MD_COM_BAUD_RATE);
	return s_handle;
}

int MD_ReadByte(unsigned char* pCh)
{
    //return uart_read(s_handle, pCh, 1);
    return FifoReadByte(&s_fifo, pCh);
}



/*
* 函数说明：底层初始化，打开通信串口、初始化数据接收发送
*
* 参    数：
*   [in] 无
*   [out]无
* 返回值：成功返回1，失败返回0。
*/
int MD_LowLayInit(void)
{
    /*打开物理通信口*/
    s_handle = uart_open(s_Port, MD_COM_BAUD_RATE);
    if(NULL == s_handle){
        printf("%s open failed!\r\n");
        return 0;
    }

    /*初始化接收缓存*/
    FifoBufInit(&s_fifo, s_fifoBuf, sizeof(s_fifoBuf));//Windows 平台本身串口接收就实现了一个FIFO
    pFifoLock = OSSemCreate(1);
    if(NULL == pFifoLock){
        printf("Uart rcv fifo lock create failed!\r\n");
        return 0;
    }

    /*创建数据接收线程*/
    OSTaskCreateExt(MD_UartReadTask,
        NULL,
        NULL,
        0,
        2,
        NULL,
        1024,
        NULL,
        OS_TASK_OPT_STK_CHK);

    return 1;
}


/*
* 函数说明：写串口（向模块发送数据）
* 
* 参    数：
*   [in] pSrc:  指向要发送的数据
*   [in] len:   要发送的数据字节数
* 返回值：  实际发送的字节数
*/
int MD_WriteBuf(const unsigned char *pSrc, int len)
{
    return uart_write(s_handle, pSrc, len);
}


/*
* 函数说明：读取接收缓存
*
* 参数：
*   [out]pDes:  指向存储读取数据的缓存区
*   [in] maxLen:最大读取字节数
* 返回值：  实际读取字节数
*/
int MD_ReadBuf(unsigned char *pDes, int maxLen)
{
    //return uart_read(s_handle, pDes, maxLen);
    return FifoReadMultByte(&s_fifo, pDes, maxLen);
}


void MD_UartReadTask(void *arg)
{
    unsigned char ch;
    int ret;
    int waitCnt = 0;
    int isReading = 0;
    unsigned int readCnt = 0;
    INT8U err;

    do{
        ret = uart_read(s_handle, &ch, 1);
        if(ret){
            waitCnt = 0;
            isReading = 1;
            readCnt ++;
            //OSSemPend(pFifoLock, 0, )//--------------------------------------------------------------尚未完成！！！
            ret = FifoWriteByte(&s_fifo, ch);
            if(!ret)printf("Uart rcv fifo write err!\r\n");
        }else{
            
            if(isReading){
                Sleep(1);
                waitCnt++;
                if(waitCnt > 10){
                    waitCnt = 0;
                    isReading = 0;
                    /*帧接收完毕，发送信号告知MD处理新数据*/
                    //printf("New data Rcved! len:%d\r\n", readCnt);
                    if(TRUE == g_AtIsIdle){//收到URC
                        //printf("post urc msg...\r\n"); //---------------------------------------这边暂时不去优化
                        err = OSQPost(g_MsgQ, &s_UartRcvMsg);
                        readCnt = 0;
                        if(1 != err){
                            printf("Uart rcv msg post err! %d\r\n", err);
                        }
                    }
                }
            }else{
                Sleep(10);
            }
        }
    }while(1);

    //不能执行到这
    printf("\r\n!!!Uart read task return!!!\r\n");
}