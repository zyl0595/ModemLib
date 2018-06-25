
/********************************** 通用AT指令定义 **************************************/
const unsigned char cmdATE0[] = "ATE0\r\n";             /*关闭回显*/
const unsigned char cmdATE1[] = "ATE1\r\n";             /*打开回显*/
const unsigned char cmdATI[] = "ATI\r\n";               /*获取模块信息*/
const unsigned char cmdATCPIN[] = "AT+CPIN?\r\n";       /*检查SIM状态*/
const unsigned char cmdATCGDCONT[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n";/*定义PDP上下文*/
const unsigned char cmdATCGATT[] = "AT+CGATT?\r\n";     /*检查网络附着状态*/
const unsigned char cmdATCGACT[] = "AT+CGACT=1,1\r\n";  /*激活PDP上下文*/
const unsigned char cmdATCGREG[] = "AT+CGREG?\r\n";     /*检查网络注册状态*/

/********************************** 通用非请求结果码 ************************************/
const unsigned char urcIPNETOPEN[] ="IPNETOPEN";
