/* 
* �ļ����ƣ�MDATCmd.h
* ժ    Ҫ��һЩͨ��ATָ���Ӧ����ص�����������
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#ifndef __MD_AT_CMD_H
#define __MD_AT_CMD_H

#include "MDType.h"

/********************************** ͨ��AT���� **************************************/
extern const unsigned char cmdATZ[];
extern const unsigned char cmdATE0[];
extern const unsigned char cmdATE1[];
extern const unsigned char cmdATI[];
extern const unsigned char cmdATGSN[];
extern const unsigned char cmdATCPIN[];
extern const unsigned char cmdATCIMI[];
extern const unsigned char cmdATCSQ[];
extern const unsigned char cmdATCGDCONT[];
extern const unsigned char cmdATCGATT[];
extern const unsigned char cmdATCGACT[];
extern const unsigned char cmdATCREG[];
extern const unsigned char cmdATCGREG[];

extern const unsigned char urcIPNETOPEN[];

/********************************** ͨ��AT����ص����� **************************************/
eMDErrCode MD_ATCGREG_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATCREG_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATCGATT_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATCSQ_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATGSN_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATCIMI_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDNetRegState MD_GetNetRegStatByCode(uint8_t code);

/********************************** ͨ�÷��������봦���� **************************************/
eMDErrCode MD_DefaultUrcHdl(const uint8_t *pBuf, void *pArg);

#endif //__MD_AT_CMD_H
 
