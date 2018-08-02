/* 
* �ļ����ƣ�MDPort.h
* ժ    Ҫ��������ֲModemLib��Ҫʵ�ֵĺ���������ϵͳ��ʱ����ģ��
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


#ifndef __MD_PORT_H
#define __MD_PORT_H

#ifdef __cplusplus 
extern "C" { 
#endif

/************ ��ֲ��Ҫʵ�ֵĺ��� ************/

/*
* ����˵������ʱ����
*
* ��    ����
*   [in] ms:��ʱʱ�䣨��λ��ms��
*   [out]��
* ����ֵ����
*/
void MD_Delay(unsigned short ms);

//���ڲ���
int MD_TtysOpen(void);
int MD_TtysClose(void);
int MD_ReadByte(unsigned char *pCh);

/*
* ����˵�����ײ��ʼ������ͨ�Ŵ��ڡ���ʼ�����ݽ��շ���
*
* ��    ����
*   [in] ��
*   [out]��
* ����ֵ���ɹ�����1��ʧ�ܷ���0��
*/
int MD_LowLayInit(void);

/*
* ����˵����д���ڣ���ģ�鷢�����ݣ�
* 
* ��    ����
*   [in] pSrc:  ָ��Ҫ���͵�����
*   [in] len:   Ҫ���͵������ֽ���
* ����ֵ��  ʵ�ʷ��͵��ֽ���
*/
int MD_WriteBuf(const unsigned char *pSrc, int len);

/*
* ����˵���������ڽ��ջ���
*
* ������
*   [out]pDes:  ָ��洢��ȡ���ݵĻ�����
*   [in] maxLen:����ȡ�ֽ���
* ����ֵ��  ʵ�ʶ�ȡ�ֽ���
*/
int MD_ReadBuf(unsigned char *pDes, int maxLen);


#ifdef __cplusplus 
	}
#endif

#endif
