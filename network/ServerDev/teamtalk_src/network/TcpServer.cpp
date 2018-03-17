#include "TcpServer.h"
#include "netlib.h"

void tcp_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	CTcpServer* pTcpServer = (CTcpServer*)callback_data;
	if (!pTcpServer)
	{
		return;
	}

	if (msg == NETLIB_MSG_CONNECT)
	{
		CTcpConn* pConn = new CTcpConn(pTcpServer);
		// 分离事件
		pConn->OnConnect(handle);
		// 通知应用层
		pTcpServer->OnConn(pConn);
	}
	else
	{
		LOG__("!!!error msg: %d", msg);
	}
}

CTcpServer::CTcpServer()
	: m_listen_handle(NETLIB_INVALID_HANDLE)
{
}


CTcpServer::~CTcpServer()
{
	Close();
}

bool CTcpServer::Listen(const char* server_ip, uint16_t port)
{	
	m_listen_handle = netlib_listen(server_ip, port, tcp_serv_callback, this);
	return (NETLIB_INVALID_HANDLE != m_listen_handle);
}

void CTcpServer::Close()
{
	if (NETLIB_INVALID_HANDLE != m_listen_handle)
	{
		netlib_close(m_listen_handle);
		m_listen_handle = NETLIB_INVALID_HANDLE;
	}	
}

void CTcpServer::OnConn(CTcpConn* pConn)
{
	
}

void CTcpServer::OnClose(CTcpConn* pConn)
{

}

void CTcpServer::OnSendCompelete(CTcpConn* pConn)
{

}

void CTcpServer::OnRecvData(CTcpConn* pConn, CSimpleBuffer& buf)
{

}
