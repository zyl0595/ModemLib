/* 
* 文件名称：MD.c
* 摘    要：
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
#include "MDType.h"
#include "MDConfig.h"
#include "MDPort.h"
#include "MD_CLM920.h"

/*************************** 私有定义 ******************************/

/*************************** 私有变量 ******************************/

/*************************** 全局变量 ******************************/



/*************************** 函数实现 ******************************/

/*
* 识别模块类型，初始化模块连接网络 
*/ 
int MD_Init(void)
{
    int ret = 0;
    if(MD_TtysOpen()){
        if(MDE_OK == CLM920_Init()){
            MD_DEBUG("CLM920 init succ!\r\n");
            CLM920_SokctInit();
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
    sMDSockData tcpData;
    const uint8_t str[] = "a test string";
    
    tcpData.pIp = "120.42.46.98";
    tcpData.port = 5700;
    tcpData.pData = str;
    tcpData.len = sizeof(str);
    
    do{
        ret = CLM920_SendIpData(3, &tcpData);
		if(MDE_OK != ret)MD_Delay(5000);
    }while(MDE_OK != ret);
}

//域名解析
eMDErrCode MD_GetHostByName(const char *pName, sMDIPv4Addr *pAddr)
{

}