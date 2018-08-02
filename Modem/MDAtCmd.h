/* 
* 文件名称：MDATCmd.h
* 摘    要：一些通用AT指令及响应处理回调函数的声明
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#ifndef __MD_AT_CMD_H
#define __MD_AT_CMD_H

#include "MDType.h"

/********************************** 通用AT命令 **************************************/
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

/********************************** 通用AT命令回调函数 **************************************/
eMDErrCode MD_ATCGREG_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATCREG_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATCGATT_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATCSQ_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATGSN_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode MD_ATCIMI_HDL(sMDAtCmdRsp *pRsp, void *pArg);
eMDNetRegState MD_GetNetRegStatByCode(uint8_t code);

/********************************** 通用非请求结果码处理函数 **************************************/
eMDErrCode MD_DefaultUrcHdl(const uint8_t *pBuf, void *pArg);

#endif //__MD_AT_CMD_H
 
