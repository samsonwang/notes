/**
 * 网络通信的基础类, SocketClient.h
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

    //TODO: 暂且保留这个接口,用于将来向UI层反馈当前网络状态
    void SetProxyWnd(HWND hProxyWnd);

    bool    Init(CNetProxy* pNetProxy);
    bool    Uninit();

    //TODO: 做成同步接口
    int Register(const char* pszUser, const char* pszPassword);
    //int Login(const char* pszUser, const char* pszPassword, int nClientType);
    void GuestLogin();  
    
    //客户端登录连接请求
    //////////////////////////////////////////////
    //异步接口
    //////////////////////////////////////////////
    void PullStockList();                                                                //获取股票代码
    /* 股票代码模糊查询
     *@param pszNum[in] 表示返回个数 不传或为空默认支持10个  -1返回所有匹配
     *@param pszType[opt_in] 股票查询类型 可不传 默认所有 1指数 2个股 例:如果是1只在指数中查找
     */
    void PullKeySpriteItems(PCTSTR pszStockName, PCTSTR pszNum = NULL, PCTSTR pszType = NULL);
    /* 快照协议
     *@param pszStockName[in] 股票代码列表 例："sz.123456,sh.600000"
     *@param pszNeedLevel[opt_in] 是否需要买卖五档,默认不需要
     */
    void PullSnapShot(PCTSTR pszStockName, PCTSTR pszNeedLevel = NULL);
    /* 字符串分割后拼接
     *@param strOutInfo[out] 拼接后的字符串
     *@param pszSrc[in] 源子符串，编码方式为unicode
     *@param nUnicodeLen[in] 源子符串的长度
     *@param strToken[in] 分割字符
     * 例如: 源串:a,b,c 分割字符为:, 拼接后字符串为: "a","b","c"
     */
    void SplitAndAsmString(std::string& strOutInfo, PCTSTR pszSrc, const int& nUnicodeLen, const std::string& strToken);
    /* 同板块涨幅排名
     *@param pszStockName[in] 股票代码
     *@param pszNum[opt_in] 最大需要的排名前多少名，可以不传，不传默认个数100
     */
    void PullBKRank(PCTSTR pszStockName, PCTSTR pszNum = NULL);
    /* 价格涨幅排名
     *@param pszType[in] 1-请求涨幅榜 2-跌幅榜 3-列表总个数
     *@param pszNum[opt_in] 最大需要的排名前多少名，可以不传，不传默认个数100
     *@param pszStart[opt_in] 从start名开始取数据 默认为0  0为从第一名开始取
     *@param pszNeed[in] 是否需要快照行情  默认0  0不需要  1为快照但无五档行情  2为快照+五档行情
     *@param pszStockType[in]  默认0 0沪深A股 1上海主板 2深圳主板 3中小板 4创业板
     */
    void PullPriceRank(PCTSTR pszType, PCTSTR pszNum = NULL, PCTSTR pszStart = NULL, PCTSTR pszNeed = NULL, PCTSTR pszStockType = NULL);
    /* 换手率涨跌幅排行
     *@param pszNum[opt_in] 最大需要的排名前多少名，可以不传，不传默认个数100
     *@param pszStart[opt_in] 从start名开始取数据 默认为0  0为从第一名开始取
     */
    void PullTurnOverRank(PCTSTR pszNum = NULL, PCTSTR pszStart = NULL);
    /* 所有股票最新价
     *@param pszStockName[opt_in] 品种信息 如果info不传或者info为空 返回全部
     */
    void PullAllLastPrice(PCTSTR pszStockName = NULL);
    /* 获取股票静态数据
     *@param pszStockName[in] 品种信息s
     */
    void PullSnapShotStatic(PCTSTR pszStockName);
    /* 快照订阅 
     *@param pszStockName[in] 股票代码列表 例："sz.123456,sh.600000"
     *@param pszType[opt_in]  0 只订阅     1 顺带取消订阅之前的所有订阅  不传默认是0
     *@param pszNeedLevel[opt_in] 是否需要买卖五档 非有效值或不传执行默认 默认是0   0不需要     1需要  2不需要返回快照只订阅
     */
    void SubscribeSnapShot(PCTSTR pszStockName, PCTSTR pszType = NULL, PCTSTR pszNeedLevel = NULL);
    /* 取消快照订阅
     *@param pszStockName[opt_in] 股票代码列表 例："sz.123456,sh.600000"
     需要取消的品种信息 如果info不传或者info为空 全部取消
     */ 
    void UnsubscribeSnapShot(PCTSTR pszStockName = NULL);

    /* 行情排名列表
     *@param pszType[in] 1-降序 2-升序 3-列表总个数
     *@param pszRankType[in] 排序类型：1-最新价 2-开盘价 3-最高价 4-最低价 5-前收价 6-均价 7-成交量 8-成交额 9-涨停价 10-跌停价 11-五日均量 12-涨跌幅 13-换手率 14-总股本 15-流通股本 16-市盈率 17-市净率
     *@param pszNum[opt_in] 最大需要的排名前多少名，可以不传，不传默认个数100  num有小数部分舍弃小数部分(如1.9直接取1)
     *@param pszStart[opt_in] 从start名开始取数据 默认为1 从第一名开始取
     *@param pszNeed[in] 是否需要五档行情  默认0  0不需要  1需要  
     *@param pszStockType[in]  默认0 0沪深A股 1上海主板 2深圳主板 3中小板 4创业板
     */
    void PullQuotRankList(PCTSTR pszType, PCTSTR pszRankType , PCTSTR pszNum = NULL, PCTSTR pszStart = NULL, PCTSTR pszNeed = NULL, PCTSTR pszStockType = NULL);
    /* 逐笔数据
     *@param pszStockName[in] 股票代码
     *@param pszNum[opt_in] 不传或为空默认40  单次请求最大支持1200根
     *@param pszTime[opt_in] 不为空 不包含该time数据 不传或为空 默认为将来时间
     */
    void PullZhuBiData(PCTSTR pszStockCode, PCTSTR pszNum = NULL,PCTSTR pszTime = NULL);
    /* 分钟K线协议
     *@param nKLineType[in] 分钟K线类型: 1分钟、5分钟、15分钟、30分钟、60分钟
     *@param pszStockCode[in] 股票代码
     *@param pszNum[opt_in] 默认650 最大个数650
     *@param pszTime[opt_in] 不为空 不包含该time数据 为空或不传 time默认为将来时间
     */
    void PullMinuteKLineData(int nKLineType, PCTSTR pszStockCode, PCTSTR pszNum = NULL, PCTSTR pszTime = NULL);

    /* 分钟K线协议 按时间段取
     *@param uMsg[in] 分钟K线类型: 1分钟、5分钟、15分钟、30分钟、60分钟
     *@param pszStockCode[in] 股票代码
     *@param pszBeginTime[opt_in] begintime和endtime不传就返回当前系统时间往前7天所有数据
                                 只传begintime就取begintime加7天的所有数据
                                 只传endtime就取endtime往前7天所有当天数据
                                 begintime与endtime时间间隔大于7天，取endtime往前7天数据
                                 暂时是7天后续根据需求修改
     *@param pszEndTime[opt_in] 
     */
    void PullMinuteKLineDataByTimepan(int nKLineType, PCTSTR pszStockCode, PCTSTR pszBeginTime = NULL, PCTSTR pszEndTime = NULL);

    /* 不，前，后 各种k线复权协议
     *@param uMsg[in] 分钟K线类型: 1分钟、5分钟、15分钟、30分钟、60分钟
     *@param pszStockCode[in] 股票代码
     *@param pszNum[opt_in] num 不传或为空默认300
     *@param pszTime[opt_in] time 不为空 不包含该time数据 time 为空或不传 默认为将来时间
     */
    void PullBQHRightKLine(const UINT& uMsg, PCTSTR pszStockCode, PCTSTR pszNum = NULL, PCTSTR pszTime = NULL);

    /* 分时协议
     *@param pszStockCode[in] 股票代码
     *@param pszBeginTime[opt_in] begintime和endtime不传就返回所有
                                 只传begintime就取begintime后的所有当天数据
                                 只传endtime就取endtime往前所有当天数据
     *@param pszEndTime[opt_in]
     */
    void PullFenShiData(PCTSTR pszStockCode, PCTSTR pszBeginTime = NULL, PCTSTR pszEndTime = NULL);

    /* 分时多日协议
     *@param pszStockCode[in] 股票代码
     *@param pszNum[opt_in] 天数 默认为0,0返回10天数据,最多返回10天(当天+历史9天)的所有数据,如果num>10 border==1 返回近10天数据(含当天)，num>10 border==0  返回近9天历史分时
     *@param pszBorder[opt_in] 默认为0 0不包含当天所有数据  1包含   
     */
    void PullFSDRData(PCTSTR pszStockCode, PCTSTR pszNum = NULL, PCTSTR pszBorder = NULL);

    /* 分时单日协议
     *@param pszStockCode[in] 股票代码
     *@param pszsubscript[opt_in] subscript 下标 默认为0  0当天的数据 1昨天的数据  2前天的数据 最大支持到9
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

    //启动发送和接收数据工作线程
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
    long                            m_nLastDataTime;        //最近一次收发数据的时间
    long                            m_nHeartbeatInterval;   //心跳包时间间隔，单位秒
    CRITICAL_SECTION                m_csLastDataTime;       //保护m_nLastDataTime的互斥体 
    HANDLE                          m_hSendDataThread;      //发送数据线程
    HANDLE                          m_hRecvDataThread;      //接收数据线程
    std::string                     m_strSendBuf;
    std::string                     m_strRecvBuf;
    HANDLE                          m_hExitEvent;
    bool                            m_bConnected;
    CRITICAL_SECTION                m_csSendBuf;
    HANDLE                          m_hSemaphoreSendBuf;
    HWND                            m_hProxyWnd;
    CNetProxy*                      m_pNetProxy;
    int                             m_nReconnectTimeInterval;    //重连时间间隔
    time_t                          m_nLastReconnectTime;        //上次重连时刻
    CFlowStatics*                   m_pFlowStatistics;
};

#endif //!__SOCKET_CLIENT_H__


