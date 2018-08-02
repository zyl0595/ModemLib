/* 
* 文件名称：MDATCmd.c
* 摘    要：一些通用AT指令的定义及响应处理回调函数的实现
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#include <stdlib.h>
#include <string.h>
#include "MDTools.h"

/********************************** 通用AT指令定义 **************************************/
const unsigned char cmdATZ[] = "ATZ\r\n";               /*恢复出厂设置*/
const unsigned char cmdATE0[] = "ATE0\r\n";             /*关闭回显*/
const unsigned char cmdATE1[] = "ATE1\r\n";             /*打开回显*/
const unsigned char cmdATI[] = "ATI\r\n";               /*获取模块信息*/
const unsigned char cmdATGSN[] = "AT+GSN\r\n";            /*获取模块IMEI号*/
const unsigned char cmdATCPIN[] = "AT+CPIN?\r\n";       /*检查SIM状态*/
const unsigned char cmdATCIMI[] = "AT+CIMI\r\n";        /*获取SIM卡IMSI号*/
const unsigned char cmdATCSQ[] = "AT+CSQ\r\n";          /*获取信号质量*/
const unsigned char cmdATCREG[] = "AT+CREG?\r\n";       /*检查网络注册状态*/
const unsigned char cmdATCGDCONT[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n";/*定义PDP上下文*/
const unsigned char cmdATCGATT[] = "AT+CGATT?\r\n";     /*检查网络附着状态*/
const unsigned char cmdATCGACT[] = "AT+CGACT=1,1\r\n";  /*激活PDP上下文*/
const unsigned char cmdATCGREG[] = "AT+CGREG?\r\n";     /*检查GPRS网络注册状态*/

/********************************** 通用非请求结果码 ************************************/
const unsigned char urcIPNETOPEN[] ="IPNETOPEN";


/*
* 函数功能：根据指令返回结果判断模块当前GPRS网络状态是否已注册
* 参数说明：
*     pRsp: 传入参数，接收到的AT指令响应
*
* 返回值：注册成功返回MDE_OK，否则返回MDE_ERROR
*/ 
eMDErrCode MD_ATCGREG_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t stat;

    pFind =  strstr(pRsp->buf, "+CGREG");
    if(NULL != pFind){
        pFind = strstr(pRsp->buf, ",");

        if(NULL != pFind){
            pFind++;
            stat = atoi(pFind);
            MD_DEBUG("Get CGREG:%d\r\n", stat);

            if(NULL != pArg){
                *(uint8_t *)pArg = stat;
            }

            if(('1' == *pFind) || ('5' == *pFind)){
                return MDE_OK;
            }
        }
    }

    return MDE_ERROR;
}


/*
* 函数功能：根据指令返回结果判断模块当前网络注册状态
* 参数说明：
*     [in ]pRsp: 接收到的AT指令响应
*     [out]pArg: 保存注册网络注册状态代码
*
* 返回值：注册成功返回MDE_OK，否则返回MDE_ERROR
*/ 
eMDErrCode MD_ATCREG_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t stat;

    pFind =  strstr(pRsp->buf, "+CREG");
    if(NULL != pFind){
        pFind = strstr(pRsp->buf, ",");

        if(NULL != pFind){
            pFind++;
            stat = atoi(pFind);
            MD_DEBUG("Get CREG:%d\r\n", stat);

            if(NULL != pArg){
                *(uint8_t *)pArg = stat;
            }

            if((1 == stat) || (5 == stat)){
                return MDE_OK;
            }
        }
    }

    return MDE_ERROR;
}



/*
* 函数功能：根据指令返回结果判断模块当前GPRS网络附着状态
* 参数说明：
*     [in ]pRsp: 接收到的AT指令响应
*     [out]pArg: 保存注册网络附着状态代码
*
* 返回值：附着返回MDE_OK，未附着返回MDE_ERROR
*/ 
eMDErrCode MD_ATCGATT_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t val;
    uint8_t *pVal = (uint8_t *)pArg;

    pFind =  strstr(pRsp->buf, "+CGATT:");
    if(NULL != pFind){
        pFind += strlen("+CGATT:");
        val = atoi(pFind);

        /*更新系统信息*/
        if(NULL != pVal){
            *pVal = val;
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
*     [in ]pRsp: 接收到的AT指令响应
*     [out]pArg: 存储信号质量信息
*/ 
eMDErrCode MD_ATCSQ_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t rssi, ber;
    sMDCSQ *pCsq = (sMDCSQ *)pArg;

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
    if(NULL != pCsq){
        pCsq->rssi = rssi;
        pCsq->ber = ber;
    }

    MD_DEBUG("CSQ update:%d,%d\r\n", rssi, ber);

    return MDE_OK;
}


eMDErrCode MD_ATGSN_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
#define MD_IMEI_STR_LEN 15 //IMEI长度

    if(pRsp->isPositive){
        if(MD_IMEI_STR_LEN == MD_GetDecStr((uint8_t *)pArg, pRsp->buf, MD_RCV_BUF_SIZE)){
            MD_DEBUG("Get IMEI:%s\r\n", (uint8_t *)pArg);
            return MDE_OK;
        }
    }
    return MDE_ERROR;
}

eMDErrCode MD_ATCIMI_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
#define MD_IMSI_STR_LEN 15 //IMEI长度

    if(pRsp->isPositive){
        if(MD_IMSI_STR_LEN == MD_GetDecStr((uint8_t *)pArg, pRsp->buf, MD_RCV_BUF_SIZE)){
            MD_DEBUG("Get IMSI:%s\r\n", (uint8_t *)pArg);
            return MDE_OK;
        }
    }
    return MDE_ERROR;
}


eMDNetRegState MD_GetNetRegStatByCode(uint8_t code)
{
    switch(code){
    case 0:
        return NET_REG_UNREG;
    case 1:
        return NET_REG_HOME;
    case 2:
        return NET_REG_SEARCHING;
    case 3:
        return NET_REG_DENIED;
    case 5:
        return NET_REG_ROAM;
    default:
        return NET_REG_UNKNOW;
    }
}


eMDErrCode MD_DefaultUrcHdl(const uint8_t *pBuf, void *pArg)
{
    MD_DEBUG("Default Urc Hdl\r\n");
    return MDE_OK;
}