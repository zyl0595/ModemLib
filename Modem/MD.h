/* 
* �ļ����ƣ�MD.h
* ժ    Ҫ��
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#ifndef __MD_H
#define __MD_H

#ifdef __cplusplus 
extern "C" { 
#endif

int MD_Init(unsigned char *pBuf, unsigned int bufSize);
int MD_connect(void);
int MD_SendDataB(unsigned char fd, unsigned char *pBuf, int len);

#ifdef __cplusplus 
}
#endif

#endif //__MD_H
