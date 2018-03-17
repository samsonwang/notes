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

	// 连接服务器
	bool Connect(const char* server_ip, uint16_t port);	
	// 发送数据
	int Send(void* data, int len);
	// 是否有数据未发送完成
	bool IsBusy();
	// 主动关闭连接
	void Close();

	// 连接成功
	virtual void OnConfirm();
	// 被动关闭通知
	virtual void OnClose();
	// 数据发送完成通知
	virtual void OnSendCompelete();
	// 收到数据通知
	virtual void OnRecvData(CSimpleBuffer& buf);

	// 以下函数不用应用层操作
	// 数据可读通知
	void OnRead();
	// 数据可写通知
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

