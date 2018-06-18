#include <string.h>
#include "MDType.h"
#include "MDConfig.h"
#include "MD_CLM920.h"



/***************** end : CLM920 ********************/

/*
* 识别模块类型，初始化模块连接网络 
*/ 
int MD_Init(void)
{
    int ret = 0;
    if(MD_TtysOpen()){
        if(MDE_OK == CLM920_Init()){
            MD_DEBUG("CLM920 init succ!\r\n");
            ret = 1;
        }else{
            MD_DEBUG("CLM920 init failed!\r\n");
        }
    }else{
         MD_DEBUG("ttys open failed!\r\n");
    }
    
    return ret;
}


void MD_test(void)
{
    eMDErrCode ret;
    sMDTcpData tcpData;
    const uint8_t str[] = "a test string";
    
    tcpData.pIp = "192.168.0.1";
    tcpData.port = 5700;
    tcpData.pData = str;
    tcpData.len = sizeof(str);
    
    do{
        ret = CLM920_SendIpData(3, &tcpData);
    }while(MDE_OK != ret);
}


