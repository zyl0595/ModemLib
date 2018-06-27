/* 
* 文件名称：MD_CLM920.h
* 摘    要：
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#ifndef __MD_CLM920_H
#define __MD_CLM920_H

#include "MDType.h"

eMDErrCode CLM920_Init(void);
eMDErrCode CLM920_SokctInit(sMDModem *pMdInfo);
eMDErrCode CLM920_SendIpData(uint8_t fd, const sMDSockData *pData);
eMDErrCode CLM920_GetHostByName(const char *pName, sMDIPv4Addr *pAddr);

#endif //__MD_CLM920_H

