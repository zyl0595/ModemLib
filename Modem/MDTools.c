/* 
* �ļ����ƣ�MDTools.c
* ժ    Ҫ�����ļ�ʵ����һЩͨ�õĹ��ߺ����Թ���ͬ��ģ��ʹ�ã���IP��ַת����У������
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "MDType.h"

/*����32λ�������͵�IP��ַ��ȡ���ַ������ͱ�ʾ*/
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
* �������ܣ������ַ���ʱ�������ĳЩ�ַ������ַ�������ַ�����ַ������
*           ����ATָ������ݽ�����
* ����˵����
*     [in] pSrc  :   ָ��洢AT��Ӧ���ݵĻ���
*     [in] pStr  :   ָ��Ҫ�����������ַ�(",")���ַ���("**"),ע�ⲻ����(',')�� 
*     [in] n     :   �����Ĵ���
*
* ����ֵ������ָ������Ŀ���ַ�������ַ����׵�ַָ��
*/ 
uint8_t *MD_SkipStr(uint8_t *pSrc, const uint8_t *pStr, uint8_t n)
{
    /*
    ���ܾ�����
        pFind = MD_SkipStr("123*abc*456", "*", 2);
        printf("%s", pFind); //��ӡ��: 456
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