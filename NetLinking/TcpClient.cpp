#include "TcpClient.h"

// 客户端套接字
SOCKET g_ClientSocket;


// 连接到服务器
BOOL Connection(const char* lpszServerIp, int iServerPort)
{
	// 初始化 Winsock 库
	WSADATA wsaData = { 0 };
	::WSAStartup(MAKEWORD(2, 2), &wsaData);
	// 创建流式套接字
	g_ClientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == g_ClientSocket)
	{
		return FALSE;
	}
	// 设置服务端地址和端口信息
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(iServerPort);
	//addr.sin_addr.S_un.S_addr = ::inet_addr(lpszServerIp); //过时函数
	inet_pton(AF_INET, lpszServerIp, (void*)&addr.sin_addr.S_un.S_addr);
	// 连接到服务器
	if (0 != ::connect(g_ClientSocket, (sockaddr*)(&addr), sizeof(addr)))
	{
		return FALSE;
	}
	// 创建接收数据多线程
	::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)RecvThreadProc, NULL, NULL, NULL);

	return TRUE;
}


// 发送数据
void SendMsg(const char* pszSend)
{
	// 发送数据
	::send(g_ClientSocket, pszSend, (1 + ::lstrlen(pszSend)), 0);
	printf("[send]%s\n", pszSend);
}


// 接收数据
void RecvMsg()
{
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
	}
}


// 接收数据多线程
UINT RecvThreadProc(LPVOID lpVoid)
{
	// 接受连接请求 并 接收数据
	RecvMsg();
	return 0;
}
