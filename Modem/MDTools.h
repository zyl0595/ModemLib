/* 
* �ļ����ƣ�MDTools.h
* ժ    Ҫ�����ļ�������һЩͨ�õĹ��ߺ����Թ���ͬ��ģ��ʹ��
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/


#ifndef __MD_TOOLS_H
#define __MD_TOOLS_H

#include "MDType.h"

int MD_Ip2Str(uint8_t *pDes, const sMDIPv4Addr *pIp);
int MD_Ip2StrAux(uint8_t *pDes, uint32_t ip);
int MD_Ip2StrAux2(uint8_t *pDes, uint8_t v4, uint8_t v3, uint8_t v2, uint8_t v1);
eMDErrCode MD_Str2Ip(sMDIPv4Addr *pIp, const uint8_t *pSrc);
uint8_t *MD_SkipStr(uint8_t *pSrc, const uint8_t *pStr, uint8_t n);
int MD_GetDecStr(uint8_t *pDes, uint8_t *pSrc, uint16_t maxLen);

#endif //__MD_TOOLS_H
