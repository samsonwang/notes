/**
 * ����ͨ�ŵĻ�����, SocketClient.h
 * zhangyl 2017.07.11
 */
#ifndef __SOCKET_CLIENT_H__
#define __SOCKET_CLIENT_H__

#include <string>
#include <winsock2.h>

#if defined(SOCKET_API_EXPORTS)
#define SOCKET_API __declspec(dllexport)
#else
#define SOCKET_API __declspec(dllimport)
#endif

class CNetProxy;
class CFlowStatics;

class SOCKET_API CSocketClient final
{
private:
    CSocketClient();
    ~CSocketClient();

    CSocketClient(const CSocketClient& rhs) = delete;
    CSocketClient& operator=(const CSocketClient& rhs) = delete;
public:
    static CSocketClient& GetInstance();

    //TODO: ���ұ�������ӿ�,���ڽ�����UI�㷴����ǰ����״̬
    void SetProxyWnd(HWND hProxyWnd);

    bool    Init(CNetProxy* pNetProxy);
    bool    Uninit();

    //TODO: ����ͬ���ӿ�
    int Register(const char* pszUser, const char* pszPassword);
    //int Login(const char* pszUser, const char* pszPassword, int nClientType);
    void GuestLogin();  
    
    //�ͻ��˵�¼��������
    //////////////////////////////////////////////
    //�첽�ӿ�
    //////////////////////////////////////////////
    void PullStockList();                                                                //��ȡ��Ʊ����
    /* ��Ʊ����ģ����ѯ
     *@param pszNum[in] ��ʾ���ظ��� ������Ϊ��Ĭ��֧��10��  -1��������ƥ��
     *@param pszType[opt_in] ��Ʊ��ѯ���� �ɲ��� Ĭ������ 1ָ�� 2���� ��:�����1ֻ��ָ���в���
     */
    void PullKeySpriteItems(PCTSTR pszStockName, PCTSTR pszNum = NULL, PCTSTR pszType = NULL);
    /* ����Э��
     *@param pszStockName[in] ��Ʊ�����б� ����"sz.123456,sh.600000"
     *@param pszNeedLevel[opt_in] �Ƿ���Ҫ�����嵵,Ĭ�ϲ���Ҫ
     */
    void PullSnapShot(PCTSTR pszStockName, PCTSTR pszNeedLevel = NULL);
    /* �ַ����ָ��ƴ��
     *@param strOutInfo[out] ƴ�Ӻ���ַ���
     *@param pszSrc[in] Դ�ӷ��������뷽ʽΪunicode
     *@param nUnicodeLen[in] Դ�ӷ����ĳ���
     *@param strToken[in] �ָ��ַ�
     * ����: Դ��:a,b,c �ָ��ַ�Ϊ:, ƴ�Ӻ��ַ���Ϊ: "a","b","c"
     */
    void SplitAndAsmString(std::string& strOutInfo, PCTSTR pszSrc, const int& nUnicodeLen, const std::string& strToken);
    /* ͬ����Ƿ�����
     *@param pszStockName[in] ��Ʊ����
     *@param pszNum[opt_in] �����Ҫ������ǰ�����������Բ���������Ĭ�ϸ���100
     */
    void PullBKRank(PCTSTR pszStockName, PCTSTR pszNum = NULL);
    /* �۸��Ƿ�����
     *@param pszType[in] 1-�����Ƿ��� 2-������ 3-�б��ܸ���
     *@param pszNum[opt_in] �����Ҫ������ǰ�����������Բ���������Ĭ�ϸ���100
     *@param pszStart[opt_in] ��start����ʼȡ���� Ĭ��Ϊ0  0Ϊ�ӵ�һ����ʼȡ
     *@param pszNeed[in] �Ƿ���Ҫ��������  Ĭ��0  0����Ҫ  1Ϊ���յ����嵵����  2Ϊ����+�嵵����
     *@param pszStockType[in]  Ĭ��0 0����A�� 1�Ϻ����� 2�������� 3��С�� 4��ҵ��
     */
    void PullPriceRank(PCTSTR pszType, PCTSTR pszNum = NULL, PCTSTR pszStart = NULL, PCTSTR pszNeed = NULL, PCTSTR pszStockType = NULL);
    /* �������ǵ�������
     *@param pszNum[opt_in] �����Ҫ������ǰ�����������Բ���������Ĭ�ϸ���100
     *@param pszStart[opt_in] ��start����ʼȡ���� Ĭ��Ϊ0  0Ϊ�ӵ�һ����ʼȡ
     */
    void PullTurnOverRank(PCTSTR pszNum = NULL, PCTSTR pszStart = NULL);
    /* ���й�Ʊ���¼�
     *@param pszStockName[opt_in] Ʒ����Ϣ ���info��������infoΪ�� ����ȫ��
     */
    void PullAllLastPrice(PCTSTR pszStockName = NULL);
    /* ��ȡ��Ʊ��̬����
     *@param pszStockName[in] Ʒ����Ϣs
     */
    void PullSnapShotStatic(PCTSTR pszStockName);
    /* ���ն��� 
     *@param pszStockName[in] ��Ʊ�����б� ����"sz.123456,sh.600000"
     *@param pszType[opt_in]  0 ֻ����     1 ˳��ȡ������֮ǰ�����ж���  ����Ĭ����0
     *@param pszNeedLevel[opt_in] �Ƿ���Ҫ�����嵵 ����Чֵ�򲻴�ִ��Ĭ�� Ĭ����0   0����Ҫ     1��Ҫ  2����Ҫ���ؿ���ֻ����
     */
    void SubscribeSnapShot(PCTSTR pszStockName, PCTSTR pszType = NULL, PCTSTR pszNeedLevel = NULL);
    /* ȡ�����ն���
     *@param pszStockName[opt_in] ��Ʊ�����б� ����"sz.123456,sh.600000"
     ��Ҫȡ����Ʒ����Ϣ ���info��������infoΪ�� ȫ��ȡ��
     */ 
    void UnsubscribeSnapShot(PCTSTR pszStockName = NULL);

    /* ���������б�
     *@param pszType[in] 1-���� 2-���� 3-�б��ܸ���
     *@param pszRankType[in] �������ͣ�1-���¼� 2-���̼� 3-��߼� 4-��ͼ� 5-ǰ�ռ� 6-���� 7-�ɽ��� 8-�ɽ��� 9-��ͣ�� 10-��ͣ�� 11-���վ��� 12-�ǵ��� 13-������ 14-�ܹɱ� 15-��ͨ�ɱ� 16-��ӯ�� 17-�о���
     *@param pszNum[opt_in] �����Ҫ������ǰ�����������Բ���������Ĭ�ϸ���100  num��С����������С������(��1.9ֱ��ȡ1)
     *@param pszStart[opt_in] ��start����ʼȡ���� Ĭ��Ϊ1 �ӵ�һ����ʼȡ
     *@param pszNeed[in] �Ƿ���Ҫ�嵵����  Ĭ��0  0����Ҫ  1��Ҫ  
     *@param pszStockType[in]  Ĭ��0 0����A�� 1�Ϻ����� 2�������� 3��С�� 4��ҵ��
     */
    void PullQuotRankList(PCTSTR pszType, PCTSTR pszRankType , PCTSTR pszNum = NULL, PCTSTR pszStart = NULL, PCTSTR pszNeed = NULL, PCTSTR pszStockType = NULL);
    /* �������
     *@param pszStockName[in] ��Ʊ����
     *@param pszNum[opt_in] ������Ϊ��Ĭ��40  �����������֧��1200��
     *@param pszTime[opt_in] ��Ϊ�� ��������time���� ������Ϊ�� Ĭ��Ϊ����ʱ��
     */
    void PullZhuBiData(PCTSTR pszStockCode, PCTSTR pszNum = NULL,PCTSTR pszTime = NULL);
    /* ����K��Э��
     *@param nKLineType[in] ����K������: 1���ӡ�5���ӡ�15���ӡ�30���ӡ�60����
     *@param pszStockCode[in] ��Ʊ����
     *@param pszNum[opt_in] Ĭ��650 ������650
     *@param pszTime[opt_in] ��Ϊ�� ��������time���� Ϊ�ջ򲻴� timeĬ��Ϊ����ʱ��
     */
    void PullMinuteKLineData(int nKLineType, PCTSTR pszStockCode, PCTSTR pszNum = NULL, PCTSTR pszTime = NULL);

    /* ����K��Э�� ��ʱ���ȡ
     *@param uMsg[in] ����K������: 1���ӡ�5���ӡ�15���ӡ�30���ӡ�60����
     *@param pszStockCode[in] ��Ʊ����
     *@param pszBeginTime[opt_in] begintime��endtime�����ͷ��ص�ǰϵͳʱ����ǰ7����������
                                 ֻ��begintime��ȡbegintime��7�����������
                                 ֻ��endtime��ȡendtime��ǰ7�����е�������
                                 begintime��endtimeʱ��������7�죬ȡendtime��ǰ7������
                                 ��ʱ��7��������������޸�
     *@param pszEndTime[opt_in] 
     */
    void PullMinuteKLineDataByTimepan(int nKLineType, PCTSTR pszStockCode, PCTSTR pszBeginTime = NULL, PCTSTR pszEndTime = NULL);

    /* ����ǰ���� ����k�߸�ȨЭ��
     *@param uMsg[in] ����K������: 1���ӡ�5���ӡ�15���ӡ�30���ӡ�60����
     *@param pszStockCode[in] ��Ʊ����
     *@param pszNum[opt_in] num ������Ϊ��Ĭ��300
     *@param pszTime[opt_in] time ��Ϊ�� ��������time���� time Ϊ�ջ򲻴� Ĭ��Ϊ����ʱ��
     */
    void PullBQHRightKLine(const UINT& uMsg, PCTSTR pszStockCode, PCTSTR pszNum = NULL, PCTSTR pszTime = NULL);

    /* ��ʱЭ��
     *@param pszStockCode[in] ��Ʊ����
     *@param pszBeginTime[opt_in] begintime��endtime�����ͷ�������
                                 ֻ��begintime��ȡbegintime������е�������
                                 ֻ��endtime��ȡendtime��ǰ���е�������
     *@param pszEndTime[opt_in]
     */
    void PullFenShiData(PCTSTR pszStockCode, PCTSTR pszBeginTime = NULL, PCTSTR pszEndTime = NULL);

    /* ��ʱ����Э��
     *@param pszStockCode[in] ��Ʊ����
     *@param pszNum[opt_in] ���� Ĭ��Ϊ0,0����10������,��෵��10��(����+��ʷ9��)����������,���num>10 border==1 ���ؽ�10������(������)��num>10 border==0  ���ؽ�9����ʷ��ʱ
     *@param pszBorder[opt_in] Ĭ��Ϊ0 0������������������  1����   
     */
    void PullFSDRData(PCTSTR pszStockCode, PCTSTR pszNum = NULL, PCTSTR pszBorder = NULL);

    /* ��ʱ����Э��
     *@param pszStockCode[in] ��Ʊ����
     *@param pszsubscript[opt_in] subscript �±� Ĭ��Ϊ0  0��������� 1���������  2ǰ������� ���֧�ֵ�9
     */
    void PullFSDanRiData(PCTSTR pszStockCode, PCTSTR pszsubscript = NULL);

    BOOL    IsClosed();
    BOOL	Connect(int timeout = 3);
    void    AddData(int cmd, const std::string& strBuffer);
    void    AddData(int cmd, const char* pszBuff, int nBuffLen);
    void    Close();

    BOOL    ConnectServer(int timeout = 3);
    BOOL    SendLoginMsg();
    BOOL    RecvLoginMsg(int& nRet);
    BOOL    Login(int& nRet);

    //�������ͺͽ������ݹ����߳�
    //bool Startup();

private:
    void LoadConfig();
    static UINT CALLBACK SendDataThreadProc(LPVOID lpParam);
    static UINT CALLBACK RecvDataThreadProc(LPVOID lpParam);
    bool Send();
    bool Recv();
    bool CheckReceivedData();
    void SendHeartbeatPackage();

private:
    SOCKET                          m_hSocket;
    short                           m_nPort;
    char                            m_szServer[64];
    long                            m_nLastDataTime;        //���һ���շ����ݵ�ʱ��
    long                            m_nHeartbeatInterval;   //������ʱ��������λ��
    CRITICAL_SECTION                m_csLastDataTime;       //����m_nLastDataTime�Ļ����� 
    HANDLE                          m_hSendDataThread;      //���������߳�
    HANDLE                          m_hRecvDataThread;      //���������߳�
    std::string                     m_strSendBuf;
    std::string                     m_strRecvBuf;
    HANDLE                          m_hExitEvent;
    bool                            m_bConnected;
    CRITICAL_SECTION                m_csSendBuf;
    HANDLE                          m_hSemaphoreSendBuf;
    HWND                            m_hProxyWnd;
    CNetProxy*                      m_pNetProxy;
    int                             m_nReconnectTimeInterval;    //����ʱ����
    time_t                          m_nLastReconnectTime;        //�ϴ�����ʱ��
    CFlowStatics*                   m_pFlowStatistics;
};

#endif //!__SOCKET_CLIENT_H__


