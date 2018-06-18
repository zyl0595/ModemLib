#include <Windows.h>
#include <stdio.h>
//#include "ucos_ii.h"
#include "uart.h"

#define TTYS0_BUF_LEN  8192

static char *s_com_file_name[6]={"COM1","COM2","COM6","COM3","COM6","COM5"};
static HANDLE s_com_handle[6];

static unsigned int ttyS0_rd;
static unsigned int ttyS0_wt;
static unsigned char ttyS0_buf[TTYS0_BUF_LEN];

void uart_set_speed(int type, int speed);
int uart_read_count(int type);

int uart_open(int type, int rate)
{
	char ucTmpBuf[128];

	if(strlen(s_com_file_name[type])>4){
		sprintf(ucTmpBuf,"\\\\.\\%s",s_com_file_name[type]);
	}else{
		strcpy(ucTmpBuf,s_com_file_name[type]);
	}

	if(s_com_handle[type]==NULL)
	{
		HANDLE hCom = CreateFile(ucTmpBuf, GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
		COMMTIMEOUTS TimeOuts;

		if(hCom==NULL)
			return 0;

		uart_set_speed(type, rate);	

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

		s_com_handle[type] = hCom;
	}	

	return 1;
}

int uart_write(int type, void *pSrc, int len)
{
	HANDLE hCom = 0;
	DWORD writeLen=0;


	hCom = s_com_handle[type];
	if(WriteFile(hCom, pSrc, (DWORD)len, &writeLen, NULL)==FALSE)
		return 0;

	return (int)writeLen;
}

int uart_read(int type, void *pRet, int retLen)
{
	HANDLE hCom = 0;
	DWORD readLen=0;

	hCom = s_com_handle[type];

	if(uart_read_count(type)<=0)
		return 0;

	if(ReadFile(hCom, pRet, retLen, &readLen, NULL)==FALSE)
		return 0;

	return (int)readLen;
}

int uart_read_count(int type)
{
	HANDLE hCom = 0;
	DWORD errors;
	COMSTAT Stat;

	hCom = s_com_handle[type];

	if(ClearCommError(hCom, &errors, &Stat) == FALSE)
		return 0;

	return (int)Stat.cbInQue;
}

int uart_close(int type)
{
	HANDLE hCom = s_com_handle[type];
	s_com_handle[type] = NULL;
	return CloseHandle(hCom);
}

void uart_empty(int type)
{
	PurgeComm(s_com_handle[type], PURGE_RXCLEAR);
}


void uart_set_speed(int type, int speed)
{
	DCB dcbParam;
	HANDLE hCom = s_com_handle[type];
	const DWORD tab[] = {0,CBR_115200,CBR_57600,CBR_56000,CBR_38400,CBR_19200,CBR_14400,CBR_9600,CBR_4800,CBR_2400,CBR_1200,CBR_600,CBR_300,CBR_110};

	if(GetCommState(hCom, &dcbParam) == FALSE)
	{
		uart_close(type);
		return;
	}


	dcbParam.BaudRate = tab[speed];
	dcbParam.ByteSize = 8;
	dcbParam.Parity = NOPARITY;
	dcbParam.StopBits = ONESTOPBIT;

	SetCommState(hCom, &dcbParam);//set uart param
}




void uart_put_ch(int *rd,int *wt,unsigned char *buf,int size,unsigned char U1RBR)
{
	int t_wt = *wt;

	buf[t_wt++] = U1RBR;
	if (t_wt >= TTYS0_BUF_LEN)
		t_wt = 0;

	*wt = t_wt;
}

void PutTtys0Buf(unsigned  char *p, int len)
{
	int i;

	for(i=0; i<len; i++){
		uart_put_ch(&ttyS0_rd, &ttyS0_wt, ttyS0_buf, sizeof(ttyS0_buf), p[i]);
	}

}

void uart_recv_task(void *p)
{
	unsigned char ch=0;
	while(1){
		if(uart_read(TTYS0,&ch,1)>0)
			uart_put_ch(&ttyS0_rd,&ttyS0_wt,ttyS0_buf,sizeof(ttyS0_buf),ch);

		//OSTimeDly(1);
	}
}
