#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__
#include "netlib.h"
#include "util.h"
#include "SimpleBuffer.h"
class CTcpClient
{
public:
	CTcpClient();
	~CTcpClient();

	// ���ӷ�����
	bool Connect(const char* server_ip, uint16_t port);	
	// ��������
	int Send(void* data, int len);
	// �Ƿ�������δ�������
	bool IsBusy();
	// �����ر�����
	void Close();

	// ���ӳɹ�
	virtual void OnConfirm();
	// �����ر�֪ͨ
	virtual void OnClose();
	// ���ݷ������֪ͨ
	virtual void OnSendCompelete();
	// �յ�����֪ͨ
	virtual void OnRecvData(CSimpleBuffer& buf);

	// ���º�������Ӧ�ò����
	// ���ݿɶ�֪ͨ
	void OnRead();
	// ���ݿ�д֪ͨ
	void OnWrite();	
protected:
	net_handle_t	m_handle;
	bool			m_busy;

	std::string		m_peer_ip;
	uint16_t		m_peer_port;
	CSimpleBuffer	m_in_buf;
	CSimpleBuffer	m_out_buf;

	uint32_t		m_last_recv_tick;
	uint32_t		m_last_send_tick;
};
#endif

