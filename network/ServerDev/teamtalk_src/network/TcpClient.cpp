#include "TcpClient.h"

void tcpclient_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	NOTUSED_ARG(handle);
	NOTUSED_ARG(pParam);

	if (!callback_data)
		return;

	CTcpClient* pConn = (CTcpClient*)callback_data;

	switch (msg)
	{
	case NETLIB_MSG_CONFIRM:
		pConn->OnConfirm();
		break;
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
		LOG__("!!!tcpclient_callback error msg: %d ", msg);
		break;
	}
}

CTcpClient::CTcpClient()
	: m_busy(false)
	, m_handle(NETLIB_INVALID_HANDLE)
{
}


CTcpClient::~CTcpClient()
{
	Close();
}


bool CTcpClient::Connect(const char* server_ip, uint16_t port)
{
	m_peer_ip = server_ip;
	m_peer_port = port;
	m_handle = netlib_connect(server_ip, port, tcpclient_callback, (void*)this);
	return NETLIB_INVALID_HANDLE != m_handle;
}

bool CTcpClient::IsBusy()
{
	return m_busy;
}


void CTcpClient::Close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		m_handle = NETLIB_INVALID_HANDLE;
	}
}

int CTcpClient::Send(void* data, int len)
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

		int ret = netlib_send(m_handle, (char*)data + offset, send_size);
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

void CTcpClient::OnRead()
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

void CTcpClient::OnWrite()
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

void CTcpClient::OnConfirm()
{
}

void CTcpClient::OnClose()
{
}

void CTcpClient::OnSendCompelete()
{
}

void CTcpClient::OnRecvData(CSimpleBuffer& buf)
{
}

