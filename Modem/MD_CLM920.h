/* 
* �ļ����ƣ�MD_CLM920.h
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

#ifndef __MD_CLM920_H
#define __MD_CLM920_H

#include "MDType.h"

eMDErrCode CLM920_Init(void);
eMDErrCode CLM920_SokctInit(sMDModem *pMdInfo);
eMDErrCode CLM920_SendIpData(uint8_t fd, const sMDSockData *pData);
eMDErrCode CLM920_GetHostByName(const char *pName, sMDIPv4Addr *pAddr);

#endif //__MD_CLM920_H

