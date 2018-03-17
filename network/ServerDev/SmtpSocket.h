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
	void Quit();	//�˳�

private:
	/*
	����:
		��֤�ӷ��������ص�ǰ��λ����ʹ��ݽ����Ĳ����Ƿ�һ��

	��ע:
		211 ��������ϵͳ״̬
		214 ������Ϣ
		220 ����׼������
		221 �ر�����
		235 �û���֤�ɹ�
		250 �����������
		251 �û����ڱ��أ�ת�ĵ�����·��
		334 �ȴ��û�������֤��Ϣ
		354 ��ʼ�ʼ�����
		421 ���񲻿���
		450 ����δִ�У�����æ
		451 ������ֹ�����ش���
		452 ����δִ�У��洢�ռ䲻��
		500 �����ʶ������Դ���
		501 �����﷨����
		502 ���֧��
		503 ����˳�����
		504 ���������֧��
		550 ����δִ�У����䲻����
		551 �Ǳ����û�
		552 ��ֹ�洢�ռ䲻��
		553 ����δִ�У�����������ȷ
		554 ����ʧ��
	*/
	BOOL CheckResponse(const char* RecvCode);

private:	
		
	CStringA	m_strSend;
	BOOL		m_bConnected;
	SOCKET		m_hSocket;
	char		m_RecvBuffer[1024];
};