#ifndef __UART_H__
#define __UART_H__

#include <Windows.h>

//CBR_115200,CBR_57600,CBR_56000,CBR_38400,CBR_19200,CBR_14400,CBR_9600,CBR_4800,CBR_2400,CBR_1200,CBR_600,CBR_300,CBR_110
HANDLE uart_open(char *CommName, int rate);
int uart_write(HANDLE hCom, void *pSrc, int len);
int uart_read(HANDLE hCom, void *pRet, int retLen);
int uart_read_count(HANDLE hCom);
int uart_close(HANDLE hCom);
void uart_empty(HANDLE hCom);
void uart_set_speed(HANDLE hCom, int speed);
//int GetSerialPort(CStringArray &arrCom);
int uart_set_param(HANDLE hCom,int dataSize,char *pStopBitStr,char *pParityStr);
void uart_set_rts(HANDLE hCom, int isEnable);

#endif