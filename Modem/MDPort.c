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
#include "Fifo.h"


/*************************** ˽�ж��� ******************************/
#define MD_COM_BAUD_RATE 115200 /*��ģ��ͨ�Ŵ��ڲ�����*/
#define MD_COM_FIFO_SIZE 2048   /*��ģ��ͨ�����ݽ���FIFO��С���ֽڣ�*/

/*************************** ˽�б��� ******************************/
static int s_handle;
static const unsigned char s_Port[] = "COM4";

static unsigned char s_fifoBuf[MD_COM_FIFO_SIZE];
static FIFO_BUF s_fifo;

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
    return uart_read(s_handle, pCh, 1);
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

    /*�������ݽ����߳�*/

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
    return uart_read(s_handle, pDes, maxLen);
    //return FifoBufRead(&s_fifo, pDes, maxLen);
}
