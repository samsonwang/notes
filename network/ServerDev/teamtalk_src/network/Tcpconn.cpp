#include "tcpconn.h"
#include "TcpServer.h"
typedef hash_map<net_handle_t, CTcpConn*> ConnMap_t;

static ConnMap_t g_tcp_conn_map;
static CTcpConn* FindTcpConn(ConnMap_t* imconn_map, net_handle_t handle)
{
	CTcpConn* pConn = NULL;
	ConnMap_t::iterator iter = imconn_map->find(handle);
	if (iter != imconn_map->end())
	{
		pConn = iter->second;
		pConn->AddRef();
	}

	return pConn;
}

void tcpconn_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	NOTUSED_ARG(handle);
	NOTUSED_ARG(pParam);

	if (!callback_data)
		return;

	ConnMap_t* conn_map = (ConnMap_t*)callback_data;
	CTcpConn* pConn = FindTcpConn(conn_map, handle);
	if (!pConn)
		return;

	//log("msg=%d, handle=%d ", msg, handle);

	switch (msg)
	{	
	case NETLIB_MSG_READ:
		pConn->OnRead();
		break;
	case NETLIB_MSG_WRITE:
		pConn->OnWrite();
		break;
	case NETLIB_MSG_CLOSE:
		pConn->OnClose();
		pConn->Close();
		break;
	default:
		LOG__("!!!imconn_callback error msg: %d ", msg);
		break;
	}

	pConn->ReleaseRef();
}

//////////////////////////
CTcpConn::CTcpConn(CTcpServer* server)
	: m_busy(false)
	, m_handle(NETLIB_INVALID_HANDLE)
	, m_server(server)
{
	m_last_send_tick = m_last_recv_tick = get_tick_count();
}

CTcpConn::~CTcpConn()
{
	//LOG__("CTcpConn::~CTcpConn, handle=%d ", m_handle);
}

bool CTcpConn::IsBusy()
{
	return m_busy;
}


void CTcpConn::Close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_tcp_conn_map.erase(m_handle);
		m_handle = NETLIB_INVALID_HANDLE;
	}
	ReleaseRef();
}

int CTcpConn::Send(void* data, int len)
{
	m_last_send_tick = get_tick_count();

	if (m_busy)
	{
		m_out_buf.Write(data, len);
		return len;
	}

	int offset = 0;
	int remain = len;
	while (remain > 0) {
		int send_size = remain;
		if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) {
			send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
		}

		int ret = netlib_send(m_handle, (char*)data + offset , send_size);
		if (ret <= 0) {
			ret = 0;
			break;
		}

		offset += ret;
		remain -= ret;
	}

	if (remain > 0)
	{
		m_out_buf.Write((char*)data + offset, remain);
		m_busy = true;
		LOG__("send busy, remain=%d ", m_out_buf.GetWriteOffset());
	}
    else
    {
		OnSendCompelete();
    }

	return len;
}


void CTcpConn::OnConnect(net_handle_t handle)
{
	m_handle = handle;
	g_tcp_conn_map.insert(std::make_pair(handle, this));

	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)tcpconn_callback);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_tcp_conn_map);
	netlib_option(handle, NETLIB_OPT_GET_REMOTE_IP, (void*)&m_peer_ip);
	netlib_option(handle, NETLIB_OPT_GET_REMOTE_PORT, (void*)&m_peer_port);

	LOG__("connect from %s:%d, handle=%d", m_peer_ip.c_str(), m_peer_port, m_handle);
}

void CTcpConn::OnRead()
{
	for (;;)
	{
		uint32_t free_buf_len = m_in_buf.GetAllocSize() - m_in_buf.GetWriteOffset();
		if (free_buf_len < READ_BUF_SIZE)
			m_in_buf.Extend(READ_BUF_SIZE);

		int ret = netlib_recv(m_handle, m_in_buf.GetBuffer() + m_in_buf.GetWriteOffset(), READ_BUF_SIZE);
		if (ret <= 0)
			break;

		m_in_buf.IncWriteOffset(ret);
		OnRecvData(m_in_buf);		
		m_last_recv_tick = get_tick_count();
	}
}

void CTcpConn::OnWrite()
{
	if (!m_busy)
		return;

	while (m_out_buf.GetWriteOffset() > 0) {
		int send_size = m_out_buf.GetWriteOffset();
		if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) {
			send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
		}

		int ret = netlib_send(m_handle, m_out_buf.GetBuffer(), send_size);
		if (ret <= 0) {
			ret = 0;
			break;
		}

		m_out_buf.Read(NULL, ret);
	}

	if (m_out_buf.GetWriteOffset() == 0) {
		m_busy = false;
		OnSendCompelete();
	}

	LOG__("onWrite, remain=%d ", m_out_buf.GetWriteOffset());
}

void CTcpConn::OnClose()
{
	if (m_server)
	{
		m_server->OnClose(this);
	}
}

void CTcpConn::OnSendCompelete()
{
	if (m_server)
	{
		m_server->OnSendCompelete(this);
	}
}

void CTcpConn::OnRecvData(CSimpleBuffer& buf)
{
	if (m_server)
	{
		m_server->OnRecvData(this, buf);
	}
}


