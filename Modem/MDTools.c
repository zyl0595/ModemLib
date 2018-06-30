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


/*
* 函数功能：解析字符串时获得跳过某些字符或者字符串后的字符串地址，可以
*           用在AT指令返回数据解析中
* 参数说明：
*     [in] pSrc  :   指向存储AT响应内容的缓存
*     [in] pStr  :   指向要跳过的特殊字符(",")或字符串("**"),注意不能是(',')； 
*     [in] n     :   跳过的次数
*
* 返回值：返回指向跳过目标字符串后的字符串首地址指针
*/ 
uint8_t *MD_SkipStr(uint8_t *pSrc, const uint8_t *pStr, uint8_t n)
{
    /*
    功能举例：
        pFind = MD_SkipStr("123*abc*456", "*", 2);
        printf("%s", pFind); //打印出: 456
    */
    uint8_t i;
    uint8_t *pFind = pSrc;
    uint16_t len = strlen(pStr);

    for(i=0;i<n;i++){
        pFind = strstr(pFind, pStr);
        if(NULL == pFind)return NULL;
        pFind += len;
    }
    return pFind;
}