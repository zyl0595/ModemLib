/* 
* �ļ����ƣ�MDATProc.h
* ժ    Ҫ���ṩһЩͨ�õ�ATָ��������Ķ��壬��Щ�������Ա�������ģ�鷢��
*           ATָ�����ģ����Ӧ����
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#ifndef __MD_AT_PROC_H
#define __MD_AT_PROC_H

#include "MDType.h"

#ifndef TABLE_SIZE
#define TABLE_SIZE(table) (sizeof(table)/sizeof(table[0]))
#endif

/********************************** ͨ��AT����������� **************************************/
eMDErrCode MD_ATSetATRcvBuf(uint8_t *addr, uint32_t size);
eMDErrCode MD_ATCmdSnd(const uint8_t *pCmd, uint8_t delay, sMDAtCmdRsp *pRsp);
eMDErrCode MD_ATCmdSndRich(const uint8_t *pCmd, uint8_t delay, ATCmdRspHdl pRspHdl, void *pArg);
eMDErrCode MD_ATCmdTableSnd(const sMDAtCmdItem *pTable, uint8_t size);
eMDErrCode MD_ATDataSend(const uint8_t *pCmd, const uint8_t *pData, uint16_t len, uint8_t delay);
eMDErrCode MD_ATDataSendRich(sMDAtCmdRsp *pRsp, const uint8_t *pCmd, const uint8_t *pData, uint16_t len, uint8_t delay, const uint8_t *pTail);
eMDErrCode MD_AtGetURCMsg(const uint8_t *pUrc, sMDAtCmdRsp *pRsp, uint32_t delay);
eMDErrCode MD_ATRcvData(uint8_t *pDes, int *pRdLen, int maxLen, uint32_t delay);
eMDErrCode MD_ATURCProc(const sMDURCHdlItem *urcHdlTable, uint32_t delay);
 
#endif //__MD_AT_PROC_H

