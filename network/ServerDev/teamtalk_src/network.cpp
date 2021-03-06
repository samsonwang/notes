// network.cpp: 定义控制台应用程序的入口点。
//

#include "network/netlib.h"
#include "network/TcpServer.h"
#include "network/TcpClient.h"

class MyTcpServer : public CTcpServer
{
	// 有客户端连接
	void OnConn(CTcpConn* pConn)
	{
		printf("MyTcpServer::OnConn\n");
	}
	// 被动关闭通知
	void OnClose(CTcpConn* pConn)
	{
		printf("MyTcpServer::OnClose\n");
	}
	// 数据发送完成通知
	void OnSendCompelete(CTcpConn* pConn)
	{
		printf("MyTcpServer::OnSendCompelete\n");
	}
	// 收到数据通知
	void OnRecvData(CTcpConn* pConn, CSimpleBuffer& buf)
	{
		printf("MyTcpServer::OnRecvData\n");
		char data[10];
		while (buf.GetWriteOffset() > 0)
		{
			memset(data, 0, 10);
			int nRecv = buf.Read(data, 10);//如果不处理，收到的数据会一直缓存在pBuffer中
			// echo
			pConn->Send(data, nRecv);
		}
	}
};

class CMyClient : public CTcpClient
{
	// 连接成功
	void OnConfirm()
	{
		printf("CMyClient::OnConfirm\n");
		char* tmp = "12345";
		Send(tmp, 5);
	}
	// 被动关闭通知
	void OnClose()
	{
		printf("CMyClient::OnClose\n");
	}
	// 数据发送完成通知
	void OnSendCompelete()
	{
		printf("CMyClient::OnSendCompelete\n");
	}
	// 收到数据通知
	void OnRecvData(CSimpleBuffer& buf)
	{
		printf("CMyClient::OnRecvData\n");		
	}
};
int main()
{
	netlib_init();

	MyTcpServer server;
	server.Listen("127.0.0.1", 60000);	

	CMyClient client;
	client.Connect("127.0.0.1", 60000);

	netlib_eventloop();	

	client.Close();
	server.Close();
	netlib_destroy();
    return 0;
}

