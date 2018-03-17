#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__
#include "Tcpconn.h"

class CTcpServer
{
public:
	CTcpServer();
	~CTcpServer();
	// ���ü���
	bool Listen(const char*	server_ip, uint16_t	port);
	// �رռ���
	void Close();	
	// �пͻ�������
	virtual void OnConn(CTcpConn* pConn);
	// �����ر�֪ͨ
	virtual void OnClose(CTcpConn* pConn);
	// ���ݷ������֪ͨ
	virtual void OnSendCompelete(CTcpConn* pConn);
	// �յ�����֪ͨ
	virtual void OnRecvData(CTcpConn* pConn, CSimpleBuffer& buf);
private:
	net_handle_t m_listen_handle;
};
#endif

