/* 
* 文件名称：MDATProc.h
* 摘    要：提供一些通用的AT指令交互函数的定义，这些函数可以被用来向模块发送
*           AT指令并接收模块响应数据
*  
* 作    者：
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

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
eMDErrCode MD_AtGetURCMsg(const uint8_t *pUrc, sMDAtCmdRsp *pRsp, uint8_t delay);

/********************************** 通用AT命令回调函数 **************************************/
eMDErrCode MD_ATCGREG_HDL(sMDAtCmdRsp *pRsp, void *pArg);
 
#endif //__MD_AT_PROC_H

