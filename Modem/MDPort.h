/* 
* 文件名称：MDPort.h
* 摘    要：定义移植ModemLib需要实现的函数，用于系统延时，与模块
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


#ifndef __MD_PORT_H
#define __MD_PORT_H

#ifdef __cplusplus 
extern "C" { 
#endif

/************ 移植需要实现的函数 ************/

/*
* 函数说明：延时函数
*
* 参    数：
*   [in] ms:延时时间（单位：ms）
*   [out]无
* 返回值：无
*/
void MD_Delay(unsigned short ms);

//串口操作
int MD_TtysOpen(void);
int MD_TtysClose(void);
int MD_ReadByte(unsigned char *pCh);

/*
* 函数说明：底层初始化，打开通信串口、初始化数据接收发送
*
* 参    数：
*   [in] 无
*   [out]无
* 返回值：成功返回1，失败返回0。
*/
int MD_LowLayInit(void);

/*
* 函数说明：写串口（向模块发送数据）
* 
* 参    数：
*   [in] pSrc:  指向要发送的数据
*   [in] len:   要发送的数据字节数
* 返回值：  实际发送的字节数
*/
int MD_WriteBuf(const unsigned char *pSrc, int len);

/*
* 函数说明：读串口接收缓存
*
* 参数：
*   [out]pDes:  指向存储读取数据的缓存区
*   [in] maxLen:最大读取字节数
* 返回值：  实际读取字节数
*/
int MD_ReadBuf(unsigned char *pDes, int maxLen);

#ifdef __cplusplus 
	}
#endif

#endif
