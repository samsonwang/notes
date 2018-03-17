/**
 * 网络通信的基础类, SocketClient.cpp
 * zhangyl 2017.07.11
 */
#include "stdafx.h"
#include "SocketClient.h"
#include "protocol.h"
#include "FlowStatistics.h"
#include "../utils/utils.h"
#include "../netproxy/ProxyMsg.h"
#include "../netproxy/NetProxy.h"
#include "../jsoncpp/jsoncpp-0.5.0/json.h"
#include <thread>
#include <memory>
#include <tchar.h>
#include <process.h>

#pragma comment(lib, "utils.lib")
#pragma comment(lib, "jsoncpp.lib")
#pragma comment(lib, "netproxy.lib")

#define MAX_JSON_LENGTH 1024
#define MAX_FIELD_LENGTH 128

unsigned int g_nSeq = 1;

unsigned short NtPkgHead2Crc16(const unsigned int* pdwBuf)
{//计算CRC
    unsigned int dwCrc = 0;
    dwCrc ^= *pdwBuf++;
    dwCrc ^= *pdwBuf++;
    dwCrc ^= *pdwBuf++;
    dwCrc ^= *pdwBuf++;
    dwCrc ^= *pdwBuf++;
    return (unsigned short)((dwCrc & 0xffff) ^ (dwCrc >> 16));
}

bool CheckPkgHead(NtPkgHead* head)
{
    //协议包起始标志不正确，丢弃此包
    if (head->bStartFlag != PKGHEAD_START_FLAG)
        return false;

    unsigned short  uCrc = head->wCrc;
    head->wCrc = 0;
    //CRC校验不正确，丢弃此包
    if (NtPkgHead2Crc16((const unsigned int *)head) != ::ntohs(uCrc))
    {
        head->wCrc = uCrc;
        return false;
    }
    head->wCrc = uCrc;
    return true;
}

CSocketClient& CSocketClient::GetInstance()
{
    static CSocketClient sc;
    return sc;
}

CSocketClient::CSocketClient()
{
    m_nPort = 20000;
    m_hSocket = INVALID_SOCKET;
    memset(m_szServer, 0, sizeof(m_szServer));
    m_nHeartbeatInterval = 30;
    m_nLastDataTime = (long)time(NULL);
    m_bConnected = false;
    m_hExitEvent = NULL;
    m_hSemaphoreSendBuf = NULL;

    m_hSendDataThread = NULL;
    m_hRecvDataThread = NULL;

    m_hProxyWnd = NULL;

    m_nReconnectTimeInterval = 0;
    m_nLastReconnectTime = 0;

    m_pFlowStatistics = new CFlowStatics();
}    

CSocketClient::~CSocketClient()
{
    delete m_pFlowStatistics;
}

void CSocketClient::SetProxyWnd(HWND hProxyWnd)
{
    m_hProxyWnd = hProxyWnd;
}

bool CSocketClient::Init(CNetProxy* pNetProxy)
{  
    m_pNetProxy = pNetProxy;
    
    //LoadConfig();

    m_hExitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    ::InitializeCriticalSection(&m_csLastDataTime);
    ::InitializeCriticalSection(&m_csSendBuf);

    m_hSemaphoreSendBuf = ::CreateSemaphore(NULL, 0, 65535, NULL);

    //if (!Connect())
    //{
    //LOG_WARNING("connect server:%s:%d error.", m_szServer, m_nPort);
    //}

    m_hSendDataThread = (HANDLE)(ULONG_PTR)::_beginthreadex(NULL, 0, &SendDataThreadProc, this, 0, NULL);
    m_hRecvDataThread = (HANDLE)(ULONG_PTR)::_beginthreadex(NULL, 0, &RecvDataThreadProc, this, 0, NULL);
    
    return true;
}

bool CSocketClient::Uninit()
{
    ::SetEvent(m_hExitEvent);

    int nWaitTimeout = 1000;
    if (::WaitForSingleObject(m_hSendDataThread, nWaitTimeout) != WAIT_OBJECT_0)
    {
        ::TerminateThread(m_hSendDataThread, 1L);
    }

    if (m_hSendDataThread != NULL)
        ::CloseHandle(m_hSendDataThread);

    if (::WaitForSingleObject(m_hRecvDataThread, nWaitTimeout) != WAIT_OBJECT_0)
    {
        ::TerminateThread(m_hRecvDataThread, 1L);
    }

    if (m_hRecvDataThread != NULL)
        ::CloseHandle(m_hRecvDataThread);
    
    if (m_hExitEvent != NULL)
        ::CloseHandle(m_hExitEvent);

    if (m_hSemaphoreSendBuf != NULL)
        ::CloseHandle(m_hSemaphoreSendBuf);

    ::DeleteCriticalSection(&m_csLastDataTime);
    ::DeleteCriticalSection(&m_csSendBuf);

    return true;
}

//int CSocketClient::Login(const char* pszUser, const char* pszPassword, int nClientType)
//{
//    return 0;
//}

void CSocketClient::GuestLogin()
{
    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"user\":\"16666666661\",\"clientversion\":\"2.4.5\",\"clienttype\":0,\"visitor\":0}");
    AddData(GP_CMD_GUESTLOGIN, szJson, strlen(szJson));
}

void CSocketClient::PullStockList()
{
    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"info\":[{\"market\":\"SH\",\"verflag\":\"1486588648\"},{\"market\":\"SZ\",\"verflag\":\"1486588648\"}]}");
    AddData(GP_CMD_GETSTOCKLIST, szJson, strlen(szJson));
}

void CSocketClient::PullKeySpriteItems(PCTSTR pszStockName, PCTSTR pszNum, PCTSTR pszType)
{  
    if (pszStockName == NULL || pszStockName[0] == NULL)
        return;
    
    char szStockName[32] = { 0 };
    EncodeUtil::UnicodeToUtf8(pszStockName, szStockName, ARRAYSIZE(szStockName));

    char szStockNum[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNum, szStockNum, ARRAYSIZE(szStockNum));
    }

    char szStockType[16] = { 0 };
    if (pszType != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszType, szStockType, ARRAYSIZE(szStockType));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"fuzzycode\":\"%s\",\"num\":\"%s\",\"type\":\"%s\"}", szStockName, szStockNum, szStockType);
    AddData(GP_CMD_FUZZYQUERY, szJson, strlen(szJson));
}

void CSocketClient::SplitAndAsmString(std::string& strOutInfo, PCTSTR pszSrc, const int& nLen, const std::string& strToken)
{
    int nUTF8Len = nLen + 1;
    char* pszAnsiStockName = new char[nUTF8Len * sizeof(char)];
    CHeapAutoDeletor helpDeletor(pszAnsiStockName, nUTF8Len);
    memset(pszAnsiStockName, 0, nUTF8Len);
    EncodeUtil::UnicodeToUtf8(pszSrc, pszAnsiStockName, nUTF8Len);

    std::vector<std::string> vectStockName;
    //拆分字符串，分隔符为逗号
    StringUtil::SplitByToken(vectStockName, pszAnsiStockName, strToken.c_str());
    //股票名组装为JSON格式
    StringUtil::ConcatenateWithToken(strOutInfo, vectStockName, strToken);
}

void CSocketClient::PullSnapShot(PCTSTR pszStockName, PCTSTR pszNeedLevel)
{
    int nLength = lstrlen(pszStockName);
    if (nLength <= 0)
        return;

    std::string strStockName;
    SplitAndAsmString(strStockName, pszStockName, nLength, STOCK_INFO_TOKEN);

    char szNeedLevel[16] = { 0 };
    if (pszNeedLevel != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNeedLevel, szNeedLevel, ARRAYSIZE(szNeedLevel));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"needlevel\":\"%s\",\"info\":[%s]}", szNeedLevel, strStockName.c_str());
    AddData(GP_CMD_SNAPSHOT, szJson, strlen(szJson));
}

void CSocketClient::PullBKRank(PCTSTR pszStockName, PCTSTR pszNum)
{
    if (pszStockName == NULL || pszStockName[0] == NULL)
        return;
    char szStockName[32] = { 0 };
    EncodeUtil::UnicodeToUtf8(pszStockName, szStockName, ARRAYSIZE(szStockName));

    char szStockNum[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNum, szStockNum, ARRAYSIZE(szStockNum));
    }
    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"id\":\"%s\",\"num\":\"%s\"}", szStockName, szStockNum);
    AddData(GP_CMD_BKRANK, szJson, strlen(szJson));
}

void CSocketClient::PullPriceRank(PCTSTR pszType, PCTSTR pszNum, PCTSTR pszStart, PCTSTR pszNeed, PCTSTR pszStockType)
{
    if (pszType == NULL || pszType[0] == NULL)
        return;
    char szPriceType[16] = { 0 };
    if (pszType != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszType, szPriceType, ARRAYSIZE(szPriceType));
    }

    char szPriceNum[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNum, szPriceNum, ARRAYSIZE(szPriceNum));
    }

    char szStart[16] = { 0 };
    if (pszStart != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszStart, szStart, ARRAYSIZE(szStart));
    }

    char szNeedSnapShot[16] = { 0 };
    if (pszNeed != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNeed, szNeedSnapShot, ARRAYSIZE(szNeedSnapShot));
    }

    char szStockType[16] = { 0 };
    if (pszStockType != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszStockType, szStockType, ARRAYSIZE(szStockType));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"type\":\"%s\",\"num\":\"%s\",\"start\":\"%s\",\"need\":\"%s\",\"stocktype\":\"%s\"}", szPriceType, szPriceNum, szStart, szNeedSnapShot, szStockType);
    AddData(GP_CMD_PRICERANK, szJson, strlen(szJson));
}

void CSocketClient::PullTurnOverRank(PCTSTR pszNum, PCTSTR pszStart)
{
    char szTurnOverNum[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNum, szTurnOverNum, ARRAYSIZE(szTurnOverNum));
    }

    char szStart[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszStart, szStart, ARRAYSIZE(szStart));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"num\":\"%s\",\"start\":\"%s\"}", szTurnOverNum, szStart);
    AddData(GP_CMD_TURNOVERRANK, szJson, strlen(szJson));
}

void CSocketClient::PullAllLastPrice(PCTSTR pszStockName)
{
    char szStockName[32] = { 0 };
    if (pszStockName != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszStockName, szStockName, ARRAYSIZE(szStockName));
    }
    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"info\":[\"%s\"]}", szStockName);
    AddData(GP_CMD_ALLLASTPRICE, szJson, strlen(szJson));
}

void CSocketClient::PullSnapShotStatic(PCTSTR pszStockName)
{
    if (pszStockName == NULL || pszStockName[0] == NULL)
        return;
    char szStockName[32] = { 0 };
    EncodeUtil::UnicodeToUtf8(pszStockName, szStockName, ARRAYSIZE(szStockName));
    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"info\":[\"%s\"]}", szStockName);
    AddData(GP_CMD_SNAPSHOTSTATIC, szJson, strlen(szJson));
}

void CSocketClient::SubscribeSnapShot(PCTSTR pszStockName, PCTSTR pszType, PCTSTR pszNeedLevel)
{
    if (pszStockName == NULL || pszStockName[0] == NULL)
        return;

    std::string strStockName;
    SplitAndAsmString(strStockName, pszStockName, lstrlen(pszStockName), STOCK_INFO_TOKEN);

    char szSubType[16] = { 0 };
    if (pszType != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszType, szSubType, ARRAYSIZE(szSubType));
    }

    char szNeedLevel[16] = { 0 };
    if (pszNeedLevel != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNeedLevel, szNeedLevel, ARRAYSIZE(szNeedLevel));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"needlevel\":\"%s\",\"type\":\"%s\",\"info\":[%s]}", szNeedLevel, szSubType, strStockName.c_str());
    AddData(GP_CMD_SUBSCRIBE, szJson, strlen(szJson));
}

void CSocketClient::UnsubscribeSnapShot(PCTSTR pszStockName)
{
    std::string strStockName;
    SplitAndAsmString(strStockName, pszStockName, lstrlen(pszStockName), STOCK_INFO_TOKEN);

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"info\":[%s]}", strStockName.c_str());
    AddData(GP_CMD_UNSUBSCRIBE, szJson, strlen(szJson));
}

void CSocketClient::LoadConfig()
{
    TCHAR szFilePath[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szFilePath, ARRAYSIZE(szFilePath));
    int i = _tcslen(szFilePath) - 1;
    for (; i >= 0; --i)
    {
        if (szFilePath[i] == _T('\\'))
        {
            szFilePath[i + 1] = _T('\0');
            break;
        }
    }

    CIniFile iniFile;
    TCHAR szIniFile[MAX_PATH] = { 0 };
    _sntprintf_s(szIniFile, ARRAYSIZE(szIniFile), _T("%sconfig\\%s"), szFilePath, _T("gbclient.ini"));
    TCHAR szTmpServer[64] = { 0 };
    iniFile.ReadString(_T("server"), _T("ip"), _T("192.168.19.190"), szTmpServer, 64, szIniFile);
    if (!EncodeUtil::UnicodeToAnsi(szTmpServer, m_szServer, ARRAYSIZE(m_szServer)))
       LOG_ERROR("gbclient.ini UnicodeToAnsi error!");
    m_nPort = iniFile.ReadInt(_T("server"), _T("port"), 7051, szIniFile);
    m_nHeartbeatInterval = iniFile.ReadInt(_T("net"), _T("heartbeatinterval"), 30, szIniFile);
}

void CSocketClient::AddData(int cmd, const std::string& strBuffer)
{
    AddData(cmd, strBuffer.c_str(), strBuffer.length());
}

void CSocketClient::AddData(int cmd, const char* pszBuff, int nBuffLen)
{
    NtPkgHead stNtPkgHead;
    stNtPkgHead.bStartFlag = 0xFF;
    stNtPkgHead.bVer = 0;
    stNtPkgHead.bEncryptFlag = '0';
    stNtPkgHead.bFrag = 0;
    stNtPkgHead.wLen = htons(sizeof(NtPkgHead)+nBuffLen);  //包头+ 包体长度
    stNtPkgHead.wCmd = htons(cmd);
    stNtPkgHead.wSeq = htons(g_nSeq++);
    stNtPkgHead.dwSID = 0;
    stNtPkgHead.wTotal = 0;
    stNtPkgHead.wCurSeq = 0;
    stNtPkgHead.wCrc = 0;
    stNtPkgHead.wCrc = htons(NtPkgHead2Crc16((const unsigned int *)&stNtPkgHead));//重新计算crc

    ::EnterCriticalSection(&m_csSendBuf);
    m_strSendBuf.append((const char*)(&stNtPkgHead), sizeof(NtPkgHead));
    m_strSendBuf.append(pszBuff, nBuffLen);
    ::LeaveCriticalSection(&m_csSendBuf);

    m_pFlowStatistics->IncreaseSentPackageNum();
    m_pFlowStatistics->IncreaseSentBytes((int64_t)(sizeof(NtPkgHead) + nBuffLen));

    ::ReleaseSemaphore(m_hSemaphoreSendBuf, 1, NULL);

    LOG_NORMAL("Send data: cmd=%u, length: %d.", cmd, nBuffLen + sizeof(NtPkgHead));   
}

BOOL CSocketClient::IsClosed()
{
    return !m_bConnected;
}

BOOL CSocketClient::Connect(int timeout)
{
    //if (!IsClosed())
    //    return TRUE;
    Close();

    m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_hSocket == INVALID_SOCKET)
        return FALSE;

    long tmSend = 3 * 1000L;
    long tmRecv = 3 * 1000L;
    long noDelay = 1;
    setsockopt(m_hSocket, IPPROTO_TCP, TCP_NODELAY, (LPSTR)&noDelay, sizeof(long));
    setsockopt(m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (LPSTR)&tmSend, sizeof(long));
    setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (LPSTR)&tmRecv, sizeof(long));

    //将socket设置成非阻塞的
    unsigned long on = 1;
    if (::ioctlsocket(m_hSocket, FIONBIO, &on) == SOCKET_ERROR)
        return FALSE;

    struct sockaddr_in addrSrv = { 0 };
    struct hostent* pHostent = NULL;
    unsigned int addr = 0;

    if ((addrSrv.sin_addr.s_addr = inet_addr(m_szServer)) == INADDR_NONE)
    {
        pHostent = ::gethostbyname(m_szServer);
        if (!pHostent)
        {
            LOG_ERROR("Could not connect server:%s, port:%d.", m_szServer, m_nPort);
            return FALSE;
        }
        else
            addrSrv.sin_addr.s_addr = *((unsigned long*)pHostent->h_addr);
    }

    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons((u_short)m_nPort);
    int ret = ::connect(m_hSocket, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
    if (ret == 0)
    {
        LOG_NORMAL("Connect server:%s, port:%d successfully.", m_szServer, m_nPort);
        m_bConnected = TRUE;
        return TRUE;
    }

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        LOG_ERROR("Could not connect server:%s, port:%d.", m_szServer, m_nPort);
        return FALSE;
    }

    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(m_hSocket, &writeset);
    struct timeval tv = { timeout, 0 };
    if (::select(m_hSocket + 1, NULL, &writeset, NULL, &tv) != 1)
    {
        LOG_ERROR("Could not connect server:%s, port:%d.", m_szServer, m_nPort);
        return FALSE;
    }

    m_bConnected = TRUE;
    LOG_NORMAL("Connect server:%s, port:%d successfully.", m_szServer, m_nPort);

    ::PostMessage(m_pNetProxy->GetReflectionWnd(), WM_NET_STATUS, (WPARAM)NET_STATUS_CONNECTED, 0);

    return TRUE;
}

BOOL CSocketClient::ConnectServer(int timeout)
{
    LoadConfig();

    m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_hSocket == INVALID_SOCKET)
        return FALSE;

    long tmSend = 3 * 1000L;
    long tmRecv = 3 * 1000L;
    long noDelay = 1;
    setsockopt(m_hSocket, IPPROTO_TCP, TCP_NODELAY, (LPSTR)&noDelay, sizeof(long));
    setsockopt(m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (LPSTR)&tmSend, sizeof(long));
    setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (LPSTR)&tmRecv, sizeof(long));

    //将socket设置成非阻塞的
    unsigned long on = 1;
    if (::ioctlsocket(m_hSocket, FIONBIO, &on) == SOCKET_ERROR)
        return FALSE;

    struct sockaddr_in addrSrv = { 0 };
    struct hostent* pHostent = NULL;
    unsigned int addr = 0;

    if ((addrSrv.sin_addr.s_addr = inet_addr(m_szServer)) == INADDR_NONE)
    {
        pHostent = ::gethostbyname(m_szServer);
        if (!pHostent)
        {
            LOG_ERROR("Could not connect server:%s, port:%d.", m_szServer, m_nPort);
            return FALSE;
        }
        else
            addrSrv.sin_addr.s_addr = *((unsigned long*)pHostent->h_addr);
    }

    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons((u_short)m_nPort);
    int ret = ::connect(m_hSocket, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
    if (ret == 0)
    {
        LOG_NORMAL("Connect server:%s, port:%d successfully.", m_szServer, m_nPort);
        m_bConnected = TRUE;
        return TRUE;
    }

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        LOG_ERROR("Could not connect server:%s, port:%d.", m_szServer, m_nPort);
        return FALSE;
    }

    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(m_hSocket, &writeset);
    struct timeval tv = { timeout, 0 };
    if (::select(m_hSocket + 1, NULL, &writeset, NULL, &tv) != 1)
    {
        LOG_ERROR("Could not connect server:%s, port:%d.", m_szServer, m_nPort);
        return FALSE;
    }

    m_bConnected = TRUE;
    LOG_NORMAL("Connect server:%s, port:%d successfully.", m_szServer, m_nPort);

    return TRUE;
}

BOOL CSocketClient::Login(int& nRet)
{
    LoadConfig();
    if (!ConnectServer())
        return FALSE;
    if (!SendLoginMsg())
        return FALSE;
    if (!RecvLoginMsg(nRet))
        return FALSE;

    return TRUE;
}

BOOL CSocketClient::SendLoginMsg()
{
    std::string strBody = "{\"user\":\"16666666661\",\"clientversion\":\"2.4.5\",\"clienttype\":0,\"visitor\":0}";

    NtPkgHead stNtPkgHead;
    stNtPkgHead.bStartFlag = 0xFF;
    stNtPkgHead.bVer = 0;
    stNtPkgHead.bEncryptFlag = '0';
    stNtPkgHead.bFrag = 0;
    stNtPkgHead.wLen = htons(sizeof(NtPkgHead)+strBody.length());  //包头+ 包体长度
    stNtPkgHead.wCmd = htons(GP_CMD_GUESTLOGIN);
    stNtPkgHead.wSeq = htons(g_nSeq++);
    stNtPkgHead.dwSID = 0;
    stNtPkgHead.wTotal = 0;
    stNtPkgHead.wCurSeq = 0;
    stNtPkgHead.wCrc = 0;
    stNtPkgHead.wCrc = htons(NtPkgHead2Crc16((const unsigned int *)&stNtPkgHead));//重新计算crc

    std::string strSendBuf;
    strSendBuf.append((const char*)(&stNtPkgHead), sizeof(NtPkgHead));
    strSendBuf.append(strBody.c_str(), strBody.length());

    int nLen = -1;
    int nSentLen = 0;
    while (true)
    {
        nLen = ::send(m_hSocket, strSendBuf.c_str(), strSendBuf.length(), 0);
        if (nLen == SOCKET_ERROR)
        {
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                break;
            else
            {
                LOG_WARNING("Send data error, disconnect server:%s, port:%d.", m_szServer, m_nPort);
                Close();
                return false;
            }
        }
        else if (nLen < 1)
        {
            //一旦出现错误就立刻关闭Socket
            LOG_WARNING("Send data error, disconnect server:%s, port:%d.", m_szServer, m_nPort);
            Close();
            return false;
        }

        nSentLen += nLen;
        strSendBuf.erase(0, nLen);
        if (strSendBuf.empty())
            break;

        ::Sleep(1);
    }// end while-loop
    return true;
}

BOOL CSocketClient::RecvLoginMsg(int& nRet)
{
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(m_hSocket, &readset);
    struct timeval tv = { 3, 0 };
    if (::select(m_hSocket + 1, &readset, NULL, NULL, &tv) != 1)
    {
        LOG_ERROR("Could not connect server:%s, port:%d.", m_szServer, m_nPort);
        return FALSE;
    }

    int nLen = -1;
    std::string     strRecvBuff;
    NtPkgHead       pkgHead;
    unsigned int    uPkgLen;
    while (true)
    {
        // TODO 修改为一次只收包头或包体
        char buff[512];
        nLen = ::recv(m_hSocket, buff, 512, 0);
        if (nLen == SOCKET_ERROR)				//一旦出现错误就立刻关闭Socket
        {
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                break;
            else
            {
                LOG_WARNING("Recv data error, disconnect server:%s, port:%d.", m_szServer, m_nPort);
                //Close();
                return false;
            }
        }
        else if (nLen < 1)
        {
            LOG_WARNING("Recv data error, disconnect server:%s, port:%d.", m_szServer, m_nPort);
            //Close();
            return false;
        }

        strRecvBuff.append(buff, nLen);
        if (strRecvBuff.length() >= sizeof(NtPkgHead))
        {
            memset(&pkgHead, 0, sizeof(pkgHead));
            memcpy_s(&pkgHead, sizeof(pkgHead), strRecvBuff.c_str(), sizeof(pkgHead));
            if (::ntohs(pkgHead.wCmd) != GP_CMD_GUESTLOGIN)
            {
                LOG_ERROR("Recv error msg, disconnect server:%s, port:%d.", m_szServer, m_nPort);
                return false;
            }
            uPkgLen = ntohs(pkgHead.wLen);
        }

        if (uPkgLen > 0)
        {
            if (strRecvBuff.length() >= uPkgLen)
                break;
        }

        ::Sleep(1);
    }
    std::string     strPkg;
    std::string     strBody;
    unsigned int    uBodyLen;

    strPkg.append(strRecvBuff.c_str(), uPkgLen);
    uBodyLen = uPkgLen - sizeof(NtPkgHead);
    strBody.append(strPkg.data() + sizeof(NtPkgHead), uBodyLen);

    Json::Reader JsonReader;
    Json::Value JsonRoot;
    if (!JsonReader.parse(strBody.c_str(), JsonRoot) && !JsonRoot["ret"].isString())
    {
        //打印出错日志
        LOG_ERROR("parse json error");
        return false;
    }

    string strRet = JsonRoot["ret"].asString();
    if (strRet == "0")
        nRet = 0;
    else
        nRet = -1;

    return true;

}

UINT CSocketClient::SendDataThreadProc(LPVOID lpParam)
{
    LOG_NORMAL("Start send data thread.");

    CSocketClient* pSocketClient = (CSocketClient*)lpParam;
    //if (pSocketClient == NULL)
    //  return 1;

    HANDLE arrHandles[2];
    arrHandles[0] = pSocketClient->m_hExitEvent;
    arrHandles[1] = pSocketClient->m_hSemaphoreSendBuf;

    DWORD dwWaitResult;
    while (true)
    {
        dwWaitResult = ::WaitForMultipleObjects(ARRAYSIZE(arrHandles), arrHandles, FALSE, INFINITE);
        if (dwWaitResult == WAIT_OBJECT_0)
            break;
        else if (dwWaitResult == WAIT_OBJECT_0 + 1)
        {
            
            if (!pSocketClient->Send())
            {

                LOG_ERROR("Send data error, disconnect server: %s:%d", pSocketClient->m_szServer, pSocketClient->m_nPort);
                ::PostMessage(pSocketClient->m_pNetProxy->GetReflectionWnd(), WM_NET_STATUS, (WPARAM)NET_STATUS_DISCONNECTED, 0);

                //重连
                ::PostMessage(pSocketClient->m_pNetProxy->GetReflectionWnd(), WM_NET_STATUS, (WPARAM)NET_STATUS_CONNECTING, 0);
                if (pSocketClient->m_nReconnectTimeInterval == 0)
                {
                    while (true)
                    {
                        //重连成功
                        if (pSocketClient->Connect())
                        {
                            pSocketClient->m_nReconnectTimeInterval = 0;
                            //先将前一次的Session数据清零，因为已经失效了
                            ::EnterCriticalSection(&pSocketClient->m_csSendBuf);
                            pSocketClient->m_strSendBuf.clear();
                            ::LeaveCriticalSection(&pSocketClient->m_csSendBuf);
                            //TODO: 暂且游客登录一下，将来支持正式账户登录,还要判断当前位置是在登录界面还是在主界面
                            pSocketClient->GuestLogin();
                            break;
                        }
                        else
                        {
                            //重连不成功，递增2秒后重试
                            pSocketClient->m_nReconnectTimeInterval += 1;
                            pSocketClient->m_nLastReconnectTime = ::time(NULL);

                            //此时程序需要退出
                            if (::WaitForSingleObject(pSocketClient->m_hExitEvent, 0) == WAIT_OBJECT_0)
                                break;

                            //TODO: Sleep会导致退出进程退出慢，后期优化下
                            ::Sleep(pSocketClient->m_nReconnectTimeInterval * 1000);
                        }
                    }// end inner while-loop
                    
                }
            }


        }// end outer-if
    }// end outer while-loop

    LOG_NORMAL("Exit send data thread.");

    return 0;
}

bool CSocketClient::Send()
{
    int nRet = 0;
    std::string strTmpSendBuffer;
    ::EnterCriticalSection(&m_csSendBuf);
    strTmpSendBuffer.append(m_strSendBuf.c_str(), m_strSendBuf.length());
    ::LeaveCriticalSection(&m_csSendBuf);
    if (strTmpSendBuffer.empty())
        return true;

    int nSentLen = 0;
    while (true)
    {
        nRet = ::send(m_hSocket, strTmpSendBuffer.c_str(), strTmpSendBuffer.length(), 0);
        if (nRet == SOCKET_ERROR)
        {
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                break;
            else
            {
                LOG_WARNING("Send data error, disconnect server:%s, port:%d.", m_szServer, m_nPort);
                Close();
                return false;
            }
        }
        else if (nRet < 1)
        {
            //一旦出现错误就立刻关闭Socket
            LOG_WARNING("Send data error, disconnect server:%s, port:%d.", m_szServer, m_nPort);
            Close();
            return false;
        }

        nSentLen += nRet;
        strTmpSendBuffer.erase(0, nRet);
        if (strTmpSendBuffer.empty())
            break;

        ::Sleep(1);
    }// end while-loop

    ::EnterCriticalSection(&m_csSendBuf);
    if (nSentLen > 0)
        m_strSendBuf.erase(0, nSentLen);
    ::LeaveCriticalSection(&m_csSendBuf);
        
    ::EnterCriticalSection(&m_csLastDataTime);
    m_nLastDataTime = (long)time(NULL);
    ::LeaveCriticalSection(&m_csLastDataTime);

    return true;
}

void CSocketClient::SendHeartbeatPackage()
{
    ::EnterCriticalSection(&m_csLastDataTime);
    m_nLastDataTime = (long)time(NULL);
    ::LeaveCriticalSection(&m_csLastDataTime);

    char szJson[32] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"heartbeattime\":\"%d\"}", time(NULL));
    AddData(GP_CMD_HEARTBEAT, szJson, strlen(szJson));
}

UINT CSocketClient::RecvDataThreadProc(LPVOID lpParam)
{
    LOG_NORMAL("Start recv data thread.");
    DWORD           dwWaitResult;
    std::string     strPkg;
    std::string     strTotalPkg;
    unsigned short  uPkgLen = 0;
    unsigned int    uBodyLen = 0;
    unsigned int    uTotalPkgLen = 0;
    unsigned int    uCmd = 0;
    NtPkgHead       pkgHead;
    unsigned short  uTotal = 0;
    unsigned short  uCurSeq = 0;
    int             nWaitTimeout = 1;   

    CSocketClient* pSocketClient = (CSocketClient*)lpParam;
    //if (pSocketClient == NULL)
    //  return 1;

    while (true)
    {
        dwWaitResult = ::WaitForSingleObject(pSocketClient->m_hExitEvent, nWaitTimeout);
        if (dwWaitResult == WAIT_OBJECT_0)
            break;

        //检测到数据则收数据
        if (pSocketClient->CheckReceivedData())
        {
            //TODO: 进行重连，如果连接不上，则向客户报告错误
            if (!pSocketClient->Recv())
            {
                LOG_ERROR("Recv data error");
                ::PostMessage(pSocketClient->m_pNetProxy->GetReflectionWnd(), WM_NET_STATUS, (WPARAM)NET_STATUS_DISCONNECTED, 0);
                //发送线程会进行重连，所以continue以等待时机
                pSocketClient->m_strRecvBuf.clear();
                continue;
            }

            //一定要放在一个循环里面解包，因为可能一片数据中有多个包，
            //对于数据收不全，这个地方我纠结了好久T_T
            while (true)
            {
                if (pSocketClient->m_strRecvBuf.length() < sizeof(NtPkgHead))
                    break;

                memset(&pkgHead, 0, sizeof(pkgHead));
                memcpy_s(&pkgHead, sizeof(pkgHead), pSocketClient->m_strRecvBuf.c_str(), sizeof(pkgHead));

                uPkgLen = ntohs(pkgHead.wLen);
                if (pSocketClient->m_strRecvBuf.length() < uPkgLen)
                    break;

                strPkg.clear();
                strPkg.append(pSocketClient->m_strRecvBuf.c_str(), uPkgLen);

                //LOG_NORMAL("package body size: %u.", uPkgLen);
                //从收取缓冲区中移除已经处理的数据部分
                pSocketClient->m_strRecvBuf.erase(0, uPkgLen);

                pSocketClient->m_pFlowStatistics->IncreaseRecvBytes(uPkgLen);

                //对包消息头检验
                if (!CheckPkgHead(&pkgHead))
                {
                    //如果包头检验不通过，缓冲区里面的数据已经是脏数据了，直接清空掉，
                    //等到给服务器发请求，下一次服务器再接着应答数据
                    //如果是连接出问题，重连后仍然可以得到正确的数据                   
                    LOG_ERROR("Check package head error, discard data %d bytes", (int)pSocketClient->m_strRecvBuf.length());
                    pSocketClient->m_pFlowStatistics->IncreaseDiscardNum();
                    pSocketClient->m_pFlowStatistics->IncreaseDiscardBytes((int64_t)pSocketClient->m_strRecvBuf.length());
                    pSocketClient->m_strRecvBuf.clear();
                    break;                 
                }

                
                uTotal = ::ntohs(pkgHead.wTotal);
                uCurSeq = ::ntohs(pkgHead.wCurSeq);
                //没有分包或者第一个分包
                if (uCurSeq == 0)
                {
                    strTotalPkg.clear();
                    uTotalPkgLen = 0;
                }

                uBodyLen = uPkgLen - sizeof(NtPkgHead);
                uTotalPkgLen += uBodyLen;
                strTotalPkg.append(strPkg.data() + sizeof(NtPkgHead), uBodyLen);
                            
                //无分包 或 分包的最后一个包 则将组装后的包发送出去
                if (uTotal == 0 || (uTotal != 0 && uTotal == uCurSeq + 1))
                {
                    if (uTotalPkgLen <= 0)
                    {
                        //TODO: 这个地方到底是continue还是break?
                        continue;
                    }

                    /*nRecvPackageNum = */pSocketClient->m_pFlowStatistics->IncreaseRecvPackageNum();
                    //nSentPackageNum = pSocketClient->m_pFlowStatistics->GetSentPackageNum();
                    //LOG_NORMAL("Sent package num: %lld, recv package num: %lld", nSentPackageNum, nRecvPackageNum);

                    uCmd = ::ntohs(pkgHead.wCmd);
                    //心跳包响应，丢弃
                    if (uCmd == GP_CMD_HEARTBEAT)
                    {
                        continue;
                    }

                    ProxyPackage proxyPackage;
                    proxyPackage.nCmd = uCmd;
                    proxyPackage.nLength = uTotalPkgLen;
                    proxyPackage.pszJson = new char[uTotalPkgLen];
                    memset(proxyPackage.pszJson, 0, uTotalPkgLen*sizeof(char));
                    memcpy_s(proxyPackage.pszJson, uTotalPkgLen, strTotalPkg.c_str(), strTotalPkg.length());
                    
                    pSocketClient->m_pNetProxy->AddPackage((const char*)&proxyPackage, sizeof(proxyPackage));
                }           
            }// end while-loop
        }
        else
        {
            long nLastDataTime = 0;
            {
                ::EnterCriticalSection(&pSocketClient->m_csLastDataTime);
                nLastDataTime = pSocketClient->m_nLastDataTime;
                ::LeaveCriticalSection(&pSocketClient->m_csLastDataTime);
            }
            //调试版本就不要发心跳包了，影响调试
//#ifndef _DEBUG
            if (time(NULL) - nLastDataTime >= pSocketClient->m_nHeartbeatInterval)
                pSocketClient->SendHeartbeatPackage();
//#endif
        }// end if
    }// end while-loop

    int64_t nRecvPackageNum = pSocketClient->m_pFlowStatistics->GetRecvPackageNum();
    int64_t nSentPackageNum = pSocketClient->m_pFlowStatistics->GetSentPackageNum();
    int64_t nDiscardNum = pSocketClient->m_pFlowStatistics->GetDiscardNum();
    int64_t nRecvBytes = pSocketClient->m_pFlowStatistics->GetRecvRecvBytes();
    int64_t nDiscardBytes = pSocketClient->m_pFlowStatistics->GetDiscardBytes();
    LOG_NORMAL("Sent package num: %lld, recv package num: %lld, discard num: %lld", nSentPackageNum, nRecvPackageNum, nDiscardNum);
    LOG_NORMAL("Recv bytes: %lld, discard bytes: %lld", nRecvBytes, nDiscardBytes);

    LOG_NORMAL("Exit recv data thread.");

    return 0;
}

bool CSocketClient::CheckReceivedData()
{
    if (!m_bConnected)
        return false;
    
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(m_hSocket, &readset);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500;

    long nRet = ::select(m_hSocket + 1, &readset, NULL, NULL, &timeout);
    if (nRet == 1)
        return true;
    else if (nRet == SOCKET_ERROR)
    {
        LOG_WARNING("Check socket data error, disconnect server:%s, port:%d.", m_szServer, m_nPort);
        //Close();
        //::PostMessage(m_pNetProxy->GetReflectionWnd(), WM_NET_STATUS, (WPARAM)NET_STATUS_DISCONNECTED, 0);
    }

    //超时nRet=0，也返回FALSE，因为在超时的这段时间内也没有数据

    return false;
}

bool CSocketClient::Recv()
{
    if (!m_bConnected)
        return false;
    
    int nRet = 0;
    while (true)
    {
        char buff[512];
        nRet = ::recv(m_hSocket, buff, 512, 0);
        if (nRet == SOCKET_ERROR)				//一旦出现错误就立刻关闭Socket
        {
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                break;
            else
            {
                LOG_WARNING("Recv data error, disconnect server:%s, port:%d.", m_szServer, m_nPort);
                //Close();
                return false;
            }
        }
        else if (nRet < 1)
        {
            LOG_WARNING("Recv data error, disconnect server:%s, port:%d.", m_szServer, m_nPort);
            //Close();
            return false;
        }

        m_strRecvBuf.append(buff, nRet);

        ::Sleep(1);
    }

    {
        ::EnterCriticalSection(&m_csLastDataTime);
        m_nLastDataTime = (long)time(NULL);
        ::LeaveCriticalSection(&m_csLastDataTime);
    }

    return true;
}

void CSocketClient::Close()
{
    if (m_hSocket == INVALID_SOCKET)
        return;

    ::shutdown(m_hSocket, SD_BOTH);
    ::closesocket(m_hSocket);
    m_hSocket = INVALID_SOCKET;

    m_bConnected = FALSE;
}

void CSocketClient::PullQuotRankList(PCTSTR pszType, PCTSTR pszRankType, PCTSTR pszNum, PCTSTR pszStart, PCTSTR pszNeed, PCTSTR pszStockType)
{
    if (pszType == NULL || pszType[0] == NULL)
        return;

    char szSortType[16] = { 0 };
    if (pszType != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszType, szSortType, ARRAYSIZE(szSortType));
    }

    char szNum[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNum, szNum, ARRAYSIZE(szNum));
    }

    char szStart[16] = { 0 };
    if (pszStart != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszStart, szStart, ARRAYSIZE(szStart));
    }

    char szNeedSnapShot[16] = { 0 };
    if (pszNeed != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNeed, szNeedSnapShot, ARRAYSIZE(szNeedSnapShot));
    }

    char szRankType[16] = { 0 };
    if (pszRankType != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszRankType, szRankType, ARRAYSIZE(szRankType));
    }

    char szStockType[16] = { 0 };
    if (pszStockType != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszStockType, szStockType, ARRAYSIZE(szStockType));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"type\":\"%s\",\"num\":\"%s\",\"start\":\"%s\",\"needlevel\":\"%s\",\"ranktype\":\"%s\",\"stocktype\":\"%s\"}", szSortType, szNum, szStart, szNeedSnapShot, szRankType, szStockType);
    AddData(GP_CMD_QUOTRANKLIST, szJson, strlen(szJson));
}

void CSocketClient::PullZhuBiData(PCTSTR pszStockCode, PCTSTR pszNum , PCTSTR pszTime )
{
    if (pszStockCode == NULL || pszStockCode[0] == NULL)
        return;
    char szStockCode[16] = { 0 };
    if (pszStockCode != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszStockCode, szStockCode, ARRAYSIZE(szStockCode));
    }

    char szNum[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNum, szNum, ARRAYSIZE(szNum));
    }

    char szTime[16] = { 0 };
    if (pszTime != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszTime, szTime, ARRAYSIZE(szTime));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"id\":\"%s\",\"time\":\"%s\",\"num\":\"%s\"}", szStockCode, szTime, szNum);
    AddData(GP_CMD_ZHUBIDATA, szJson, strlen(szJson));
}

void CSocketClient::PullMinuteKLineData(int nKLineType, PCTSTR pszStockCode, PCTSTR pszNum, PCTSTR pszTime)
{
    if (pszStockCode == NULL || pszStockCode[0] == NULL)
        return;

    char szStockCode[32] = { 0 };
    EncodeUtil::UnicodeToUtf8(pszStockCode, szStockCode, ARRAYSIZE(szStockCode));

    char szNum[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNum, szNum, ARRAYSIZE(szNum));
    }

    char szTime[16] = { 0 };
    if (pszTime != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszTime, szTime, ARRAYSIZE(szTime));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"id\":\"%s\",\"time\":\"%s\",\"num\":\"%s\"}", szStockCode, szTime, szNum);
    AddData(nKLineType, szJson, strlen(szJson));
}

void CSocketClient::PullMinuteKLineDataByTimepan(int nKLineType, PCTSTR pszStockCode, PCTSTR pszBeginTime, PCTSTR pszEndTime)
{
    if (pszStockCode == NULL || pszStockCode[0] == NULL)
        return;

    char szStockCode[32] = { 0 };
    EncodeUtil::UnicodeToUtf8(pszStockCode, szStockCode, ARRAYSIZE(szStockCode));

    char szBeginTime[16] = { 0 };
    if (pszBeginTime != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszBeginTime, szBeginTime, ARRAYSIZE(szBeginTime));
    }

    char szEndTime[16] = { 0 };
    if (pszEndTime != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszEndTime, szEndTime, ARRAYSIZE(szEndTime));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"id\":\"%s\",\"begintime\":\"%s\",\"endtime\":\"%s\"}", szStockCode, szBeginTime, szEndTime);
    AddData(nKLineType, szJson, strlen(szJson));
}

void CSocketClient::PullBQHRightKLine(const UINT& uMsg, PCTSTR pszStockCode, PCTSTR pszNum, PCTSTR pszTime)
{
    if (pszStockCode == NULL || pszStockCode[0] == NULL)
        return;

    char szStockCode[32] = { 0 };
    EncodeUtil::UnicodeToUtf8(pszStockCode, szStockCode, ARRAYSIZE(szStockCode));

    char szNum[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNum, szNum, ARRAYSIZE(szNum));
    }

    char szTime[16] = { 0 };
    if (pszTime != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszTime, szTime, ARRAYSIZE(szTime));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"id\":\"%s\",\"time\":\"%s\",\"num\":\"%s\"}", szStockCode, szTime, szNum);
    AddData(uMsg, szJson, strlen(szJson));
}

void CSocketClient::PullFenShiData(PCTSTR pszStockCode, PCTSTR pszBeginTime, PCTSTR pszEndTime)
{
    if (pszStockCode == NULL || pszStockCode[0] == NULL)
        return;

    char szStockCode[32] = { 0 };
    EncodeUtil::UnicodeToUtf8(pszStockCode, szStockCode, ARRAYSIZE(szStockCode));

    char szBeginTime[16] = { 0 };
    if (pszBeginTime != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszBeginTime, szBeginTime, ARRAYSIZE(szBeginTime));
    }

    char szEndTime[16] = { 0 };
    if (pszEndTime != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszEndTime, szEndTime, ARRAYSIZE(szEndTime));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"id\":\"%s\",\"begintime\":\"%s\",\"endtime\":\"%s\"}", szStockCode, szBeginTime, szEndTime);
    AddData(GP_CMD_FENSHIDATA, szJson, strlen(szJson));
}

void CSocketClient::PullFSDRData(PCTSTR pszStockCode, PCTSTR pszNum, PCTSTR pszBorder)
{
    if (pszStockCode == NULL || pszStockCode[0] == NULL)
        return;

    char szStockCode[32] = { 0 };
    EncodeUtil::UnicodeToUtf8(pszStockCode, szStockCode, ARRAYSIZE(szStockCode));

    char szNum[16] = { 0 };
    if (pszNum != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszNum, szNum, ARRAYSIZE(szNum));
    }

    char szBorder[16] = { 0 };
    if (pszBorder != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszBorder, szBorder, ARRAYSIZE(szBorder));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"id\":\"%s\",\"border\":\"%s\",\"num\":\"%s\"}", szStockCode, szBorder, szNum);
    AddData(GP_CMD_FSDUORI_DATA, szJson, strlen(szJson));
}

void CSocketClient::PullFSDanRiData(PCTSTR pszStockCode, PCTSTR pszsubscript)
{
    if (pszStockCode == NULL || pszStockCode[0] == NULL)
        return;

    char szStockCode[32] = { 0 };
    EncodeUtil::UnicodeToUtf8(pszStockCode, szStockCode, ARRAYSIZE(szStockCode));

    char szsubscript[16] = { 0 };
    if (pszsubscript != NULL)
    {
        EncodeUtil::UnicodeToUtf8(pszsubscript, szsubscript, ARRAYSIZE(szsubscript));
    }

    char szJson[MAX_JSON_LENGTH] = { 0 };
    sprintf_s(szJson, ARRAYSIZE(szJson), "{\"id\":\"%s\",\"subscript\":\"%s\"}", szStockCode, szsubscript);
    AddData(GP_CMD_FSDANRI_DATA, szJson, strlen(szJson));
}



