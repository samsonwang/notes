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

	// 发送数据
	int Send(void* data, int len);	
	// 是否有数据未发送完成
	bool IsBusy();
	// 主动关闭连接
	void Close();	
	
    // 以下函数不用应用层操作
	// 被动关闭通知
	void OnClose();
	// 数据发送完成通知
	void OnSendCompelete();
	// 收到数据通知
	void OnRecvData(CSimpleBuffer& buf);
	// 数据可读通知
	void OnRead();
	// 数据可写通知
	void OnWrite();
	// 刚创建此对象，分离事件回调到此对象
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
