#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "MDType.h"

int MD_Ip2Str(uint8_t *pDes, uint32_t ip)
{
    return sprintf(pDes, "%d.%d.%d.%d", ip>>24, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);
}

eMDErrCode MD_Str2Ip(sMDIPv4Addr *pIp, const uint8_t *pSrc)
{
    const uint8_t *pFind = pSrc;

    pIp->sVal.v4 = atoi(pFind);

    pFind = strstr(pFind, ".");
    if(NULL == pFind){
        return MDE_ERROR;
    }
    pFind ++;
    pIp->sVal.v3 = atoi(pFind);

    pFind = strstr(pFind, ".");
    if(NULL == pFind){
        return MDE_ERROR;
    }
    pFind ++;
    pIp->sVal.v2 = atoi(pFind);

    pFind = strstr(pFind, ".");
    if(NULL == pFind){
        return MDE_ERROR;
    }
    pFind ++;
    pIp->sVal.v1 = atoi(pFind);

    return MDE_OK;
}