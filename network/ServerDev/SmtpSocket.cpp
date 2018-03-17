#include "stdafx.h"
#include "SmtpSocket.h"
#include "LogTrace.h"


using namespace Log;

SmtpSocket::SmtpSocket() 
:m_hSocket(NULL)
{
	
}

SmtpSocket::~SmtpSocket()
{

}

BOOL SmtpSocket::CheckResponse(const char* RecvCode)
{
	try
	{
		memset(m_RecvBuffer,0,sizeof(m_RecvBuffer));
		long lResult=0;
		lResult=recv(m_hSocket,m_RecvBuffer,1024,0);
		if (lResult== SOCKET_ERROR)return FALSE;
		if (lResult<3) return FALSE;
		
		MailLogNormalA("[SmtpSocket::CheckResponse] %s",m_RecvBuffer);
		return RecvCode[0] == m_RecvBuffer[0] && \
			   RecvCode[1] == m_RecvBuffer[1] && \
			   RecvCode[2] == m_RecvBuffer[2] ? TRUE : FALSE;
	}
	catch(...)
	{
		return FALSE;
	}
}

void SmtpSocket::Quit()
{
	if (!m_hSocket) return;

	//退出
	if(send(m_hSocket,"QUIT\r\n",strlen("QUIT\r\n"),0) == SOCKET_ERROR) 
	{
		Close();
		return;
	}
	
	if(!CheckResponse("221")) return;
}


BOOL SmtpSocket::Logon(LPCSTR lpUser,LPCSTR lpPassword)
{
	if (!m_hSocket) return FALSE;
	//发送"AUTH LOGIN"
	if(send(m_hSocket,"AUTH LOGIN\r\n",strlen("AUTH LOGIN\r\n"),0) == SOCKET_ERROR) return FALSE;
	if(!CheckResponse("334")) return FALSE;

	//发送经base64编码的用户名
	char szUserEncoded[64] = {0};
	Base64_Encode(szUserEncoded, lpUser, strlen(lpUser), '=', 64);
	strcat_s(szUserEncoded, 64, "\r\n");
	MailLogNormalA("[SmtpSocket::Logon] Logon [User:%s].",lpUser);
	if(send(m_hSocket,szUserEncoded, strlen(szUserEncoded), 0) == SOCKET_ERROR) return FALSE;
	if(!CheckResponse("334")) return FALSE;

	//发送经base64编码的密码
	//验证密码
	char szPwdEncoded[64] = {0};
	Base64_Encode(szPwdEncoded, lpPassword, strlen(lpPassword), '=', 64);
	strcat_s(szPwdEncoded, 64, "\r\n");
	MailLogNormalA("[SmtpSocket::Logon] Logon [User:%s] [Pass:*****].",lpUser);
	if(send(m_hSocket, szPwdEncoded, strlen(szPwdEncoded),0) == SOCKET_ERROR)return FALSE;
	if(!CheckResponse("235")) return FALSE;

	MailLogNormalA("[SmtpSocket::Logon] Logon [User:%s] successfully",lpUser);
	return TRUE;
}

void SmtpSocket::Close()
{
	if (m_hSocket) 
	{
		closesocket(m_hSocket);
		m_hSocket=NULL;
	}
}

BOOL SmtpSocket::Connect(LPCSTR lpUrl,long lPort)
{
	MailLogNormalA("[SmtpSocket::Connect] Start connect [%s:%d].",lpUrl,lPort);
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
		MailLogErrorA("[SmtpSocket::Connect] Connect [%s:%d] failed.Error=%d",lpUrl,lPort,WSAGetLastError());
		return FALSE;
	}

	if(!CheckResponse("220")) return FALSE;

	//向服务器发送"HELO "+服务器名
	//string strTmp="HELO "+SmtpAddr+"\r\n";
	m_strSend.Format("HELO %s\r\n", lpUrl);
	if(send(m_hSocket, m_strSend.GetString(), m_strSend.GetLength(), 0) == SOCKET_ERROR) return FALSE;
	if(!CheckResponse("250")) return FALSE;

	return TRUE;
}

BOOL SmtpSocket::SendMailFrom(LPCSTR lpFrom)
{
	if (!m_hSocket) return FALSE;
	m_strSend.Format("MAIL FROM:<%s>\r\n", lpFrom);
	if(send(m_hSocket,m_strSend.GetString(),m_strSend.GetLength(),0) == SOCKET_ERROR) return FALSE;
	if(!CheckResponse("250")) return FALSE;

	return TRUE;
}

BOOL SmtpSocket::SendMailTo(LPCSTR lpSendTo)
{
	if (!m_hSocket) return FALSE;
	m_strSend.Format("RCPT To:<%s>\r\n", lpSendTo);
	if(send(m_hSocket,m_strSend.GetString(),m_strSend.GetLength(),0) == SOCKET_ERROR) return FALSE;
	if(!CheckResponse("250")) return FALSE;

	return TRUE;
}

BOOL SmtpSocket::SendMail(MailAnalysis& mail)
{
	if (!m_hSocket) return FALSE;
	CStringA strContent=mail.GetMailData();
	CStringA strSubject=mail.GetSubject();
	CString strMailID;
	CStringA strMailIDA,strMailDate;
	strMailID.Format(_T("Zealink-ID:%s\r\n"),mail.GetMessageID());
	strMailIDA=strMailID;
	strMailDate.Format("Zealink-Date:%d %d\r\n",mail.GetDate(),mail.GetTime());
	strSubject+="\r\n";
	long lPos=strContent.Find("Content-Type:");
	if (lPos>0) strContent=strContent.Mid(lPos);

	strContent.Insert(0,strMailDate);
	strContent.Insert(0,strMailIDA);
	strContent.Insert(0,strSubject);

	DWORD dwOffset=0;
	long lTotal=strContent.GetLength();
	LPCSTR lpSendBuffer=strContent.GetString();

	//发送"DATA\r\n"
	if(send(m_hSocket,"DATA\r\n",strlen("DATA\r\n"),0) == SOCKET_ERROR)return FALSE;
	if(!CheckResponse("354")) return FALSE;

	DWORD dwSend=0;
	long lResult=0;
	const long SEND_MAX_SIZE=1024*100000;
	while((long)dwOffset<lTotal)
	{
		if (lTotal-dwOffset>SEND_MAX_SIZE) dwSend=SEND_MAX_SIZE;
		else dwSend=lTotal-dwOffset;

		lResult=::send(m_hSocket,lpSendBuffer+dwOffset,dwSend,0);
		if (lResult==SOCKET_ERROR) return FALSE;

		dwOffset+=lResult;
	}

	if(!CheckResponse("250")) return FALSE;

	return TRUE;
}
