/* 
* 文件名称：MDTools.h
* 摘    要：本文件声明了一些通用的工具函数以供不同的模块使用
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/


#ifndef __MD_TOOLS_H
#define __MD_TOOLS_H

#include "MDType.h"

int MD_Ip2StrAux(uint8_t *pDes, uint32_t ip);
int MD_Ip2Str(uint8_t *pDes, const sMDIPv4Addr *pIp);
eMDErrCode MD_Str2Ip(sMDIPv4Addr *pIp, const uint8_t *pSrc);

#endif //__MD_TOOLS_H
