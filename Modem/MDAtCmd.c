/* 
* �ļ����ƣ�MDATCmd.c
* ժ    Ҫ���Գ����ַ�������ʽ����һЩͨ��ATָ���Թ����������ģ����н���ʱʹ�ã����ĵ�
*           ��ҪΪ�˽�ͨ�õ�ATָ����������Է�����Ĺ���
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

/********************************** ͨ��ATָ��� **************************************/
const unsigned char cmdATZ[] = "ATZ\r\n";               /*�ָ���������*/
const unsigned char cmdATE0[] = "ATE0\r\n";             /*�رջ���*/
const unsigned char cmdATE1[] = "ATE1\r\n";             /*�򿪻���*/
const unsigned char cmdATI[] = "ATI\r\n";               /*��ȡģ����Ϣ*/
const unsigned char cmdATCPIN[] = "AT+CPIN?\r\n";       /*���SIM״̬*/
const unsigned char cmdATCSQ[] = "AT+CSQ\r\n";          /*��ȡ�ź�����*/
const unsigned char cmdATCGDCONT[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n";/*����PDP������*/
const unsigned char cmdATCGATT[] = "AT+CGATT?\r\n";     /*������總��״̬*/
const unsigned char cmdATCGACT[] = "AT+CGACT=1,1\r\n";  /*����PDP������*/
const unsigned char cmdATCGREG[] = "AT+CGREG?\r\n";     /*�������ע��״̬*/

/********************************** ͨ�÷��������� ************************************/
const unsigned char urcIPNETOPEN[] ="IPNETOPEN";
