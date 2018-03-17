#ifndef TCPCONN_H_
#define TCPCONN_H_

#include "netlib.h"
#include "util.h"
#include "SimpleBuffer.h"

class CTcpServer;

class CTcpConn : public CRefObject
{
public:
	CTcpConn(CTcpServer* server);
	virtual ~CTcpConn();

	// ��������
	int Send(void* data, int len);	
	// �Ƿ�������δ�������
	bool IsBusy();
	// �����ر�����
	void Close();	
	
    // ���º�������Ӧ�ò����
	// �����ر�֪ͨ
	void OnClose();
	// ���ݷ������֪ͨ
	void OnSendCompelete();
	// �յ�����֪ͨ
	void OnRecvData(CSimpleBuffer& buf);
	// ���ݿɶ�֪ͨ
	void OnRead();
	// ���ݿ�д֪ͨ
	void OnWrite();
	// �մ����˶��󣬷����¼��ص����˶���
	void OnConnect(net_handle_t handle);
protected:
	CTcpServer*		m_server;
	net_handle_t	m_handle;
	bool			m_busy;

	std::string		m_peer_ip;
	uint16_t		m_peer_port;
	CSimpleBuffer	m_in_buf;
	CSimpleBuffer	m_out_buf;

	uint32_t		m_last_recv_tick;
	uint32_t		m_last_send_tick;
};
#endif /* TCPCONN_H_ */
