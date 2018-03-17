#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__
#include "Tcpconn.h"

class CTcpServer
{
public:
	CTcpServer();
	~CTcpServer();
	// 设置监听
	bool Listen(const char*	server_ip, uint16_t	port);
	// 关闭监听
	void Close();	
	// 有客户端连接
	virtual void OnConn(CTcpConn* pConn);
	// 被动关闭通知
	virtual void OnClose(CTcpConn* pConn);
	// 数据发送完成通知
	virtual void OnSendCompelete(CTcpConn* pConn);
	// 收到数据通知
	virtual void OnRecvData(CTcpConn* pConn, CSimpleBuffer& buf);
private:
	net_handle_t m_listen_handle;
};
#endif

