/* 
* �ļ����ƣ�MDPort.c
* ժ    Ҫ��ʵ����ֲModemLib��Ҫʵ�ֵĺ���������ϵͳ��ʱ����ģ��
*           ͨ�ŵ����ݷ��ͺͽ��ջ����ȡ��
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#include <Windows.h>
#include <stdio.h>
#include "uart.h"
#include "fifo.h"
#include "ucos_ii.h"
#include "MDType.h"

/*************************** ˽�ж��� ******************************/
#define MD_COM_BAUD_RATE 115200 /*��ģ��ͨ�Ŵ��ڲ�����*/
#define MD_COM_FIFO_SIZE 2048   /*��ģ��ͨ�����ݽ���FIFO��С���ֽڣ�*/

/*************************** ˽�б��� ******************************/
static int s_handle;
static const unsigned char s_Port[] = "COM4";

static unsigned char s_fifoBuf[MD_COM_FIFO_SIZE];
static FIFO_BUF s_fifo;
OS_EVENT *pFifoLock = NULL;

sMDMsg s_UartRcvMsg = {
    MSG_UART_RCV,
    NULL,
};

/*************************** �ⲿ���� ******************************/
extern OS_EVENT *g_MsgQ;
extern uint8_t g_AtIsIdle;

/*************************** �������� ******************************/
void MD_UartReadTask(void *arg);/*�������ݽ��ս��̣�ģ����ʵʹ��ʱ�Ĵ��ڽ����ж�*/

/*************************** ����ʵ�� ******************************/
/*
* ����˵������ʱ����
*
* ��    ����
*   [in] ms:��ʱʱ�䣨��λ��ms��
*   [out]��
* ����ֵ����
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
* ����˵�����ײ��ʼ������ͨ�Ŵ��ڡ���ʼ�����ݽ��շ���
*
* ��    ����
*   [in] ��
*   [out]��
* ����ֵ���ɹ�����1��ʧ�ܷ���0��
*/
int MD_LowLayInit(void)
{
    /*������ͨ�ſ�*/
    s_handle = uart_open(s_Port, MD_COM_BAUD_RATE);
    if(NULL == s_handle){
        printf("%s open failed!\r\n");
        return 0;
    }

    /*��ʼ�����ջ���*/
    FifoBufInit(&s_fifo, s_fifoBuf, sizeof(s_fifoBuf));//Windows ƽ̨�����ڽ��վ�ʵ����һ��FIFO
    pFifoLock = OSSemCreate(1);
    if(NULL == pFifoLock){
        printf("Uart rcv fifo lock create failed!\r\n");
        return 0;
    }

    /*�������ݽ����߳�*/
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
* ����˵����д���ڣ���ģ�鷢�����ݣ�
* 
* ��    ����
*   [in] pSrc:  ָ��Ҫ���͵�����
*   [in] len:   Ҫ���͵������ֽ���
* ����ֵ��  ʵ�ʷ��͵��ֽ���
*/
int MD_WriteBuf(const unsigned char *pSrc, int len)
{
    return uart_write(s_handle, pSrc, len);
}


/*
* ����˵������ȡ���ջ���
*
* ������
*   [out]pDes:  ָ��洢��ȡ���ݵĻ�����
*   [in] maxLen:����ȡ�ֽ���
* ����ֵ��  ʵ�ʶ�ȡ�ֽ���
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
            //OSSemPend(pFifoLock, 0, )//--------------------------------------------------------------��δ��ɣ�����
            ret = FifoWriteByte(&s_fifo, ch);
            if(!ret)printf("Uart rcv fifo write err!\r\n");
        }else{
            
            if(isReading){
                Sleep(1);
                waitCnt++;
                if(waitCnt > 10){
                    waitCnt = 0;
                    isReading = 0;
                    /*֡������ϣ������źŸ�֪MD����������*/
                    //printf("New data Rcved! len:%d\r\n", readCnt);
                    if(TRUE == g_AtIsIdle){//�յ�URC
                        //printf("post urc msg...\r\n"); //---------------------------------------�����ʱ��ȥ�Ż�
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

    //����ִ�е���
    printf("\r\n!!!Uart read task return!!!\r\n");
}