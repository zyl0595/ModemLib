/* 
* 文件名称：MDPort.c
* 摘    要：实现移植ModemLib需要实现的函数，用于系统延时，与模块
*           通信的数据发送和接收缓存读取等
*  
* 作    者：
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
#include "Fifo.h"


/*************************** 私有定义 ******************************/
#define MD_COM_BAUD_RATE 115200 /*与模块通信串口波特率*/
#define MD_COM_FIFO_SIZE 2048   /*与模块通信数据接收FIFO大小（字节）*/

/*************************** 私有变量 ******************************/
static int s_handle;
static const unsigned char s_Port[] = "COM4";

static unsigned char s_fifoBuf[MD_COM_FIFO_SIZE];
static FIFO_BUF s_fifo;

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
    return uart_read(s_handle, pCh, 1);
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

    /*创建数据接收线程*/

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
    return uart_read(s_handle, pDes, maxLen);
    //return FifoBufRead(&s_fifo, pDes, maxLen);
}
