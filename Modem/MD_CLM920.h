#ifndef __MD_CLM920_H
#define __MD_CLM920_H

#include "MDType.h"

eMDErrCode CLM920_Init(void);
eMDErrCode CLM920_SendIpData(uint8_t fd, const sMDTcpData *pData);

#endif //__MD_CLM920_H

