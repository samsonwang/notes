#pragma once
#include <vector>
#include <map>


//目前只有前三种权限，如果是【未知权限】，说明程序出错。
#define PRIVILEGE_GROUP_PUBLIC	_T("")			//公共权限
#define PRIVILEGE_GROUP_A		_T("A组")
#define PRIVILEGE_GROUP_B		_T("B组")
#define PRIVILEGE_GROUP_UNKNOWN _T("未知")		//如果是这个权限，说明程序出错

class MailAnalysis;

class PopSocket
{
public:
	PopSocket(void);
	~PopSocket(void);

	BOOL Connect(LPCSTR lpUrl,long lPort=110);
	BOOL Logon(LPCSTR lpUser,LPCSTR lpPassword);
	BOOL IsConnect() const { return m_hSocket; }

	BOOL GetStat();
	BOOL GetList();
	long GetMailCount() const;
	long GetMainIndex(long lIndex) const;
	long GetMailSize(long lIndex) const;
	BOOL GetHeader(int nMailIndex);
	BOOL GetMail(int nMailIndex,MailAnalysis& mail);
	BOOL GetMail(long nMailIndex,long lMailSize,MailAnalysis& mail);
	void Quit();	//退出

	LPCTSTR GetTitle() const { return m_strTitle; }
	//LPCTSTR GetUserGroup();
	LPCTSTR GetFrom() const { return m_strFrom; }
	DWORD	GetDate() const { return m_dwDate; }
	DWORD	GetTime() const { return m_dwTime; }
	LPCTSTR GetMessageID() const { return m_strMessageID; }
	LPCSTR GetSubject() const { return m_strSubject; }

	LPCSTR GetMailData() const { return m_strMailData; }
	LPCSTR GetRecvData() const { return m_strRecv; }

	__int64 GetTotalRecvSize() const { return m_iRecvTotal; }
	
	void Close();

private:
	BOOL RecvBody();
	BOOL RecvData();

	BOOL Request(BOOL bBody);
	

	BOOL VerifyRecvData();

	SOCKET		m_hSocket;
	CStringA	m_strSend;
	CStringA	m_strRecv;
	char		m_szBuffer[1024*200];
	BOOL		m_bConnected;

	CString		m_strTitle;		//邮件主题
	CString		m_strUserGroup;	//权限组描述字符
	DWORD		m_dwDate;
	DWORD		m_dwTime;
	CString		m_strFrom;
	CString		m_strMessageID;
	CStringA	m_strMailData;
	CStringA	m_strSubject;

	BOOL		m_bProxy;
	
	__int64		m_iRecvTotal;

	struct Impl;
	Impl*	m_pImpl;
};

class MailAnalysis
{
public:
	MailAnalysis();
	~MailAnalysis();

	void SetTitle(LPCWSTR lpTitle)	{ m_strTitle=lpTitle; }
	void SetFrom(LPCTSTR lpFrom)		{ m_strFrom=lpFrom; }
	void SetDateTime(DWORD dwDate,DWORD dwTime) { m_dwDate=dwDate; m_dwTime=dwTime; }
	void SetMail(LPCSTR lpMail)		{ m_strMail=lpMail; }
	void SetMessageID(LPCTSTR lpValue)		{ m_strMessageID=lpValue; }
	void SetSubject(LPCSTR lpSubject) { m_strSubject=lpSubject; }

	LPCTSTR GetMessageID() const { return m_strMessageID; }

	LPCTSTR GetText(long lIndex=0) const;
	LPCTSTR GetTitle() const { return m_strTitle; }
	LPCTSTR GetHTML(long lIndex=0) const;
	DWORD	GetDate() const { return m_dwDate; }
	DWORD	GetTime() const { return m_dwTime; }
	LPCSTR	GetMailData() const { return m_strMail; }
	LPCSTR	GetSubject() const { return m_strSubject; }

	static void SetEmailSavePath(LPCTSTR lpPath); 

	BOOL Save();

	BOOL GetAttachment(long lIndex,CString& strTitle,CStringA& strAttachment) const;
	long GetAttachmentCount() const { return (long)m_aryAttachment.size(); }

	BOOL Analysis();
	BOOL IsMobilePlatform()	const { return m_bIsFromMobilePlatform; }
	BOOL DetectMobilePlatform();
	void Clear();

private:
	BOOL GetBoundaryTag();
	BOOL AnalysisBoundary(LPCSTR lpString,LPCSTR lpBoundary);
	BOOL IsExistBoundary(const CStringA& strData,LPCSTR lpExclude) const;

	CString		m_strTitle;
	CString		m_strFrom;
	DWORD		m_dwDate;
	DWORD		m_dwTime;
	CStringA	m_strMail;		//邮件内容
	CStringA	m_strSubject;	
	CString		m_strMessageID;
	BOOL		m_bIsFromMobilePlatform;

	std::vector<CStringA>	m_aryBoundary;
	std::vector<CString>	m_aryText;
	std::vector<CString>	m_aryHTML;

	std::map<CString,CStringA>	m_aryAttachment;	//附件

	static TCHAR g_szEmailSavePath[MAX_PATH];
};

