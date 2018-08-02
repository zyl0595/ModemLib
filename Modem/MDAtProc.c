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
#include "MDConfig.h"
#include "MDPort.h"
#include "MDType.h"

#if MD_OS_ENABLE
#include "ucos_ii.h"
#endif

/************************** 私有定义 *********************************/
#define AT_OK			"OK\r\n"
#define AT_ERROR		"ERROR\r\n"
#define AT_RTR          ">"     //Ready To Recive
#define CRLF			"\r\n"
#define CR				'\r'
#define LF				'\n'
#define RTR             '>'

#define IS_AT_RSP_OK(str) (str == strstr(str, AT_OK))
#define IS_AT_RSP_ERR(str) (str == strstr(str, AT_ERROR))

/************************** 私有变量  *********************************/

/*接收缓存*/
uint8_t s_ATRcvBuf[MD_RCV_BUF_SIZE];    /*AT指令交互接收缓存*/
#if MD_OS_ENABLE
OS_EVENT s_ATRcvBufLock;
#endif

/************************** 全局变量  *********************************/
sMDIPData g_ipRcvData = {
    FALSE,
    0,
    {0},
    {s_ATRcvBuf, MD_RCV_BUF_SIZE, 0}//默认使用AT指令交互接收缓存
};   //TCP/IP数据接收存储（必须进行初始化给pBuf赋值使其拥有真正的存储空间）




/************************** 函数实现  *********************************/

/*
* 函数功能：设置用于接收模块数据（如从模块接收TCP数据）的缓冲区，如果用户
*           没有调用进行设置，则默认使用AT指令交互时用于接收交互数据的缓存
*           （该缓存大小一般比较小，且只要进行AT指令交互，该缓存就会被覆盖，
*           故强烈建议设置一块单独的缓存用于接收TCP/IP数据）
*
*     注意：需要在模块初始化调用之前就调用该函数设置才有效
*
* 参数说明：
*     [in]addr :   指向接收缓存的地址； 
*     [in]size :   接收缓存的大小；
*
* 返回值：发送结果：成功
*/ 
eMDErrCode MD_ATSetATRcvBuf(uint8_t *addr, uint32_t size)
{
    if((NULL != addr) || (0 != size)){
        g_ipRcvData.rcvData.pBuf = addr;
        g_ipRcvData.rcvData.bufSize = size;
        MD_DEBUG("Using custom rcv buf, size:%d\r\n", size);
    }else{
        g_ipRcvData.rcvData.pBuf = s_ATRcvBuf;
        g_ipRcvData.rcvData.bufSize = MD_RCV_BUF_SIZE;
        MD_DEBUG("Using default rcv buf, size:%d\r\n", MD_RCV_BUF_SIZE);
    }
    return MDE_OK;
}

#if 0 //这两个函数用于对AT指令接收缓存进行读写操作，暂时没有用 20180720
eMDErrCode MD_WriteATRcvBuf(uint32_t pos, uint8_t *pSrc, uint32_t len)
{
    uint32_t i;

    for(i=0; i< len; i++,pos++){
        if(pos < sizeof(s_ATRcvBuf)){
            s_ATRcvBuf[pos] = pSrc[i];
        }else{
            return MDE_BUFOVFL;
        }
    }

    return MDE_OK;
}

eMDErrCode MD_ReadATRcvBuf(uint32_t pos, uint8_t *pDes, uint32_t len)
{
    uint32_t i;

    for(i=0; i< len; i++,pos++){
        if(pos < sizeof(s_ATRcvBuf)){
            pDes[i] = s_ATRcvBuf[pos];
        }else{
            return MDE_BUFOVFL;
        }
    }

    return MDE_OK;
}
#endif

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
* 函数功能：从接收缓存中接收AT指令响应数据，
* 参数说明：
*     [out]pRsp  :   指向存储AT响应内容的缓存
*     [in] delay :   等待模块返回完整响应的最长时间(单位：ms)； 
*     [in] pTail :   特殊响应结尾字符串，如">"，NULL时默认匹配"OK"或"ERROR"
*     [in] isCfn :   TRUE时需要接收到"OK"或"ERROR"才返回，FLASH时只要匹配到pTail就返回
*
* 返回值：获取结果：成功("OK"或"ERROR")、超时、接收缓存溢出
*
* 注意：当pStr不为NULL时，一匹配到pStr函数便立即结束接收返回
*/ 
static eMDErrCode MD_GetATRsp(sMDAtCmdRsp *pRsp, uint32_t delay, const uint8_t *pTail, bool isCfn)
{
    uint32_t waitCnt = 0;   //等待模块响应AT指令计时
    uint16_t rcvIndex = 0;
    uint16_t rcvLen;        //一次读取接收缓存得到的数据长度
    uint8_t *pCurFind;
    bool isTailFind = FALSE;//响应匹配字符串是否已找到
    
    pRsp->buf = s_ATRcvBuf;
    pCurFind = pRsp->buf;

    do{
        if(rcvIndex < MD_RCV_BUF_SIZE-1){
            rcvLen = MD_ReadByte(pRsp->buf+rcvIndex);
        }else{
            return MDE_BUFOVFL;
        }

        if(rcvLen > 0){
            
            if(LF == pRsp->buf[rcvIndex]){
                if(CR == pRsp->buf[rcvIndex-1]){//接收到完整的一行
                    pRsp->buf[rcvIndex+1] = '\0';

                    /*查找特殊字符串*/
                    if((NULL != pTail) && (FALSE == isTailFind) && strstr(pCurFind, pTail)){
                        isTailFind = TRUE;
                        if(!isCfn)break;//马上退出，不继续接收"OK"或者"ERROR"
                    }

                    /*逐行查找："OK"、"ERROR"*/
                    if(IS_AT_RSP_OK(pCurFind)){
                        pRsp->isPositive = TRUE;
                        break;
                    }else if(IS_AT_RSP_ERR(pCurFind)){
                        pRsp->isPositive = FALSE;
                        break;
                    }
                    pCurFind = &(pRsp->buf[rcvIndex+1]);//下次从下一行开始找以提高速度
                }
            }else if(RTR == pRsp->buf[rcvIndex]){
                if(0 == strcmp(AT_RTR, pTail)){
                    pRsp->buf[rcvIndex+1] = '\0';
                    isTailFind = TRUE;
                    break;//'>'后不再返回"OK"，故直接退出接收
                }
            }

            rcvIndex ++;
        }else{
            if(delay)MD_Delay(1);
            waitCnt++;
        }
    }while(waitCnt <= delay);

    pRsp->len = rcvIndex;

    if(waitCnt <= delay){

        if(NULL != pTail){
            /*在需要匹配特殊返回字符串的情况下，如果因提前收到"OK"或"ERROR"而结束，需要根据
            是否有收到目标字符串来返回结果*/
            if(TRUE == isTailFind){
                return MDE_OK;
            }else{
                return MDE_ERROR;
            }
        }else{
            return MDE_OK;
        }

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
    MD_DEBUG_AT("Snd:%s", pCmd);
    ret = MD_SndATCmd(pCmd);
    if(MDE_OK != ret){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return ret;
    }

    /*接收指令回复*/
    ret = MD_GetATRsp(pRsp, 1000*delay, NULL, TRUE);
    if(MDE_OK == ret){
        MD_DEBUG_AT("Rsp:%s", pRsp->buf);
    }else if(MDE_TIMEOUT == ret){
        MD_DEBUG("AT cmd rsp timeout!\r\n");
    }else if(MDE_BUFOVFL == ret){
        MD_DEBUG("AT cmd rsp overflow!\r\n");
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
*             根据返回"OK"还是"ERROR"返回结果。
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
eMDErrCode MD_ATCmdSndRich(const uint8_t *pCmd, uint8_t delay, ATCmdRspHdl pRspHdl, void *pArg)
{
    eMDErrCode ret;
    sMDAtCmdRsp rsp;

    /*参数检查*/
    if(NULL == pCmd)return MDE_PARAM_ERR;

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
        }else{                  //3.有完整回应根据返回信息返回结果
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
*   [out]pRsp : AT指令响应数据接收缓存；
*   [in]pTable: 指向要发送的AT指令表； 
*   [in]pData : 指向要通过该条指令发送的数据；
*   [in]len :   要发送数据的长度
*   [in]delay:  等待模块返回完整响应的最长时间(单位：s) 
*   [in]pTail:  特殊匹配字符
* 返回值：  成功、失败、指令发送实发、超时 、接收缓存溢出
*/ 
eMDErrCode MD_ATDataSendRich(sMDAtCmdRsp *pRsp, const uint8_t *pCmd, const uint8_t *pData, uint16_t len, uint8_t delay, const uint8_t *pTail)
{
    eMDErrCode ret;

    /*发送AT命令到串口上*/
    MD_DEBUG_AT("Snd:%s", pCmd);
    ret = MD_SndATCmd(pCmd);
    if(MDE_OK != ret){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return ret;
    }

    /*等待模块确认*/
    //MD_DEBUG("Rcving '>'...\r\n");
    ret = MD_GetATRsp(pRsp, 1000*delay, AT_RTR, FALSE);
    MD_DEBUG_AT("Rsp:%s\r\n", pRsp->buf);
    if(MDE_OK != ret){
        MD_DEBUG("Snd faild!\r\n");
        return ret;
    }

    /*发送数据*/
    MD_DEBUG_AT("Snd data len:%d\r\n", len);
    if(len != MD_WriteBuf(pData, len)){
        MD_DEBUG("Data snd failed!\r\n");
        return MDE_TTYSERR;
    }

    /*等待确认*/
    ret = MD_GetATRsp(pRsp, 1000*delay, pTail, TRUE);
    if(MDE_OK == ret){
        MD_DEBUG_AT("Rsp:%s", pRsp->buf);
        if(TRUE == pRsp->isPositive){//收到"OK"
            return MDE_OK;
        }else{//收到"ERROR"
            MD_DEBUG("Data snd failed!\r\n");
            return MDE_ERROR;
        }
    }else if(MDE_TIMEOUT == ret){
        MD_DEBUG("Data snd timeout!\r\n");
    }else{
        MD_DEBUG("Data snd failed! %d\r\n", ret);
    }

    return ret;
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
    sMDAtCmdRsp rsp;
    return MD_ATDataSendRich(&rsp, pCmd, pData, len, delay, NULL);
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
        
        ret = MD_ATCmdSndRich(pCmd,
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
*     [in]delay:    接收最大等待时间(ms)；
* 返回值： 成功、超时 、接收缓存溢出
*/ 
eMDErrCode MD_AtGetURCMsg(const uint8_t *pUrc, sMDAtCmdRsp *pRsp, uint32_t delay)
{
    eMDErrCode ret;

    ret = MD_GetATRsp(pRsp, delay, pUrc, FALSE);//FALSE: URC 数据不带"OK" "ERROR"
    if(MDE_OK == ret){
        MD_DEBUG_AT("Urc:%s", pRsp->buf);
    }
    return ret;
}

/*
* 函数功能：从AT指令接收缓存中接收指定长度数据
* 参数说明：
*     [out]pRsp:    存放非请求结果响应； 
*     [out]pRdLen:  实际从接收缓存中读取出来的数据长度；
*     [in]delay:    接收最大等待时间(ms)；
* 返回值： 成功、超时
*/ 
eMDErrCode MD_ATRcvData(uint8_t *pDes, int *pRdLen, int maxLen, uint32_t delay)
{
    int curReadLen;
    int index = 0;
    uint32_t waitCnt = 0;

    do{
        curReadLen = MD_ReadBuf(pDes+index, maxLen-index);
        if(curReadLen > 0){
            index += curReadLen;
        }else{
            if(delay)MD_Delay(1);
            waitCnt++;
        }
    }while((waitCnt < delay) && (index < maxLen));

    *pRdLen = index;

    if(waitCnt < delay){
        return MDE_OK;
    }else{
        return MDE_TIMEOUT;
    }
}


/*
* 函数功能：检查接收缓存收取非请求结果码(URC)，并根据URC内容调用响应的处理
*           函数进行处理
* 参数说明：
*     [in]urcHdlTable:      URC处理函数表； 
*     [in]delay:接收等待时间，当接收到缓冲区中有数据时，但数据不完整，则进
*               行适当的等待以完整接收整个URC内容，以接收到"\r\n"为判断依据；
*
* 返回值： 成功、超时、接收缓存溢出
*/ 
eMDErrCode MD_ATURCProc(const sMDURCHdlItem *urcHdlTable, uint32_t delay)
{
    eMDErrCode ret = MDE_OK;
    uint32_t waitCnt = 0;   //等待直到接收到完整("\r\n")一条URC数据的时间
    uint16_t rcvIndex = 0;
    uint16_t rcvLen = 0;

    //AT rcv buf lock

    /*接收URC*/
    do{
        if(rcvIndex < MD_RCV_BUF_SIZE-1){
            rcvLen = MD_ReadByte(s_ATRcvBuf+rcvIndex);
        }else{
            ret = MDE_BUFOVFL;
            break;
        }

        if(rcvLen > 0){

            /*检查如果接收到新行，则对新行数据进行处理，否则继续接收*/
            if(LF == s_ATRcvBuf[rcvIndex]){
                if(CR == s_ATRcvBuf[rcvIndex-1]){//接收到完整的一行
                    uint16_t tableIndex;
                    s_ATRcvBuf[rcvIndex+1] = '\0';
                    
                    if(rcvIndex+1 > 2){//收到的数据不止是"\r\n"
                        /*遍历URC处理列表找到合适的函数处理次URC数据*/
                        for(tableIndex = 0; ;tableIndex++){
                            if(NULL != urcHdlTable[tableIndex].pURC){
                                if(strstr(s_ATRcvBuf, urcHdlTable[tableIndex].pURC)){
                                    ret = urcHdlTable[tableIndex].handle(s_ATRcvBuf, urcHdlTable[tableIndex].pArg);
                                    break;
                                }
                                //else{//调试专用
                                //    MD_DEBUG("\r\n--------%d--------+++++\r\n", tableIndex);
                                //    MD_DEBUG("%s", s_ATRcvBuf);
                                //    MD_DEBUG("\r\n---\r\n");
                                //    MD_DEBUG("%s", urcHdlTable[tableIndex].pURC);
                                //    MD_DEBUG("\r\n--------end------+++++\r\n");
                                //}
                            }else{
                                MD_DEBUG("UnRegisted URC len:%d Content:\r\n%s", rcvIndex+1, s_ATRcvBuf);
                                ret = urcHdlTable[tableIndex].handle(s_ATRcvBuf, urcHdlTable[tableIndex].pArg);
                                break;
                            }
                        }
                    }

                    /*处理完毕，可以退出了*/
                    break;//do{}while()
                }
            }

            rcvIndex ++;
        }else{
            if(rcvIndex > 0){
                MD_Delay(1);//当前有接收且接收到一半，稍微等待一下
                waitCnt++;
            }else{
                ret = MDE_BUFEMPTY;
                break;//当前缓冲区里面没有数据，直接退出接收
            }
        }
    }while(waitCnt < delay);

    if(waitCnt >= delay){
        s_ATRcvBuf[rcvIndex] = '\0';
        MD_DEBUG("Uncompelet URC:%s\r\n", s_ATRcvBuf);
    }

    //AT rcv buf unlock

    return ret;
}