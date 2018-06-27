#include <Windows.h>
#include <stdio.h>
#include "uart.h"


HANDLE uart_open(char *CommName, int rate)
{
	char ucTmpBuf[128];
	HANDLE hCom;
	COMMTIMEOUTS TimeOuts;

	if(strlen(CommName)>4)
		sprintf(ucTmpBuf, "\\\\.\\%s", CommName);	
	else
		strcpy(ucTmpBuf, CommName);

	hCom = CreateFile(ucTmpBuf, GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	

	if(hCom==INVALID_HANDLE_VALUE)
		return hCom;

	uart_set_speed(hCom, rate);	

	SetupComm(hCom, 10*1024,10*1024);//set in out buf size

	//设定读超时
	TimeOuts.ReadIntervalTimeout=10;
	TimeOuts.ReadTotalTimeoutMultiplier=10;
	TimeOuts.ReadTotalTimeoutConstant=10;

	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier=10;
	TimeOuts.WriteTotalTimeoutConstant=10;
	SetCommTimeouts(hCom,&TimeOuts); 

	PurgeComm(hCom, PURGE_TXCLEAR|PURGE_RXCLEAR);//clear rx tx
	return hCom;
}

int uart_write(HANDLE hCom, void *pSrc, int len)
{
	DWORD writeLen=0;
	DWORD sendLen = len;

#if 0
	unsigned char *pTmp = (unsigned char*)pSrc;

	while(len>0){
		writeLen=0;
		WriteFile(hCom, pTmp, (DWORD)1, &writeLen, NULL);

		len -= writeLen;
		pTmp += writeLen;
		Sleep(1);
	}
#elif 0
	unsigned char *pTmp = (unsigned char*)pSrc;

	while(len>0){
		writeLen=0;
		WriteFile(hCom, pTmp, (DWORD)len, &writeLen, NULL);

		len -= writeLen;
		pTmp += writeLen;
	}
#else
	// Clear buffer
	//PurgeComm(hCom, PURGE_TXCLEAR | PURGE_TXABORT);
	if(WriteFile(hCom, pSrc, (DWORD)len, &writeLen, NULL)==FALSE)
		return 0;
#endif
	return (int)sendLen;
}

int uart_read(HANDLE hCom, void *pRet, int retLen)
{
	DWORD readLen=0;

	if(uart_read_count(hCom)<=0)
		return 0;

	if(ReadFile(hCom, pRet, retLen, &readLen, NULL)==FALSE)
		return 0;

	return (int)readLen;
}

int uart_read_count(HANDLE hCom)
{
	DWORD errors;
	COMSTAT Stat;

	if(ClearCommError(hCom, &errors, &Stat) == FALSE)
		return 0;

	return (int)Stat.cbInQue;
}

int uart_close(HANDLE hCom)
{
	if(hCom==INVALID_HANDLE_VALUE)
		return 0;
	return CloseHandle(hCom);
}

void uart_empty(HANDLE hCom)
{
	PurgeComm(hCom, PURGE_RXCLEAR);
}

void uart_set_rts(HANDLE hCom, int isEnable)
{
	DCB dcbParam;
	if(GetCommState(hCom, &dcbParam) == FALSE)
	{
		uart_close(hCom);
		return;
	}
	
	if(isEnable){
		dcbParam.fRtsControl = RTS_CONTROL_ENABLE;
	}else{
		dcbParam.fRtsControl = RTS_CONTROL_DISABLE;
	}

	SetCommState(hCom, &dcbParam);//set uart param
}

void uart_set_speed(HANDLE hCom, int speed)
{
	DCB dcbParam;
	DWORD tab[] = {0,CBR_115200,CBR_57600,CBR_56000,CBR_38400,CBR_19200,CBR_14400,CBR_9600,CBR_4800,CBR_2400,CBR_1200,CBR_600,CBR_300,CBR_110};

	if(GetCommState(hCom, &dcbParam) == FALSE)
	{
		uart_close(hCom);
		return;
	}

	dcbParam.BaudRate = speed;
	dcbParam.ByteSize = 8;
	dcbParam.Parity = NOPARITY;
	dcbParam.StopBits = ONESTOPBIT;
	dcbParam.fRtsControl = RTS_CONTROL_DISABLE;
	dcbParam.fOutxCtsFlow = FALSE;

	SetCommState(hCom, &dcbParam);//set uart param
}

int uart_set_param(HANDLE hCom,int dataSize,char *pStopBitStr,char *pParityStr)
{	
	DCB dcbParam;
	DCB retParam;

	if(GetCommState(hCom, &dcbParam) == FALSE){
		return 0;
	}

	dcbParam.ByteSize = dataSize;

	if(strcmp(pStopBitStr,"1")==0){
		dcbParam.StopBits = ONESTOPBIT;
	}else if(strcmp(pStopBitStr,"1.5")==0){
		dcbParam.StopBits = ONE5STOPBITS;
	}else if(strcmp(pStopBitStr,"2")==0){
		dcbParam.StopBits = TWOSTOPBITS;
	}

	//None;Odd;Even;Mark;Space;
	if(strcmp(pParityStr,"None")==0){
		dcbParam.Parity = NOPARITY;
	}else if(strcmp(pParityStr,"Odd")==0){
		dcbParam.Parity = ODDPARITY;
	}else if(strcmp(pParityStr,"Even")==0){
		dcbParam.Parity = EVENPARITY;
	}else if(strcmp(pParityStr,"Mark")==0){
		dcbParam.Parity = MARKPARITY;
	}else if(strcmp(pParityStr,"Space")==0){
		dcbParam.Parity = SPACEPARITY;
	}	

	SetCommState(hCom, &dcbParam);//set uart param
	GetCommState(hCom, &retParam);
	if(memcmp(&dcbParam,&retParam,sizeof(DCB))){
		return 0;
	}

	return 1;
}
