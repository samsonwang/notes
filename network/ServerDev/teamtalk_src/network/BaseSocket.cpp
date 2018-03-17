#include "BaseSocket.h"
#include "EventDispatch.h"

typedef hash_map<net_handle_t, CBaseSocket*> SocketMap;
SocketMap	g_socket_map;

void AddBaseSocket(CBaseSocket* pSocket)
{
	g_socket_map.insert(std::make_pair((net_handle_t)pSocket->GetSocket(), pSocket));
}

void RemoveBaseSocket(CBaseSocket* pSocket)
{
	g_socket_map.erase((net_handle_t)pSocket->GetSocket());
}

CBaseSocket* FindBaseSocket(net_handle_t fd)
{
	CBaseSocket* pSocket = NULL;
	SocketMap::iterator iter = g_socket_map.find(fd);
	if (iter != g_socket_map.end())
	{
		pSocket = iter->second;
		pSocket->AddRef();
	}

	return pSocket;
}

//////////////////////////////

CBaseSocket::CBaseSocket()
{
	//LOG__("CBaseSocket::CBaseSocket\n");
	m_socket = INVALID_SOCKET;
	m_state = SOCKET_STATE_IDLE;
}

CBaseSocket::~CBaseSocket()
{
	//LOG__("CBaseSocket::~CBaseSocket, socket=%d\n", m_socket);
}

SOCKET CBaseSocket::GetSocket()
{
	return m_socket;
}

void CBaseSocket::SetSocket(SOCKET fd)
{
	m_socket = fd;
}

void CBaseSocket::SetState(BS_SOCKET_STATE state)
{
	m_state = state;
}

void CBaseSocket::SetCallback(callback_t callback)
{
	m_callback = callback;
}

void CBaseSocket::SetCallbackData(void* data)
{
	m_callback_data = data;
}

void CBaseSocket::SetRemoteIP(char* ip)
{
	m_remote_ip = ip;
}

void CBaseSocket::SetRemotePort(uint16_t port)
{
	m_remote_port = port;
}

net_handle_t CBaseSocket::Listen(const char* server_ip, uint16_t port, callback_t callback, void* callback_data)
{
	m_local_ip = server_ip;
	m_local_port = port;
	m_callback = callback;
	m_callback_data = callback_data;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		printf("socket failed, err_code=%d\n", _GetErrorCode());
		return NETLIB_INVALID_HANDLE;
	}

	_SetReuseAddr(m_socket);
	_SetNonblock(m_socket);

	sockaddr_in serv_addr;
	_SetAddr(server_ip, port, &serv_addr);
    int ret = ::bind(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret == SOCKET_ERROR)
	{
		LOG__("bind failed, err_code=%d", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_INVALID_HANDLE;
	}

	ret = listen(m_socket, 64);
	if (ret == SOCKET_ERROR)
	{
		LOG__("listen failed, err_code=%d", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_INVALID_HANDLE;
	}

	m_state = SOCKET_STATE_LISTENING;

	LOG__("CBaseSocket::Listen on %s:%d", server_ip, port);

	AddBaseSocket(this);
	CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_READ | SOCKET_EXCEP);
	return (net_handle_t)m_socket;
}

net_handle_t CBaseSocket::Connect(const char* server_ip, uint16_t port, callback_t callback, void* callback_data)
{
	LOG__("CBaseSocket::Connect, server_ip=%s, port=%d", server_ip, port);

	m_remote_ip = server_ip;
	m_remote_port = port;
	m_callback = callback;
	m_callback_data = callback_data;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		LOG__("socket failed, err_code=%d", _GetErrorCode());
		return NETLIB_INVALID_HANDLE;
	}

	_SetNonblock(m_socket);
	_SetNoDelay(m_socket);
	sockaddr_in serv_addr;
	_SetAddr(server_ip, port, &serv_addr);
	int ret = connect(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if ( (ret == SOCKET_ERROR) && (!_IsBlock(_GetErrorCode())) )
	{	
		LOG__("connect failed, err_code=%d", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_INVALID_HANDLE;
	}
	m_state = SOCKET_STATE_CONNECTING;
	AddBaseSocket(this);
	CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_ALL);
	
	return (net_handle_t)m_socket;
}

int CBaseSocket::Send(void* buf, int len)
{
	if (m_state != SOCKET_STATE_CONNECTED)
		return NETLIB_ERROR;

	int ret = send(m_socket, (char*)buf, len, 0);
	if (ret == SOCKET_ERROR)
	{
		int err_code = _GetErrorCode();
		if (_IsBlock(err_code))
		{
#if ((defined _WIN32) || (defined __APPLE__))	// ֮����linux�²�ע���д�¼���ԭ���linux�汾��CEventDispatch::AddEvent
			CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_WRITE);
#endif
			ret = 0;
		}
		else
		{
			LOG__("!!!send failed, error code: %d", err_code);
		}
	}
	return ret;
}

int CBaseSocket::Recv(void* buf, int len)
{
	return recv(m_socket, (char*)buf, len, 0);
}

int CBaseSocket::Close()
{
	CEventDispatch::Instance()->RemoveEvent(m_socket, SOCKET_ALL);
	RemoveBaseSocket(this);
	closesocket(m_socket);
	ReleaseRef();

	return 0;
}

bool CBaseSocket::Shutdown(BS_SHUTDOWN_TYPE type)
{
	return (0 == shutdown(m_socket, type));
}

void CBaseSocket::OnRead()
{
	if (m_state == SOCKET_STATE_LISTENING)
	{
		_AcceptNewSocket();
	}
	else
	{
		u_long avail = 0;
		if ( (ioctlsocket(m_socket, FIONREAD, &avail) == SOCKET_ERROR) || (avail == 0) )
		{
			m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
		}
		else
		{
			m_callback(m_callback_data, NETLIB_MSG_READ, (net_handle_t)m_socket, NULL);
		}
	}
}

void CBaseSocket::OnWrite()
{
#if ((defined _WIN32) || (defined __APPLE__))	// ֮����linux�²��Ƴ���д�¼���ԭ���CBaseSocket::Send
	CEventDispatch::Instance()->RemoveEvent(m_socket, SOCKET_WRITE);
#endif

	if (m_state == SOCKET_STATE_CONNECTING)
	{
		int error = 0;		
#ifdef _WIN32
		int len = sizeof(error);
		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
#else
		socklen_t len = sizeof(error);
		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (void*)&error, &len);
#endif
		if (error) {
			m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
		} else {
			m_state = SOCKET_STATE_CONNECTED;
			m_callback(m_callback_data, NETLIB_MSG_CONFIRM, (net_handle_t)m_socket, NULL);
		}
	}
	else
	{
		m_callback(m_callback_data, NETLIB_MSG_WRITE, (net_handle_t)m_socket, NULL);
	}
}

void CBaseSocket::OnClose()
{
	m_state = SOCKET_STATE_CLOSING;
	m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
}

void CBaseSocket::SetSendBufSize(uint32_t send_size)
{
#ifdef _WIN32
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&send_size, sizeof(send_size));
#else
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &send_size, sizeof(send_size));
#endif
	
	if (ret == SOCKET_ERROR) {
		LOG__("set SO_SNDBUF failed for fd=%d", m_socket);
	}

	socklen_t len = 4;
	int size = 0;
#ifdef _WIN32
	getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&size, &len);
#else
	getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (void*)&size, &len);
#endif
	
	LOG__("socket=%d send_buf_size=%d", m_socket, size);
}

void CBaseSocket::SetRecvBufSize(uint32_t recv_size)
{
#ifdef _WIN32
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&recv_size, sizeof(recv_size));
#else
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &recv_size, sizeof(recv_size));
#endif	
	if (ret == SOCKET_ERROR) {
		LOG__("set SO_RCVBUF failed for fd=%d", m_socket);
	}

	socklen_t len = 4;
	int size = 0;
#ifdef _WIN32
	getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&size, &len);
#else
	getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (void*)&size, &len);
#endif
	
	LOG__("socket=%d recv_buf_size=%d", m_socket, size);
}

const char* CBaseSocket::GetRemoteIP()
{
	return m_remote_ip.c_str();
}

uint16_t CBaseSocket::GetRemotePort()
{
	return m_remote_port;
}

const char* CBaseSocket::GetLocalIP()
{
	return m_local_ip.c_str();
}

uint16_t CBaseSocket::GetLocalPort()
{
	return m_local_port;
}

int CBaseSocket::_GetErrorCode()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

bool CBaseSocket::_IsBlock(int error_code)
{
#ifdef _WIN32
	return ( (error_code == WSAEINPROGRESS) || (error_code == WSAEWOULDBLOCK) );
#else
	return ( (error_code == EINPROGRESS) || (error_code == EWOULDBLOCK) );
#endif
}

void CBaseSocket::_SetNonblock(SOCKET fd)
{
#ifdef _WIN32
	u_long nonblock = 1;
	int ret = ioctlsocket(fd, FIONBIO, &nonblock);
#else
	int ret = fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));
#endif
	if (ret == SOCKET_ERROR)
	{
		LOG__("_SetNonblock failed, err_code=%d", _GetErrorCode());
	}
}

void CBaseSocket::_SetReuseAddr(SOCKET fd)
{
	int reuse = 1;
	int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
	if (ret == SOCKET_ERROR)
	{
		LOG__("_SetReuseAddr failed, err_code=%d", _GetErrorCode());
	}
}

void CBaseSocket::_SetNoDelay(SOCKET fd)
{
	int nodelay = 1;
	int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
	if (ret == SOCKET_ERROR)
	{
		LOG__("_SetNoDelay failed, err_code=%d", _GetErrorCode());
	}
}

void CBaseSocket::_SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr)
{
	memset(pAddr, 0, sizeof(sockaddr_in));
	pAddr->sin_family = AF_INET;
	pAddr->sin_port = htons(port);
	pAddr->sin_addr.s_addr = inet_addr(ip);
	if (pAddr->sin_addr.s_addr == INADDR_NONE)
	{
		hostent* host = gethostbyname(ip);
		if (host == NULL)
		{
			LOG__("gethostbyname failed, ip=%s", ip);
			return;
		}

		pAddr->sin_addr.s_addr = *(uint32_t*)host->h_addr;
	}
}

void CBaseSocket::_AcceptNewSocket()
{
	SOCKET fd = 0;
	sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(sockaddr_in);
	char ip_str[64];
	while ( (fd = accept(m_socket, (sockaddr*)&peer_addr, &addr_len)) != INVALID_SOCKET )
	{
		CBaseSocket* pSocket = new CBaseSocket();
		uint32_t ip = ntohl(peer_addr.sin_addr.s_addr);
		uint16_t port = ntohs(peer_addr.sin_port);

		snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

		LOG__("AcceptNewSocket, socket=%d from %s:%d\n", fd, ip_str, port);

		pSocket->SetSocket(fd);
		pSocket->SetCallback(m_callback);
		pSocket->SetCallbackData(m_callback_data);
		pSocket->SetState(SOCKET_STATE_CONNECTED);
		pSocket->SetRemoteIP(ip_str);
		pSocket->SetRemotePort(port);

		_SetNoDelay(fd);
		_SetNonblock(fd);
		AddBaseSocket(pSocket);
		CEventDispatch::Instance()->AddEvent(fd, SOCKET_READ | SOCKET_EXCEP);
		m_callback(m_callback_data, NETLIB_MSG_CONNECT, (net_handle_t)fd, NULL);
	}
}

