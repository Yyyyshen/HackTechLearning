#include "TcpServer.h"


// 服务端套接字
SOCKET g_ServerSocket;
// 客户端套接字
SOCKET g_ClientSocket;


// 绑定端口并监听
BOOL SocketBindAndListen(const char *lpszIp, int iPort)
{
	// 初始化 Winsock 库
	WSADATA wsaData = {0};
	::WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建流式套接字
	g_ServerSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == g_ServerSocket)
	{
		return FALSE;
	}
	// 设置服务端地址和端口信息
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(iPort);
	//addr.sin_addr.S_un.S_addr = ::inet_addr(lpszIp);
	inet_pton(AF_INET, lpszIp, (void*)&addr.sin_addr.S_un.S_addr);
	// 绑定IP和端口
	if (0 != ::bind(g_ServerSocket, (sockaddr *)(&addr), sizeof(addr)))
	{
		return FALSE;
	}
	// 设置监听
	if (0 != ::listen(g_ServerSocket, 1))
	{
		return FALSE;
	}

	// 创建接收数据多线程
	::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)RecvThreadProc, NULL, NULL, NULL);

	return TRUE;
}


// 发送数据
void SendMsg(const char *pszSend)
{
	// 发送数据
	::send(g_ClientSocket, pszSend, (1 + ::lstrlen(pszSend)), 0);
	printf("[send]%s\n", pszSend);
}


// 接受连接请求 并 接收数据
void AcceptRecvMsg()
{
	sockaddr_in addr = { 0 };
	// 注意：该变量既是输入也是输出
	int iLen = sizeof(addr);   
	// 接受来自客户端的连接请求
	g_ClientSocket = ::accept(g_ServerSocket, (sockaddr *)(&addr), &iLen);
	printf("accept a connection from client!\n");

	char szBuf[MAX_PATH] = { 0 };
	while (TRUE)
	{
		// 接收数据
		int iRet = ::recv(g_ClientSocket, szBuf, MAX_PATH, 0);
		if (0 >= iRet)
		{
			continue;
		}
		printf("[recv]%s\n", szBuf);
		SendMsg(szBuf);
	}
}


// 接收数据多线程
UINT RecvThreadProc(LPVOID lpVoid) 
{
	// 接受连接请求 并 接收数据
	AcceptRecvMsg();
	return 0;
}
