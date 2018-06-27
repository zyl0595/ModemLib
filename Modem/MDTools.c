/* 
* 文件名称：MDTools.c
* 摘    要：本文件实现了一些通用的工具函数以供不同的模块使用，如IP地址转换、校验计算等
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "MDType.h"

/*根据32位整型类型的IP地址获取其字符串类型表示*/
int MD_Ip2StrAux(uint8_t *pDes, uint32_t ip)
{
    return sprintf(pDes, "%d.%d.%d.%d", ip>>24, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);
}

int MD_Ip2Str(uint8_t *pDes, const sMDIPv4Addr *pIp)
{
    return sprintf(pDes, "%d.%d.%d.%d", pIp->sVal.v4, pIp->sVal.v3, pIp->sVal.v2, pIp->sVal.v1);
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