/* 
* 文件名称：MDATCmd.c
* 摘    要：以常量字符串的形式定义一些通用AT指令以供各个具体的模块进行交互时使用，本文档
*           主要为了将通用的AT指令集中在这里以方便查阅管理
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

/********************************** 通用AT指令定义 **************************************/
const unsigned char cmdATZ[] = "ATZ\r\n";               /*恢复出厂设置*/
const unsigned char cmdATE0[] = "ATE0\r\n";             /*关闭回显*/
const unsigned char cmdATE1[] = "ATE1\r\n";             /*打开回显*/
const unsigned char cmdATI[] = "ATI\r\n";               /*获取模块信息*/
const unsigned char cmdATCPIN[] = "AT+CPIN?\r\n";       /*检查SIM状态*/
const unsigned char cmdATCSQ[] = "AT+CSQ\r\n";          /*获取信号质量*/
const unsigned char cmdATCGDCONT[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n";/*定义PDP上下文*/
const unsigned char cmdATCGATT[] = "AT+CGATT?\r\n";     /*检查网络附着状态*/
const unsigned char cmdATCGACT[] = "AT+CGACT=1,1\r\n";  /*激活PDP上下文*/
const unsigned char cmdATCGREG[] = "AT+CGREG?\r\n";     /*检查网络注册状态*/

/********************************** 通用非请求结果码 ************************************/
const unsigned char urcIPNETOPEN[] ="IPNETOPEN";
