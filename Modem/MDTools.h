#ifndef __MD_TOOLS_H
#define __MD_TOOLS_H

int MD_Ip2Str(uint8_t *pDes, uint32_t ip);
eMDErrCode MD_Str2Ip(sMDIPv4Addr *pIp, const uint8_t *pSrc);

#endif //__MD_TOOLS_H
