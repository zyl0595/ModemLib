#include <string.h>
#include "MDConfig.h"
#include "MDAtCmd.h"
#include "MDPort.h"
#include "MDType.h"

/************************** 私有定义 *********************************/
#define IS_AT_RSP_OK(str) (strstr(str, "OK"))
#define IS_AT_RSP_ERR(str) (strstr(str, "ERROR"))



/************************** 私有变量  *********************************/
//static uint8_t s_rcvBuf[];


/************************** 函数实现  *********************************/
/*
* 函数功能：向模块发送AT指令，并等待接收模块响应，并对响应数据进行处理
*           对模块响应数据的处理有三种方式：
*           1.在有传入回调函数的情况下，调用回调函数对模块回应数据进行解析处理； 
*           2.在没有设置回调函数，但有传入匹配字符串时，在模块响应数据中匹配指 
*             定字符串，并根据匹配结果返回对应值； 
*           3.在没有设置回调函数和目标匹配字符串时，当判断模块有产生完整回应时，
*             便返回成功，否则返回失败。
* 参数说明：
*     [in]pCmd  :   指向要发送的AT指令(NULL terminated!)； 
*     [in]delay :   等待模块返回完整响应的最长时间(单位：s) ； 
*     [in]pRspHdl:  模块返回结果处理回调函数
*     [in]pArg  :   当(pRspHdl != NULL)时作为pRspHdl参数，当(pRspHdl == NULL)时，
*                   pArg用作返回结果匹配目标字符串，为NULL时默认比较"OK"或者"ERROR"
*
* 注意：本函数不对要发送到模块的AT指令的合法性进行检查
* 返回值：成功、失败、指令发送实发、超时 、接收缓存溢出
*/ 
eMDErrCode MD_ATCmdSnd(const uint8_t *pCmd, uint8_t delay, ATCmdRspHdl pRspHdl, void *pArg)
{
    uint32_t waitCnt = 0;   //等待模块响应AT指令计时 
    uint16_t cmdLen;        //AT指令长度 
    sMDAtCmdRsp rsp;
    uint16_t rcvIndex = 0;
    uint16_t rcvLen;
    uint8_t *pFind = rsp.buf;  //指向指令接收过程中检查回应数据起始位置 
    BOOLEAN isStrRcved = FALSE;

    /*发送AT命令到串口上*/
    MD_DEBUG("Snd:%s", pCmd);
    cmdLen = strlen(pCmd);
    if(cmdLen != MD_WriteBuf(pCmd, cmdLen)){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return MDE_TTYSERR;
    }
    
    /*接收回复指令*/
    do{
        rcvLen = MD_ReadBuf(rsp.buf+rcvIndex, sizeof(rsp.buf)-1 - rcvIndex);
        if(rcvLen > 0){
            rcvIndex += rcvLen;
            rsp.buf[rcvIndex] = '\0';

            /*检查是否接收到回车换行符，并且是完整响应*/
            if(strstr(rsp.buf, "\r\n")){

                /*匹配指令返回结果看是否接收到完整响应*/
                if(IS_AT_RSP_OK(pFind)){
                    rsp.isPositive = TRUE;
                    break; 
                }else if(IS_AT_RSP_ERR(pFind)){
                    rsp.isPositive = FALSE;
                    break;
                }else if((FALSE == isStrRcved) && (NULL == pRspHdl) && (NULL != pArg) && (strstr(pFind, pArg))){
                    //MD_DEBUG("String rcved!\r\n");
                    isStrRcved = TRUE;
                }
            }
            
            if(rcvIndex >= (sizeof(rsp.buf)-1)){
                MD_DEBUG("AT cmd rcv buf overflow!\r\n");
                return MDE_BUFOVFL; //接收缓存溢出
            }
        }else{
            if(TRUE == isStrRcved){
                /*即使匹配到也继续接收以完整接收整个响应信息（直到收到"OK"）或者超时*/
                if((TRUE == rsp.isPositive) || (waitCnt >= 10)){ //10*10ms
                    break;
                }
            }

            MD_Delay(10);
            waitCnt++;
        }
    } while (waitCnt <= (delay*100u));
    
    /*若响应成功，调用回调函数进行处理，否则返回超时*/
    if(waitCnt <= (delay*100u)){
        rsp.len = rcvIndex;
        
        MD_DEBUG("Rsp:%s", rsp.buf);
        
        /*可以有三种方式对回应数据进行处理*/ 
        if(NULL != pRspHdl){    //1.有回调函数调用回调函数处理 
            return  pRspHdl(&rsp, pArg);
        }else if(NULL != pArg){ //2.没有回调函数，但有匹配字符串，返回内容匹配结果
            if(strstr(rsp.buf, pArg)){
                return MDE_OK;
            }else{
                return MDE_ERROR;
            }
        }else{                  //3.有完整回应就直接返回结果
            if(TRUE == rsp.isPositive){
                return MDE_OK;
            }else{
                return MDE_ERROR;
            }
            
        }
    }else{
        MD_DEBUG("AT cmd timeout!\r\n");
        return MDE_TIMEOUT;
    }
}


/*
* 函数功能：执行用于发送数据的AT指令并发送数据，如发送短信、发送TCP数据报
* 
* 参数说明：
*   [in]pTable: 指向要发送的AT指令表； 
*   [in]pData : 指向要通过该条指令发送的数据；
*   [in]len :   要发送数据的长度
*   [in]delay:  等待模块返回完整响应的最长时间(单位：s) 
* 返回值：  成功、失败、指令发送实发、超时 、接收缓存溢出
*/ 
eMDErrCode MD_ATDataSend(const uint8_t *pCmd, const uint8_t *pData, uint16_t len, uint8_t delay)
{
    uint32_t waitCnt = 0;   //等待模块响应AT指令计时 
    uint16_t cmdLen;        //AT指令长度 
    uint16_t rcvIndex = 0;
    uint16_t rcvLen;
    uint8_t rcvBuf[MD_RCV_BUF_SIZE];
    uint8_t *pFind = rcvBuf;
    BOOLEAN isEnterRcved = FALSE;
    
    /*发送AT命令到串口上*/
    MD_DEBUG("Snd:%s", pCmd);
    cmdLen = strlen(pCmd);
    if(cmdLen != MD_WriteBuf(pCmd, cmdLen)){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return MDE_TTYSERR;
    }
    
    MD_DEBUG("Rcving '>'...\r\n");
    do{
        rcvLen = MD_ReadBuf(rcvBuf+rcvIndex, sizeof(rcvBuf)-1 - rcvIndex);
        if(rcvLen > 0){
            rcvIndex += rcvLen;
            rcvBuf[rcvIndex] = '\0';    //确保其为 NULL terminated string

            if(strstr(rcvBuf, ">")){
                break;//succeed! can send data now
            }else if(IS_AT_RSP_ERR(pFind)){
                MD_DEBUG("rcv:%s", rcvBuf);
                return MDE_ERROR;   //发送失败 
            }

            if(rcvIndex >= (sizeof(rcvBuf)-1)){
                MD_DEBUG("AT cmd rcv buf overflow!\r\n");
                return MDE_BUFOVFL; //接收缓存溢出
            }
        }else{
            MD_Delay(10);
            waitCnt++;
        }
    }while (waitCnt <= (delay*100u));
    
    if(waitCnt > (delay*100u)){
        MD_DEBUG("AT cmd timeout!\r\n");
        return MDE_TIMEOUT;
    }else{
        waitCnt = 0;
        rcvIndex = 0;
    }
    
    /*发送数据*/
    MD_DEBUG("Snd data len:%d\r\n", len);
    if(len != MD_WriteBuf(pData, len)){
        MD_DEBUG("Snd failed!\r\n");
        return MDE_TTYSERR;
    }
    
    do{
        rcvLen = MD_ReadBuf(rcvBuf+rcvIndex, sizeof(rcvBuf)-1 - rcvIndex);
        if(rcvLen > 0){
            rcvIndex += rcvLen;
            rcvBuf[rcvIndex] = '\0';
            
            /*检查是否接收到回车换行符，并且是完整响应*/
            if(strstr(rcvBuf, "\r\n")){

                if(IS_AT_RSP_OK(pFind)){
                    MD_DEBUG("Data snd succ!\r\n");
                    return MDE_OK;      //发送成功 
                }else if(IS_AT_RSP_ERR(pFind)){
                    MD_DEBUG("Data snd failed!\r\n");
                    return MDE_ERROR;   //发送失败
                }
            }

            if(rcvIndex >= (sizeof(rcvBuf)-1)){
                MD_DEBUG("AT cmd rcv buf overflow!\r\n");
                return MDE_BUFOVFL; //接收缓存溢出
            }
            
        }else{
            MD_Delay(10);
            waitCnt++;
        }
    }while (waitCnt <= (delay*100u));
    
    MD_DEBUG("Data rsp timeout!\r\n");
    return MDE_TIMEOUT;   //发送失败 
}


/*
* 函数功能：按照顺序向模块发送AT指令表内的所有指令， 若有一条指令发送返回失败超
*           最大重发次数，则返回错误，否则返回成功 
* 参数说明：
*     [in]pTable:   指向要发送的AT指令表； 
*     [in]size  :   指令表内指令条数；
* 返回值： 成功、失败、指令发送实发、超时 、接收缓存溢出
*/ 
eMDErrCode MD_ATCmdTableSnd(const sMDAtCmdItem *pTable, uint8_t size)
{
    //uint8_t buf[100];//动态指令缓存区 
    eMDErrCode ret;
    uint8_t i;
    uint8_t tryCnt = 0;
    const uint8_t *pCmd; 
    
    for(i=0; i<size; ){
        
        pCmd = pTable[i].pCmd;
        
        /*若为动态指令，则生成动态指令（即需要根据运行时变量变更的指令）*/
        //if(cmdATCGDCONT == pCmd){//根据当前运营商生成PDP上下文定义指令 
        //    sprintf(buf, "AT+CGDCONT %s %d\r\n", "123", 15);
        //    pCmd = buf;
        //}
        
        ret = MD_ATCmdSnd(pCmd,
                          pTable[i].delay,
                          pTable[i].rspHdl,
                          pTable[i].pArg);
        tryCnt ++;
                          
        if(MDE_OK == ret){
            i++;
            tryCnt = 0;
            MD_Delay(MD_ATCMD_ITV);
        }else{
            if(tryCnt >= pTable[i].tryTms){
                MD_DEBUG("AT cmd falied:\r\n----\r\n%s----\r\n", pTable[i].pCmd);
                return ret;
                //i++; tryCnt=0; //测试时打开，即使一条指令错误也可以继续下一条
            }else{
                MD_Delay(MD_ATCMD_ITV_WHENERR);//稍作等待再进行重发
            }
        }
    }
    
    return MDE_OK;
}


/*
* 函数功能：接收模块的非请求结果响应
* 参数说明：
*     [out]pRsp:    存放非请求结果响应； 
*     [in]delay:    接收最大等待时间(s)；
* 返回值： 成功、失败、指令发送实发、超时 、接收缓存溢出
*/ 
eMDErrCode MD_GetURCMsg(const uint8_t *pUrc, sMDAtCmdRsp *pRsp, uint8_t delay)
{
    uint32_t waitCnt = 0;
    uint16_t rcvIndex = 0;
    uint16_t rcvLen;
    BOOLEAN isStrRcved = FALSE;

    do 
    {
        rcvLen = MD_ReadBuf(pRsp->buf+rcvIndex, sizeof(pRsp->buf)-1 - rcvIndex);
        if(rcvLen > 0){
            rcvIndex += rcvLen;
            pRsp->buf[rcvIndex] = '\0';

            if(FALSE == isStrRcved){
                if(strstr(pRsp->buf, "\r\n")){
                    if(strstr(pRsp->buf, pUrc)){
                        isStrRcved = TRUE;//字符串匹配成功
                    }
                }
            }

            if(rcvIndex >= (sizeof(pRsp->buf)-1)){
                if(isStrRcved){
                    return MDE_OK;//接收成功
                }else{
                    return MDE_BUFOVFL; //接收缓存溢出
                }
            }
        }else{
            if((isStrRcved) && (waitCnt > 10)){
                pRsp->buf[rcvIndex] = '\0'; 
                pRsp->isPositive = TRUE;
                pRsp->len = rcvIndex;
                return MDE_OK;//接收成功
            }

            MD_Delay(10);
            waitCnt++;
        }

    }while(waitCnt <= (delay*100u));

    return MDE_TIMEOUT;   //接收超时
}


/*
* 函数功能：根据指令返回结果判断模块当前网络状态 
* 参数说明：
*     pRsp: 传入参数，接收到的AT指令响应
*/ 
eMDErrCode MD_ATCGREG_HDL(sMDAtCmdRsp *pRsp, void *pArg)
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
