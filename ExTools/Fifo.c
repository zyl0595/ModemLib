#if FIFO_DEBUG_EN
#include <stdio.h>
#endif
#include "fifo.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#if FIFO_DEBUG_EN
#define FIFO_DEBUG(m)  printf m
#else
#define FIFO_DEBUG(m)
#endif 

#define MIN(a,b) ((a > b)? (b):(a))

//�ɹ�����1 ʧ�ܷ���0 
int FifoBufInit(FIFO_BUF *pBuf, void *pStore, unsigned int size)
{
    #if FIFO_PARAM_CHECK_EN
    if((NULL == pBuf) || (NULL == pStore) || (size < 2)){
        return 0;
    }
    #endif 
    
    pBuf->buf = pStore;
    pBuf->size = size;
    pBuf->r = 0;
    pBuf->w = 0;
    
    return 1;
}

//�ɹ�����1 ʧ�ܷ���0 
int FifoWriteByte(FIFO_BUF *pBuf, unsigned char ch)
{
    #if FIFO_PARAM_CHECK_EN
    if(NULL == pBuf){
        return 0;
    }
    #endif
    
    FIFO_ENTER_CRITICAL
    if(pBuf->w >= pBuf->r){
        if(pBuf->w - pBuf->r != pBuf->size-1){
            pBuf->buf[pBuf->w++] = ch;
            if(pBuf->w == pBuf->size){
                pBuf->w = 0;
            }
            return 1;
        }
    }else{
        if(pBuf->w+1 < pBuf->r){
            pBuf->buf[pBuf->w++] = ch;
            return 1;
        }
    }
    FIFO_EXIT_CRITICAL
    
    FIFO_DEBUG(("FIFO:buffer overflow!\r\n"));
    return 0;
}

//�ɹ�����1 ʧ�ܷ���0 
int FifoReadByte(FIFO_BUF *pBuf, unsigned char *ch)
{
    #if FIFO_PARAM_CHECK_EN
    if((NULL == pBuf) || (NULL == ch)){
        return 0;
    }
    #endif
    
    FIFO_ENTER_CRITICAL
    if(pBuf->r != pBuf->w){
        *ch = pBuf->buf[pBuf->r++];
        if(pBuf->r == pBuf->size){
            pBuf->r = 0;
        }
        return 1;
    }
    FIFO_EXIT_CRITICAL
    
    FIFO_DEBUG(("FIFO:buffer empty!\r\n"));
    return 0;
}


//����FIFO����Ч���ݳ���
unsigned int FifoGetDataLen(FIFO_BUF *pBuf)
{
    unsigned int validLen;

    FIFO_ENTER_CRITICAL
    if(pBuf->r > pBuf->w){
        validLen = pBuf->size - pBuf->r + pBuf->w;
    }else if(pBuf->r < pBuf->w){
        validLen = pBuf->w - pBuf->r;
    }else{
        validLen = 0;
    }
    FIFO_EXIT_CRITICAL

    return validLen;
}

unsigned int FifoGetRestSpace(FIFO_BUF *pBuf)
{
    return (pBuf->size - FifoGetDataLen(pBuf) -1);
}

//�ɹ�����ʵ�ʴ�fifo�ж�ȡ���ֽ��� ������(��������)����-1 
int FifoReadMultByte(FIFO_BUF *pBuf, unsigned char *pDes, unsigned int len)
{
    unsigned int validLen; //fifo�е�ǰ��Ч�����ֽ���
    unsigned int i;

#if FIFO_PARAM_CHECK_EN
    if((NULL == pBuf) || (NULL == pDes) || (0 == len)){
        return -1;
    }
#endif

    
    validLen = FifoGetDataLen(pBuf);
    len = MIN(validLen, len);

    FIFO_ENTER_CRITICAL
    for(i=0;i<len;i++){
        pDes[i] = pBuf->buf[pBuf->r++];
        if(pBuf->r == pBuf->size){
            pBuf->r = 0;
        }
    }
    FIFO_EXIT_CRITICAL

    return len;
}



//��FIFO��д�����ֽڣ�ע�⣺��ʣ��ռ�С��lenʱ����0�Ҳ�ִ���κ�д�룬���򷵻�len
int FifoWriteMultByte(FIFO_BUF *pBuf, unsigned char *pSrc, unsigned int len)
{
    unsigned int i;

    #if FIFO_PARAM_CHECK_EN
    if((NULL == pBuf) || (NULL == pSrc) || (len == 0)){
        return 0;
    }
    #endif 

    if(len > FifoGetRestSpace(pBuf)){
        FIFO_DEBUG(("FIFO:no enough room, multi-byte write break!\r\n"));
        return 0;
    }

    /*ִ��д��*/
    FIFO_ENTER_CRITICAL
    for(i=0;i<len;i++){
        pBuf->buf[pBuf->w++] = pSrc[i];
        if(pBuf->w == pBuf->size){
            pBuf->w = 0;
        }
    }
    FIFO_EXIT_CRITICAL

    return len;
}
