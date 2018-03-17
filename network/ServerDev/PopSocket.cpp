#include "StdAfx.h"
#include "PopSocket.h"
#include <vector>
#include <map>
#include "MailStringHelper.h"
#include "LogTrace.h"

using namespace MailStringHelper;
using namespace Log;



#pragma  pack(1)	//结构体全部1字节对齐
typedef struct tagMailItem
{
	long	_lID;
	long	_lSize;
}MAIL_ITEM,*PMAIL_ITEM;
#pragma  pack()


struct PopSocket::Impl
{
	typedef std::vector<MAIL_ITEM> MAIL_LIST;
	MAIL_LIST	_MailList;	
};

PopSocket::PopSocket(void)
:m_hSocket(NULL),m_bConnected(FALSE),m_pImpl(new Impl),m_iRecvTotal(0),m_bProxy(FALSE)
{
	memset(m_szBuffer,0,sizeof(m_szBuffer));
	m_strRecv.GetBuffer(1024*10000);

	//m_bProxy=TRUE;
}

PopSocket::~PopSocket(void)
{
	if (m_pImpl)
	{
		delete m_pImpl;
		m_pImpl=NULL;
	}
}

void PopSocket::Close()
{
	if (m_hSocket) 
	{
		closesocket(m_hSocket);
		m_hSocket=NULL;
	}

	m_bConnected=FALSE;
}

long PopSocket::GetMailCount() const
{
	return (long)m_pImpl->_MailList.size();
}

long PopSocket::GetMainIndex(long lIndex) const
{
	if (lIndex<0 || (long)m_pImpl->_MailList.size()<=lIndex) return -1;

	return m_pImpl->_MailList[lIndex]._lID;
}

long PopSocket::GetMailSize(long lIndex) const
{
	if (lIndex<0 || (long)m_pImpl->_MailList.size()<=lIndex) return -1;

	return m_pImpl->_MailList[lIndex]._lSize;
}

BOOL PopSocket::Connect(LPCSTR lpUrl, long lPort)
{
	MailLogNormalA("[PopSocket::Connect] Start connect [%s:%d].",lpUrl,lPort);
	struct sockaddr_in server={0}; 
	struct hostent *pHostent=NULL;
	unsigned int addr=0;
	
	Close();
	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (m_hSocket == INVALID_SOCKET) return FALSE;

	long tmSend(15*1000L),tmRecv(15*1000L),noDelay(1);
	setsockopt(m_hSocket,IPPROTO_TCP,TCP_NODELAY,(LPSTR)&noDelay,sizeof(long));
	setsockopt(m_hSocket,SOL_SOCKET,SO_SNDTIMEO,(LPSTR)&tmSend,sizeof(long));
	setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(LPSTR)&tmRecv,sizeof(long));

	//m_bProxy = TRUE;
	if (!m_bProxy)
	{
		if(inet_addr(lpUrl)==INADDR_NONE)
		{
			pHostent=gethostbyname(lpUrl) ;
		}
		else
		{
			addr=inet_addr(lpUrl);
			pHostent=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
		}

		if (!pHostent) return FALSE;

		server.sin_family = AF_INET;
		server.sin_port = htons((u_short)lPort); 
		server.sin_addr.s_addr=*((unsigned long*)pHostent->h_addr);
		if(connect(m_hSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
		{
			MailLogErrorA("[PopSocket::Connect] Connect [%s:%d] failed.Error=%d",lpUrl,lPort,WSAGetLastError());
			return FALSE;
		}

		if (!RecvData()) return FALSE;
		MailLogNormalA("[PopSocket::Connect] Connect [%s:%d]. Recv:%s",lpUrl,lPort,m_strRecv);

		if (!VerifyRecvData()) return FALSE;

		MailLogNormalA("[PopSocket::Connect] Connect [%s:%d] successfully.",lpUrl,lPort);
	}
	else
	{
		WORD wPortConn=8080;
		char	szUrl[]="10.67.4.124";
		sockaddr_in	remote={0};
		remote.sin_family = AF_INET;
	
		remote.sin_port = htons(wPortConn);
		LPHOSTENT lphost=NULL;
		if((remote.sin_addr.s_addr=inet_addr(szUrl))==INADDR_NONE)
		{
			if(lphost=gethostbyname(szUrl))
				remote.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		}

		if(connect(m_hSocket,(sockaddr*)&remote,sizeof(remote))==SOCKET_ERROR)
		{
			MailLogErrorA("[PopSocket::Connect] Connect proxy [%s,%d] failed. Error=%d",szUrl,wPortConn,WSAGetLastError());
			return FALSE;
		}

		char szBuffer[1024]={0};
		sprintf_s(szBuffer,"CONNECT %s:%d HTTP/1.0\r\nUser-Agent:rmtcmd/0.1\r\n\r\n",lpUrl,lPort);
		send(m_hSocket,szBuffer,strlen(szBuffer),0);
		if(recv(m_hSocket,szBuffer,ARRAYSIZE(szBuffer)-1,0)<1)
		{
			MailLogErrorA("[PopSocket::Connect] recv [%s,%d] failed. Error=%d",szUrl,wPortConn,WSAGetLastError());
			return FALSE;
		}

		MailLogNormalA("[PopSocket::Connect] recv proxy string: %s",szBuffer);

		if(strstr(szBuffer,"Connection established")==NULL)
		{
			MailLogErrorA("[PopSocket::Connect] connect proxy [%s,%d] failed. Error=%s",szUrl,wPortConn,szBuffer);
			return FALSE;
		}
		
	}

	return TRUE;
}

BOOL PopSocket::RecvBody()
{
	if (!m_hSocket) return FALSE;

	int nRecv=0;
	long lRecvSize=0;
	long lPos=0;
	long lEndLen=0;
	m_strRecv.Empty();
	long lTryCount=20;
	while( TRUE )
	{
		fd_set fd={0};
		timeval tmout={0,1};
		FD_SET(m_hSocket,&fd);
		if(select(0,&fd,NULL,NULL,&tmout)<1||!FD_ISSET(m_hSocket,&fd))
		{
			if (lTryCount<=0) break;

			Sleep(500);
			--lTryCount;
			continue;
		}

		memset(m_szBuffer,0,sizeof(m_szBuffer));
		nRecv = ::recv(m_hSocket , m_szBuffer , sizeof(m_szBuffer)-1 , NULL);					
		if(nRecv == SOCKET_ERROR) 
		{
			MailLogWarning(_T("[PopSocket::RecvBody] Recv data failed. Error=%d."),WSAGetLastError());
			Close();
			return FALSE;
		}

		if (nRecv>0)
		{
			m_strRecv.Append(m_szBuffer,nRecv);
			lRecvSize+=nRecv;
			m_iRecvTotal+=nRecv;
		}

		if (m_strRecv.Find("\r\n.\r\n")!=-1) 
			break;
	}

	//严格判断邮件结尾,否则无效
	if (m_strRecv.Find("\r\n.\r\n")==-1)
	{
		MailLogWarning(_T("[PopSocket::RecvBody] Recv data not have '\\r\\n.\\r\\n'."));
		MailLogWarningA("[PopSocket::RecvBody] Recv data:\r\n%s",m_szBuffer);
		return FALSE;
	}

	return TRUE;
}

BOOL PopSocket::RecvData()
{
	if (!m_hSocket) return FALSE;

	int nRecv=0;
	long lRecvSize=0;
	long lPos=0;
	long lEndLen=0;
	m_strRecv.Empty();
	long lTryCount=5;
	while( TRUE )
	{
		fd_set fd={0};
		timeval tmout={0,1};
		FD_SET(m_hSocket,&fd);
		if(select(0,&fd,NULL,NULL,&tmout)<1||!FD_ISSET(m_hSocket,&fd))
		{
			if (lTryCount<=0) break;

			--lTryCount;
			Sleep(50);
			continue;
		}

		memset(m_szBuffer,0,sizeof(m_szBuffer));
		nRecv = ::recv(m_hSocket , m_szBuffer , sizeof(m_szBuffer)-1 , NULL);					
		if(nRecv == SOCKET_ERROR) 
		{
			MailLogWarning(_T("[PopSocket::RecvData] Recv data failed. Error=%d."),WSAGetLastError());
			Close();
			return FALSE;
		}

		if (nRecv>0)
		{
			m_strRecv.Append(m_szBuffer,nRecv);
			lRecvSize+=nRecv;
			m_iRecvTotal+=nRecv;
		}

		if (m_strRecv.Find("\r\n")!=-1) break;
	}

	//严格判断结尾,否则无效
	if ( m_strRecv.Find("\r\n")==-1) 
	{
		MailLogWarning(_T("[PopSocket::RecvData] Recv data not have '\\n\\r'."));
		return FALSE;
	}
	
	return TRUE;
}

BOOL PopSocket::VerifyRecvData()
{
	if (m_strRecv.IsEmpty()) return FALSE;
	char szHeader[5]={0};
	long lHeaderSize=sizeof(szHeader)-1;
	strncpy_s(szHeader,m_strRecv.GetBuffer(),lHeaderSize);
	_strupr_s(szHeader);
	MailLogNormalA("[PopSocket::VerifyRecvData] %s",szHeader);
	if (szHeader[0]=='+' && szHeader[1]=='O' && szHeader[2]=='K') return TRUE;
	if (szHeader[0]=='-' && szHeader[1]=='E' && szHeader[2]=='R' && szHeader[3]=='R') return FALSE; 
	return TRUE;
}

BOOL PopSocket::Logon(LPCSTR lpUser,LPCSTR lpPassword)
{
	if (!m_hSocket) return FALSE;

	MailLogNormalA("[PopSocket::Logon] Logon [User:%s].",lpUser);
	int nResult=0;
	m_strSend.Format("USER %s\r\n",lpUser);
	if (!Request(FALSE)) return FALSE;
	if (!VerifyRecvData()) return FALSE;

	MailLogNormalA("[PopSocket::Logon] Logon [User:%s] [Pass:*****].",lpUser);
	m_strSend.Format("PASS %s\r\n", lpPassword);
	if (!Request(FALSE)) return FALSE;
	if (!VerifyRecvData()) return FALSE;

	MailLogNormalA("[PopSocket::Logon] Logon [User:%s] successfully",lpUser);
	m_bConnected = TRUE;

	return TRUE;
}

BOOL PopSocket::GetStat()
{
	if (!m_hSocket) return FALSE;

	m_strSend="STAT\r\n";
	if (!Request(FALSE)) return FALSE;
	if (!VerifyRecvData()) return FALSE;

	/*
	m_strSend.Format("UIDL\r\n",0);
	if (!Request(DATA_END_FLAG)) return FALSE;
	if (!VerifyRecvData()) return FALSE;
	*/

	return TRUE;
}

BOOL PopSocket::GetList()
{
	if (!m_hSocket) return FALSE;

	m_strSend="LIST\r\n";
	if (!Request(TRUE)) return FALSE;
	if (!VerifyRecvData()) return FALSE;

	Impl::MAIL_LIST& mailList=m_pImpl->_MailList;
	mailList.clear();

	MAIL_ITEM item={0};
	STRING_LIST aryString,aryStringItem;
	SplitString(m_strRecv,COMMAND_END_FLAG,aryString);
	size_t lCount=aryString.size();
	for(size_t i=0;i<lCount;++i)
	{
		const CStringA& strItem=aryString[i];
		if (strItem[0]=='+') continue;

		if (SplitString(strItem," ",aryStringItem)==2)
		{
			item._lID=atoi(aryStringItem[0]);
			item._lSize=atoi(aryStringItem[1]);
			mailList.push_back(item);
		}
	}
	
	return TRUE;
}


BOOL PopSocket::Request(BOOL bBody)
{
	m_strRecv.Empty();
	if (!m_hSocket) return FALSE;
	if (m_strSend.IsEmpty()) return TRUE;

	int nResult=0;
	nResult = ::send(m_hSocket , m_strSend.GetString() , m_strSend.GetLength(), NULL);
	Sleep(300);
	if (nResult== SOCKET_ERROR) return FALSE;

	if (bBody)
	{
		if (!RecvBody()) 
		{
			Close();
			return FALSE;
		}
	}
	else
	{
		if (!RecvData()) 
		{
			Close();
			return FALSE;
		}
	}

	return TRUE;
}

BOOL PopSocket::GetHeader(int nMailIndex)
{	
	m_dwDate=m_dwTime=0;
	m_strFrom.Empty();
	m_strMessageID.Empty();
	m_strTitle.Empty();
	m_strSubject.Empty();
	if (!m_hSocket) return FALSE;

	MailLogTrace(LOG_LEVEL_DEBUG,_T("[PopSocket::GetHeader] [Index=%d] request header......"),nMailIndex);

	int nResult=0;
	m_strSend.Format("TOP %d 1\r\n" , nMailIndex);

	if (!Request(TRUE)) return FALSE;

	MailLogTrace(LOG_LEVEL_DEBUG,_T("[PopSocket::GetHeader] [Index=%d] Analysis ......"),nMailIndex);

	STRING_LIST aryString,aryStringItem;
	STRING_MAP mapString;
	SplitString(m_strRecv,COMMAND_END_FLAG,aryString);
	AnalysisTag(aryString,mapString);
	CStringA strTemp;
	STRING_MAP::iterator iter;
	long lStart=0,lEnd=0;

	iter=mapString.find(EMAIL_DATE_TAG);	//邮件时间
	if (iter!=mapString.end())
	{
		CStringW strTemp(iter->second);
		lStart = strTemp.Find(_T(","),0);
		strTemp = strTemp.Mid(lStart+1);
		lStart = strTemp.Find(_T("+"),0);
		//去掉时区
		if (lStart<0) lStart=strTemp.Find(_T("-"),0);
		if (lStart>0) strTemp = strTemp.Left(lStart);
		COleDateTime oledt;
		oledt.ParseDateTime(strTemp);
		if(oledt.GetStatus()!=COleDateTime::valid) oledt = COleDateTime::GetCurrentTime();
		m_dwDate = oledt.GetYear()*10000+oledt.GetMonth()*100+oledt.GetDay();
		m_dwTime = oledt.GetHour()*10000+oledt.GetMinute()*100+oledt.GetSecond();
	}

	iter=mapString.find(EMAIL_SUBJECT_TAG);
	if (iter!=mapString.end())
	{
		m_strSubject=EMAIL_SUBJECT_TAG+iter->second;
		AnalysisString(iter->second,m_strTitle);
	}

	iter=mapString.find(EMAIL_FROM_TAG);
	if (iter!=mapString.end())
	{
		lStart=iter->second.Find('<');
		if (lStart>0)
		{
			lEnd=iter->second.Find('>',lStart+1);
			if (lEnd>0) 
			{
				strTemp=iter->second.Mid(lStart+1,lEnd-lStart-1);
				Code::GB2312ToUnicode(strTemp,m_strFrom);
			}
		}
	}

	iter=mapString.find(EMAIL_MESSAGE_ID_TAG);
	if (iter!=mapString.end())
	{
		strTemp=iter->second;
		lStart=iter->second.Find('<');
		if (lStart>0)
		{
			lEnd=iter->second.Find('>',lStart+1);
			if (lEnd>0) strTemp=iter->second.Mid(lStart+1,lEnd-lStart-1);	
		}
		Code::GB2312ToUnicode(strTemp,m_strMessageID);
	}

	MailLogTrace(LOG_LEVEL_DEBUG,_T("[PopSocket::GetHeader] [Index=%d] [Title=%s] [Frome=%s] [DateTime=%d %d] [ID=%s]"),nMailIndex,m_strTitle,m_strFrom,m_dwDate,m_dwTime,m_strMessageID);
	
	
	return TRUE;
}


//LPCTSTR PopSocket::GetUserGroup()
//{
//	m_strUserGroup = _T("");
//	TCHAR modulePath[MAX_PATH];
//	GetModuleFileName(NULL, modulePath, MAX_PATH);
//	CString strModulePath(modulePath);
//	strModulePath = strModulePath.Left(strModulePath.ReverseFind(_T('\\')));
//	strModulePath += _T("\\ReportMailTitleKey.xml");
//	
//	TiXmlDocument doc(CStringA(strModulePath).GetString());  
//	bool loadOkay = doc.LoadFile(); 
//	if(!loadOkay)
//	{
//		MailLogWarning(_T("Loading % failed!\n"), strModulePath.GetString());
//	}
//
//	CString strTitle=m_strTitle;
//	CString strGroup;
//	strTitle.MakeUpper();	//全部大写
//	//根节点
//	TiXmlElement* root = doc.RootElement();
//	int nGroupNum = 0;
//	//获取权限组树木
//	if(root!=NULL)
//		nGroupNum = atoi(root->FirstAttribute()->Value());
//
//	TiXmlNode* child = NULL;
//	for(TiXmlElement *item = root->FirstChildElement(); item!=NULL; item=item->NextSiblingElement())
//	{
//		const char* groupname = item->Attribute("name");
//		
//		//<group name="A组">
//		for(TiXmlElement *desc = item->FirstChildElement(); desc; desc=desc->NextSiblingElement())
//		{
//			const char* descText = desc->GetText();
//			CString strTemp;
//			Code::GB2312ToUnicode(descText,strTemp);
//			strTemp.MakeUpper();
//			if(!strTemp.IsEmpty() && m_strTitle.Find(strTemp.GetString())!=-1)
//			{
//				Code::GB2312ToUnicode(groupname,strGroup);
//				m_strUserGroup = strGroup;
//				return m_strUserGroup;
//			}	
//		}
//			
//	}
//
//
//	return m_strUserGroup;
//}

//用户组信息权限描述
//const TCHAR* g_aryAUserGroupDescriptions[4] = {_T("发a"), _T("发送a"),  _T("发A"), _T("发送A"), _T("烦请发送A"), _T("发送a")};
//const TCHAR* g_aryBUserGroupDescriptions[4] = {_T("发b"), _T("") _T("发B"), _T("发送B"), _T("烦请发送B"), _T("发送b")};
//LPCTSTR PopSocket::GetUserGroup()
//{
//	
//	if(m_strTitle.IsEmpty())
//		return m_strUserGroup;
//	//获取用户权限组描述信息
//	//nFound = -1 未查找状态 
//	//nFound = 0  所有权限
//	//nFound = 1  A组权限
//	//nFound = 2  B组权限
//	int nFound = -1;
//
//	for(int i=0; i<ARRAYSIZE(g_aryAUserGroupDescriptions); ++i)
//	{
//		if(m_strTitle.Find(g_aryAUserGroupDescriptions[i], 0)>=0)
//		{
//			nFound = 1;
//			m_strUserGroup = PRIVILEGE_GROUP_A;
//			break;
//		}
//	}
//	if(nFound<0)
//	{
//		for(int i=0; i<ARRAYSIZE(g_aryBUserGroupDescriptions); ++i)
//		{
//			if(m_strTitle.Find(g_aryBUserGroupDescriptions[i], 0)>=0)
//			{
//				nFound = 2;
//				m_strUserGroup = PRIVILEGE_GROUP_B;
//				break;
//			}
//		}
//	}
//
//	if(nFound<0)
//	{
//		nFound = 0;
//		m_strUserGroup = PRIVILEGE_GROUP_PUBLIC;
//	}
//
//	return m_strUserGroup;
//}

BOOL PopSocket::GetMail(long nMailIndex,long lMailSize,MailAnalysis& mail)
{
	if (!m_hSocket) return FALSE;

	MailLogNormal(_T("[PopSocket::GetMail] [Index=%d] [Size=%d][Title=%s] request mail ......"),nMailIndex,lMailSize,m_strTitle);

	m_strSend.Format("RETR %d\r\n" , nMailIndex);
	if (!Request(TRUE)) return FALSE;
	if (!VerifyRecvData()) return FALSE;

	mail.Clear();
	mail.SetMail(m_strRecv);
	mail.SetFrom(m_strFrom);
	mail.SetTitle(m_strTitle);
	mail.SetDateTime(m_dwDate,m_dwTime);
	mail.SetMessageID(m_strMessageID);
	mail.SetSubject(m_strSubject);

	return TRUE;
}

BOOL PopSocket::GetMail(int nMailIndex,MailAnalysis& mail)
{
	if (!m_hSocket) return FALSE;

	MailLogNormal(_T("[PopSocket::GetMail] [Index=%d] [Title=%s] request mail ......"),nMailIndex,m_strTitle);

	m_strSend.Format("RETR %d\r\n" , nMailIndex);
	if (!Request(TRUE)) return FALSE;
	if (!VerifyRecvData()) return FALSE;

	mail.Clear();
	mail.SetMail(m_strRecv);
	mail.SetFrom(m_strFrom);
	mail.SetTitle(m_strTitle);
	mail.SetDateTime(m_dwDate,m_dwTime);
	mail.SetMessageID(m_strMessageID);

	return TRUE;
}


void PopSocket::Quit()
{
	if (!m_hSocket) return;
	m_strSend.Format("QUIT \r\n");
	Request(FALSE);
	return;
}




