#ifndef __MD_TOOLS_H
#define __MD_TOOLS_H

#include "MDType.h"

int MD_Ip2StrAux(uint8_t *pDes, uint32_t ip);
int MD_Ip2Str(uint8_t *pDes, const sMDIPv4Addr *pIp);
eMDErrCode MD_Str2Ip(sMDIPv4Addr *pIp, const uint8_t *pSrc);

#endif //__MD_TOOLS_H
