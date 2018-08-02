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
* �������ܣ������ַ���ʱ�������ĳЩ�ַ������ַ�������ַ�����ַ������
*           ����ATָ������ݽ�����
* ����˵����
*     [in] pSrc  :   ָ��洢AT��Ӧ���ݵĻ���
*     [in] pStr  :   ָ��Ҫ�����������ַ�(",")���ַ���("**"),ע�ⲻ����(',')�� 
*     [in] n     :   �����Ĵ���
*
* ����ֵ������ָ������Ŀ���ַ�������ַ����׵�ַָ�룬���󷵻�NULL
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

/*
* �������ܣ���һ���ַ����л�ȡ�������׸������������ַ���
* ����˵����
*     [in] pSrc  :   ����Դ
*     [in] maxLen:   ���ջ�����󳤶�
*     [out]pDes  :   ����ҵ��İ����������ַ���
*
* ����ֵ���ҵ��İ����������ַ����ĳ��ȣ�û�ҵ��κΰ��������ַ���0
*
* ע�⣺Դ����ΪNULL��β�ַ����������ַ�����ΪNULL��β�ַ���
*/ 
int MD_GetDecStr(uint8_t *pDes, uint8_t *pSrc, uint16_t maxLen)
{
    uint16_t srcLen;
    uint16_t iF = 0;    //Find index
    uint16_t iW = 0;    //Write index

    /*����ʾ����
        pSrc: "afakarnh\r\n  14197af10751750105sfefn\r\nqwrq"
        ���н����
        pDes: "14197"
        ����ֵ��5 //strlen(pDes)
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
