/* 
* 文件名称：MDATProc.c
* 摘    要：提供一些通用的AT指令交互函数的实现，这些函数可以被用来向模块发送
*           AT指令并接收模块响应数据
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#include <string.h>
#include <stdlib.h>
#include "MDConfig.h"
#include "MDAtCmd.h"
#include "MDPort.h"
#include "MDType.h"

/************************** 私有定义 *********************************/
#define AT_OK			"OK"
#define AT_ERROR		"ERROR"
#define AT_RTR          ">"     //Ready To Recive
#define CRLF			"\r\n"
#define CR				0x0Du
#define LF				0x0Au

#define IS_AT_RSP_OK(str) (strstr(str, AT_OK))
#define IS_AT_RSP_ERR(str) (strstr(str, AT_ERROR))


/************************** 私有变量  *********************************/
//static uint8_t s_rcvBuf[];

/************************** 函数实现  *********************************/
/*
* 函数功能：调用底层数据发送接口发送AT命令到模块
* 参数说明：
*     [in]pCmd :   要发送的AT命令（必须是 NULL terminated string）； 
*
* 返回值：发送结果：成功、失败(串口写入失败)
*/ 
static eMDErrCode MD_SndATCmd(const uint8_t *pCmd)
{
    uint32_t cmdLen = strlen(pCmd);

    if(cmdLen == MD_WriteBuf(pCmd, cmdLen)){
        return MDE_OK;
    }else{
        return MDE_TTYSERR;
    }
}


/*
* 函数功能：从接收缓存中接收AT指令响应数据
* 参数说明：
*     [out]pRsp  :   指向存储AT响应内容的缓存
*     [in] delay :   等待模块返回完整响应的最长时间(单位：10ms)； 
*     [in] pTail :   特殊响应结尾字符串，如">"，NULL时默认匹配"OK"或"ERROR"
*
* 返回值：获取结果：成功、失败(超时 、接收缓存溢出)
* 注意：当pStr不为NULL时，一匹配到pStr函数便立即结束接收返回
*/ 
static eMDErrCode MD_GetATRsp(sMDAtCmdRsp *pRsp, uint32_t delay, const uint8_t *pTail)
{
    uint32_t waitCnt = 0;   //等待模块响应AT指令计时
    uint16_t rcvIndex = 0;
    uint16_t rcvLen;        //一次读取接收缓存得到的数据长度
    uint16_t remainedSpac;  //剩余接收缓存空间
    uint8_t *pFind;
    uint8_t *pCurFind = pRsp->buf;

    do{
        remainedSpac = MD_RCV_BUF_SIZE-1 - rcvIndex;//缓存剩余空间
        if(remainedSpac > 0){
            rcvLen = MD_ReadBuf(pRsp->buf+rcvIndex, remainedSpac);
        }else{
            return MDE_BUFOVFL;
        }

        if(rcvLen > 0){
            rcvIndex += rcvLen;
            pRsp->buf[rcvIndex] = '\0';

            /*查找特殊字符串*/
            if((NULL != pTail) && strstr(pCurFind, pTail)){
                break;
            }

            /*逐行查找："OK"、"ERROR"*/
            pFind = strstr(pCurFind, CRLF);//查找新行
            if(pFind){
                if(IS_AT_RSP_OK(pCurFind)){
                    pRsp->isPositive = TRUE;
                    break;
                }else if(IS_AT_RSP_ERR(pCurFind)){
                    pRsp->isPositive = FALSE;
                    break;
                }
                pCurFind = pFind + strlen(CRLF);//下次从下一行开始找以提高速度
            }

        }else{
            MD_Delay(10);
            waitCnt++;
        }
    }while(waitCnt <= delay);

    pRsp->len = rcvIndex;

    if(waitCnt <= delay){
        return MDE_OK;
    }else{
        return MDE_TIMEOUT;
    }
}


/*
* 函数功能：向模块发送AT指令，并接收模块响应
*
* 参数说明：
*     [in]pCmd  :   指向要发送的AT指令(NULL terminated!)； 
*     [in]delay :   等待模块返回完整响应的最长时间(单位：s) ； 
*     [out]pRsp :   接收模块指令返回结果
*
* 注意：本函数不对要发送到模块的AT指令的合法性进行检查
* 返回值：成功、失败、指令发送实发、超时 、接收缓存溢出
*/ 
eMDErrCode MD_ATCmdSnd(const uint8_t *pCmd, uint8_t delay, sMDAtCmdRsp *pRsp)
{
    eMDErrCode ret;

    /*发送AT命令到串口上*/
    MD_DEBUG("Snd:%s", pCmd);
    ret = MD_SndATCmd(pCmd);
    if(MDE_OK != ret){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return ret;
    }

    /*接收指令回复*/
    ret = MD_GetATRsp(pRsp, 100*delay, NULL);
    if(MDE_OK == ret){
        MD_DEBUG("Rsp:%s", pRsp->buf);
    }else{
        MD_DEBUG("AT cmd rsp failed!\r\n");
    }

    return ret;
}


/*
* 函数功能：向模块发送AT指令，并接收模块响应，并对响应数据进行处理对模块响应数
*           据的处理有三种方式：
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
eMDErrCode MD_ATCmdSndWithCb(const uint8_t *pCmd, uint8_t delay, ATCmdRspHdl pRspHdl, void *pArg)
{
    eMDErrCode ret;
    sMDAtCmdRsp rsp;

    /*执行AT命令交互*/
    ret = MD_ATCmdSnd(pCmd, delay, &rsp);
    
    /*响应数据解析处理*/
    if(MDE_OK == ret){
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
        return ret;
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
    eMDErrCode ret;
    sMDAtCmdRsp rsp;
    
    /*发送AT命令到串口上*/
    MD_DEBUG("Snd:%s", pCmd);
    ret = MD_SndATCmd(pCmd);
    if(MDE_OK != ret){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return ret;
    }
    
    /*等待模块确认*/
    MD_DEBUG("Rcving '>'...\r\n");
    ret = MD_GetATRsp(&rsp, 100*delay, AT_RTR);
    if(MDE_OK != ret){
        MD_DEBUG("Get '>' faild!\r\n");
        return ret;
    }
    
    /*发送数据*/
    MD_DEBUG("Snd data len:%d\r\n", len);
    if(len != MD_WriteBuf(pData, len)){
        MD_DEBUG("Snd failed!\r\n");
        return MDE_TTYSERR;
    }
    
    /*等待确认*/
    ret = MD_GetATRsp(&rsp, 100*delay, NULL);
    if(MDE_OK == ret){
        if(rsp.isPositive){//收到"OK"
            MD_DEBUG("Data snd succ!\r\n");
            return MDE_OK;
        }else{//收到"ERROR"
            MD_DEBUG("Data snd failed!\r\n");
            return MDE_ERROR;
        }
    }else{
        MD_DEBUG("Data snd failed! %d\r\n", ret);
        return ret;
    }
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
        
        ret = MD_ATCmdSndWithCb(pCmd,
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
eMDErrCode MD_AtGetURCMsg(const uint8_t *pUrc, sMDAtCmdRsp *pRsp, uint8_t delay)
{
    return MD_GetATRsp(pRsp, 100*delay, pUrc);
}


/*
* 函数功能：根据指令返回结果判断模块当前网络状态是否已注册
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


/*
* 函数功能：根据指令返回结果判断模块当前网络附着状态
* 参数说明：
*     pRsp: 传入参数，接收到的AT指令响应
*/ 
eMDErrCode MD_ATCGATT_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t val;
    sMDModem *pMD = (sMDModem *)pArg;

    pFind =  strstr(pRsp->buf, "+CGATT:");
    if(NULL != pFind){
        pFind += strlen("+CGATT:");
        val = atoi(pFind);

        /*更新系统信息*/
        if(NULL != pMD){
            //暂未实现
        }

        if(1 == val){
            return MDE_OK;
        }
    }

    return MDE_ERROR;
}

/*
* 函数功能：根据指令返回结果获取当前信号质量
* 参数说明：
*     pRsp: 传入参数，接收到的AT指令响应
*/ 
eMDErrCode MD_ATCSQ_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t rssi, ber;
    sMDModem *pMD = (sMDModem *)pArg;
    
    /*获取接收信号强度指示值(received signal strength indication)*/
    pFind =  strstr(pRsp->buf, "+CSQ:");
    if(NULL != pFind){
        pFind += strlen("+CSQ:");
        rssi = atoi(pFind);
    }else{
        MD_DEBUG("CSQ rsp error!\r\n");
        return MDE_ERROR;
    }

    /*获取通道误码率(channel bit error rate)*/
    pFind = strstr(pFind, ",");
    if(NULL != pFind){
        pFind += 1;
        ber = atoi(pFind);
    }else{
        MD_DEBUG("CSQ rsp parse failed!\r\n");
        return MDE_ERROR;
    }

    /*更新系统信息*/
    if(NULL != pMD){
        pMD->mdInfo.csq.rssi = rssi;
        pMD->mdInfo.csq.ber = ber;
    }
    
    MD_DEBUG("CSQ update:%d,%d\r\n", rssi, ber);

    return MDE_OK;
}