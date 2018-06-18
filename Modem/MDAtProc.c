#include <string.h>
#include "MDConfig.h"
#include "MDAtCmd.h"
#include "MDPort.h"
#include "MDType.h"

/************************** 私有宏定义 *********************************/
#define IS_AT_RSP_OK(str) (strstr(str, "OK"))
#define IS_AT_RSP_ERR(str) (strstr(str, "ERROR"))

/*
* 函数功能：向模块发送AT指令，并等待接收模块响应，并对响应数据进行处理
*           对模块响应数据的处理有三种方式：
*           1.在有传入回调函数的情况下，调用回调函数对模块回应数据进行解析处理； 
*           2.在没有设置回调函数，但有传入匹配字符串时，在模块响应数据中匹配指 
*             定字符串，并根据匹配结果返回对应值； 
*           3.在没有设置回调函数和目标匹配字符串时，当判断模块有产生完整回应时，
*             便返回成功，否则返回失败。
* 参数说明：
*     pCmd  :   指向要发送的AT指令(NULL terminated!)； 
*     delay :   等待模块返回完整响应的最长时间(单位：s) ； 
*     pRspHdl:  模块返回结果处理回调函数，当传入了正确的回调函数时，pArg可作为
*               传递给该回调函数的参数； 
*     pArg  :   当 pRspHdl == NULL 时，pArg用作传入模块返回结果字符串匹配目标
*               字符串，不用置为 NULL 
* 注意：本函数不对要发送到模块的AT指令的合法性进行检查 
* 返回值：成功、失败、超时 
*/ 
eMDErrCode MD_ATCmdSnd(const uint8_t *pCmd, uint8_t delay, ATCmdRspHdl pRspHdl, void *pArg)
{
    uint32_t waitCnt = 0;   //等待模块响应AT指令计时 
    uint16_t cmdLen;        //AT指令长度 
    sMDAtCmdRsp rsp;
    uint16_t rcvIndex = 0;
    uint8_t *pFind = rsp.buf;  //指向指令接收过程中检查回应数据起始位置 
    BOOLEAN isEnterRcved = FALSE;

    /*发送AT命令到串口上*/
    cmdLen = strlen(pCmd);
    if(cmdLen != MD_WriteStr(pCmd, cmdLen)){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return MDE_TTYSERR;
    }
    
    MD_DEBUG("AT cmd snd succ!Rcving...\r\n");
    
    /*接收回复指令*/
    do{
        if(MD_ReadByte(&(rsp.buf[rcvIndex])) > 0){
            
            /*检查是否接收到回车换行符，并且是完整响应*/
            if('\r' == rsp.buf[rcvIndex]){
                isEnterRcved = 1;
            }
            if(isEnterRcved && ('\n' == rsp.buf[rcvIndex])){
                rsp.buf[rcvIndex+1] = '\0';   //确保其为 NULL terminated string
                if(IS_AT_RSP_OK(pFind)){
                    rsp.isPositive = 1;
                    break; 
                }else if(IS_AT_RSP_ERR(pFind)){
                    rsp.isPositive = 0;
                    break;
                }else{
                    pFind = &(rsp.buf[rcvIndex]);   //下次查找从这里开始以提高速度 
                    isEnterRcved = 0; 
                }
            }
            rcvIndex ++;
            
            if(rcvIndex >= (sizeof(rsp.buf)-1)){
                MD_DEBUG("AT cmd rcv buf overflow!\r\n");
                return MDE_BUFOVFL; //接收缓存溢出
            }
        }else{
            MD_Delay(10);
            waitCnt++;
        }
    } while (waitCnt <= (delay*100));
    
    /*若响应成功，调用回调函数进行处理，否则返回超时*/
    if(waitCnt <= (delay*100)){
        rsp.len = rcvIndex+1;
        
        MD_DEBUG("AT cmd %s rsp,len:%d\r\n", (rsp.isPositive ? "positive":"negtive"), rcvIndex);
        
        /*可以有三种方式对回应数据进行处理*/ 
        if(NULL != pRspHdl){    //1.有回调函数调用回调函数处理 
            return  pRspHdl(&rsp, pArg);
        }else if(NULL != pArg){ //2.没有回调函数，但有匹配字符串，返回内容匹配结果
            if(strstr(rsp.buf, pArg)){
                return MDE_OK;
            }else{
                return MDE_ERROR;
            }
        }else{                  //3.有完整回应就直接返回
            return MDE_OK;
        }
    }else{
        MD_DEBUG("AT cmd rsp timeout!\r\n");
        return MDE_TIMEOUT;
    }
}


eMDErrCode MD_ATDataSend(const uint8_t *pCmd, const uint8_t *pData, uint16_t len, uint8_t delay)
{
    uint32_t waitCnt = 0;   //等待模块响应AT指令计时 
    uint16_t cmdLen;        //AT指令长度 
    uint16_t rcvIndex = 0;
    uint8_t rcvBuf[MD_RCV_BUF_SIZE];
    uint8_t *pFind = rcvBuf;
    BOOLEAN isEnterRcved = FALSE;
    
    /*发送AT命令到串口上*/
    cmdLen = strlen(pCmd);
    if(cmdLen != MD_WriteStr(pCmd, cmdLen)){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return MDE_TTYSERR;
    }
    
    MD_DEBUG("AT cmd snd succ!Rcving '>'...\r\n");
    do{
        if(MD_ReadByte(rcvBuf) > 0){
            if('>' == rcvBuf[0]){
                break;//succeed! can send data now
            }
        }else{
            MD_Delay(10);
            waitCnt++;
        }
    }while (waitCnt <= (delay*100));
    
    if(waitCnt > (delay*100)){
        MD_DEBUG("AT cmd rsp timeout!\r\n");
        return MDE_TIMEOUT;
    }else{
        waitCnt = 0;
        rcvIndex = 0;
    }
    
    /*发送数据*/
    if(len != MD_WriteStr(pData, len)){
        MD_DEBUG("AT data snd failed!\r\n");
        return MDE_TTYSERR;
    }
    
    MD_DEBUG("AT data snd succ!Rcving...\r\n");
    do{
        if(MD_ReadByte(&rcvBuf[rcvIndex]) > 0){
            
            /*检查是否接收到回车换行符，并且是完整响应*/
            if('\r' == rcvBuf[rcvIndex]){
                isEnterRcved = 1;
            }
            if(isEnterRcved && ('\n' == rcvBuf[rcvIndex])){
                rcvBuf[rcvIndex+1] = '\0';   //确保其为 NULL terminated string
                if(IS_AT_RSP_OK(pFind)){
                    return MDE_OK;      //发送成功 
                }else if(IS_AT_RSP_ERR(pFind)){
                    return MDE_ERROR;   //发送失败 
                }else{
                    pFind = &(rcvBuf[rcvIndex]);   //下次查找从这里开始以提高速度 
                    isEnterRcved = 0;
                }
            }
            rcvIndex ++;
            
            if(rcvIndex >= (sizeof(rcvBuf)-1)){
                MD_DEBUG("AT cmd rcv buf overflow!\r\n");
                return MDE_BUFOVFL; //接收缓存溢出
            }
            
        }else{
            MD_Delay(10);
            waitCnt++;
        }
    }while (waitCnt <= (delay*100));
    
    MD_DEBUG("AT data rsp timeout!\r\n");
    
    return MDE_TIMEOUT;   //发送失败 
}


/*
* 函数功能：按照顺序向模块发送AT指令表内的所有指令， 若有一条指令发送返回失败超
*           最大重发次数，则返回错误，否则返回成功 
* 参数说明：
*     pTable:   指向要发送的AT指令表； 
*     size  :   指令表内指令条数；
* 返回值： 
*/ 
eMDErrCode MD_ATCmdTableSnd(const sMDAtCmdItem *pTable, uint8_t size)
{
    uint8_t buf[100];//动态指令缓存区 
    eMDErrCode ret;
    uint8_t i;
    uint8_t tryCnt = 0;
    const uint8_t *pCmd; 
    
    for(i=0; i<size; ){
        
        pCmd = pTable[i].pCmd;
        
        /*若为动态指令，则生成动态指令（即需要根据运行时变量变更的指令）*/
        if(cmdATCGDCONT == pCmd){//根据当前运营商生成PDP上下文定义指令 
            sprintf(buf, "AT+CGDCONT %s %d\r\n", "123", 15);
            pCmd = buf;
        }
        
        ret = MD_ATCmdSnd(pCmd,
                          pTable[i].delay,
                          pTable[i].rspHdl,
                          pTable[i].pArg);
        tryCnt ++;
                          
        if(MDE_OK == ret){
            i++;
            tryCnt = 0;
        }else{
            if(tryCnt >= pTable[i].tryTms){
                MD_DEBUG("AT cmd:\r\n%sfailed!\r\n", pTable[i].pCmd);
                //return ret;
                i++; tryCnt=0;
            }
        }
    }
    
    return MDE_OK;
}


/*
* 函数功能：根据指令返回结果判断模块当前网络状态 
* 参数说明：
*     pTable:   指向要发送的AT指令表； 
*     size  :   指令表内指令条数； 
*/ 
eMDErrCode MD_ATCGREG_HDL(sMDAtCmdRsp *pRsp, void * arg)
{
    uint8_t *pFind;
    
    pFind =  strstr(pRsp->buf, "+CGREG");
    if(NULL != pFind){
        pFind += 7;
        pFind = strstr(pRsp->buf, ",");
        
        if(NULL != pFind){
            pFind++;
            if('1' == *pFind){//第二个参数为1说明注册成功 
                return MDE_OK;
            }
        }
    }
    
    return MDE_ERROR;
}
