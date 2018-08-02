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

int MD_Ip2StrAux2(uint8_t *pDes, uint8_t v4, uint8_t v3, uint8_t v2, uint8_t v1)
{
    return sprintf(pDes, "%d.%d.%d.%d", v4, v3, v2, v1);
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
* 返回值：返回指向跳过目标字符串后的字符串首地址指针，错误返回NULL
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

/*
* 函数功能：从一个字符串中获取遇到的首个阿拉伯数字字符串
* 参数说明：
*     [in] pSrc  :   数据源
*     [in] maxLen:   接收缓存最大长度
*     [out]pDes  :   存放找到的阿拉伯数据字符串
*
* 返回值：找到的阿拉伯数据字符串的长度，没找到任何阿拉伯数字返回0
*
* 注意：源必须为NULL结尾字符串，接收字符串不为NULL结尾字符串
*/ 
int MD_GetDecStr(uint8_t *pDes, uint8_t *pSrc, uint16_t maxLen)
{
    uint16_t srcLen;
    uint16_t iF = 0;    //Find index
    uint16_t iW = 0;    //Write index

    /*功能示例：
        pSrc: "afakarnh\r\n  14197af10751750105sfefn\r\nqwrq"
        运行结果：
        pDes: "14197"
        返回值：5 //strlen(pDes)
    */

    srcLen = strlen(pSrc);
    do{
        if(('0' <= pSrc[iF]) && ('9' >= pSrc[iF])){
            pDes[iW] = pSrc[iF];
            iW++;
        }else{
            if(iW)break;
        }
        iF++;
    }while((iW < maxLen-1) && (iF < srcLen));

    pDes[iW] = '\0';//make pDes null terminated string

    return iW;
}
