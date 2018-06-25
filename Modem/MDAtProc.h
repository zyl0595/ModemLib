#ifndef __MD_AT_PROC_H
#define __MD_AT_PROC_H

#include "MDType.h"

#ifndef TABLE_SIZE
#define TABLE_SIZE(table) (sizeof(table)/sizeof(table[0]))
#endif

/********************************** 通用AT命令操作方法 **************************************/
eMDErrCode MD_ATCmdSnd(const uint8_t *pCmd, uint8_t delay, ATCmdRspHdl pRspHdl, void *pArg);
eMDErrCode MD_ATCmdTableSnd(const sMDAtCmdItem *pTable, uint8_t size);
eMDErrCode MD_ATDataSend(const uint8_t *pCmd, const uint8_t *pData, uint16_t len, uint8_t delay);
eMDErrCode MD_GetURCMsg(const uint8_t *pUrc, sMDAtCmdRsp *pRsp, uint8_t delay);

/********************************** 通用AT命令回调函数 **************************************/
eMDErrCode MD_ATCGREG_HDL(sMDAtCmdRsp *pRsp, void *pArg);
 
#endif //__MD_AT_PROC_H

