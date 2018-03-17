/*
 *  a wrap for non-block socket class for Windows, LINUX and MacOS X platform
 */

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "ostype.h"
#include "util.h"

enum BS_SOCKET_STATE
{
	SOCKET_STATE_IDLE,
	SOCKET_STATE_LISTENING,
	SOCKET_STATE_CONNECTING,
	SOCKET_STATE_CONNECTED,
	SOCKET_STATE_CLOSING
};

enum BS_SHUTDOWN_TYPE
{ 
	shutdown_recv = 0,
	shutdown_send = 1,
	shutdown_both = 2
};

class CBaseSocket : public CRefObject
{
public:
	CBaseSocket();
	
	virtual ~CBaseSocket();

	SOCKET GetSocket();
	void SetSocket(SOCKET fd);
	void SetState(BS_SOCKET_STATE state);

	void SetCallback(callback_t callback);
	void SetCallbackData(void* data);
	void SetRemoteIP(char* ip);
	void SetRemotePort(uint16_t port);
	void SetSendBufSize(uint32_t send_size);
	void SetRecvBufSize(uint32_t recv_size);

	const char*	GetRemoteIP();
	uint16_t	GetRemotePort();
	const char*	GetLocalIP();
	uint16_t	GetLocalPort();
public:
	net_handle_t Listen(
		const char*		server_ip, 
		uint16_t		port,
		callback_t		callback,
		void*			callback_data);

	net_handle_t Connect(
		const char*		server_ip, 
		uint16_t		port,
		callback_t		callback,
		void*			callback_data);

	int Send(void* buf, int len);

	int Recv(void* buf, int len);

	int Close();
	
	bool Shutdown(BS_SHUTDOWN_TYPE nHow = shutdown_recv);

public:	
	void OnRead();
	void OnWrite();
	void OnClose();

private:	
	int _GetErrorCode();
	bool _IsBlock(int error_code);

	void _SetNonblock(SOCKET fd);
	void _SetReuseAddr(SOCKET fd);
	void _SetNoDelay(SOCKET fd);
	void _SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr);

	void _AcceptNewSocket();

private:
	std::string		m_remote_ip;
	uint16_t		m_remote_port;
	std::string		m_local_ip;
	uint16_t		m_local_port;

	callback_t		m_callback;
	void*			m_callback_data;

	BS_SOCKET_STATE	m_state;
	SOCKET			m_socket;
};

CBaseSocket* FindBaseSocket(net_handle_t fd);

#endif
