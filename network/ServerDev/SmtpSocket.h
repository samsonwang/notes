#pragma once
#include <deque>
#include <set>

class SmtpSocket
{
public:
	SmtpSocket(void);
	virtual ~SmtpSocket(void);
	
	BOOL IsConnect() const { return m_hSocket; }
	BOOL Connect(LPCSTR lpUrl,long lPort=25);
	BOOL Logon(LPCSTR lpUser,LPCSTR lpPassword);
	BOOL SendMailFrom(LPCSTR lpFrom);
	BOOL SendMailTo(LPCSTR lpSendTo);
	BOOL SendMail(MailAnalysis& mail);

	void Close();
	void Quit();	//退出

private:
	/*
	功能:
		验证从服务器返回的前三位代码和传递进来的参数是否一样

	备注:
		211 帮助返回系统状态
		214 帮助信息
		220 服务准备就绪
		221 关闭连接
		235 用户验证成功
		250 请求操作就绪
		251 用户不在本地，转寄到其他路径
		334 等待用户输入验证信息
		354 开始邮件输入
		421 服务不可用
		450 操作未执行，邮箱忙
		451 操作中止，本地错误
		452 操作未执行，存储空间不足
		500 命令不可识别或语言错误
		501 参数语法错误
		502 命令不支技
		503 命令顺序错误
		504 命令参数不支持
		550 操作未执行，邮箱不可用
		551 非本地用户
		552 中止存储空间不足
		553 操作未执行，邮箱名不正确
		554 传输失败
	*/
	BOOL CheckResponse(const char* RecvCode);

private:	
		
	CStringA	m_strSend;
	BOOL		m_bConnected;
	SOCKET		m_hSocket;
	char		m_RecvBuffer[1024];
};