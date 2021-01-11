#include "UDPTest.h"


SOCKET g_sock;


// 绑定IP地址和端口
BOOL Bind(char *lpszIp, int iPort)
{
	// 初始化 Winsock 库
	WSADATA wsaData = { 0 };
	::WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建数据报套接字
	g_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == g_sock)
	{
		return FALSE;
	}
	// 设置绑定IP地址和端口信息
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(iPort);
	addr.sin_addr.S_un.S_addr = ::inet_addr(lpszIp);
	// 绑定IP地址和端口
	if (0 != bind(g_sock, (sockaddr *)(&addr), sizeof(addr)))
	{
		return FALSE;
	}
	// 创建接收信息多线程
	::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)UDPRecvThreadProc, NULL, NULL, NULL);
	return TRUE;
}


// 数据发送
void SendMsg(char *lpszText, char *lpszIp, int iPort)
{
	// 设置目的主机的IP地址和端口等地址信息
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(iPort);
	addr.sin_addr.S_un.S_addr = ::inet_addr(lpszIp);
	// 发送数据到目的主机
	::sendto(g_sock, lpszText, (1 + ::lstrlen(lpszText)), 0, (sockaddr *)(&addr), sizeof(addr));
	printf("[sendto]%s\n", lpszText);
}


// 数据接收
void UDPRecvMsg()
{
	char szBuf[MAX_PATH] = { 0 };
	while (TRUE)
	{
		sockaddr_in addr = { 0 };
		// 注意此处, 既是输入参数也是输出参数
		int iLen = sizeof(addr);   
		// 接收数据
		::recvfrom(g_sock, szBuf, MAX_PATH, 0, (sockaddr *)(&addr), &iLen);
		printf("[recvfrom]%s\n", szBuf);
	}
}


// 接收数据多线程
UINT UDPRecvThreadProc(LPVOID lpVoid)
{
	// 接收数据
	UDPRecvMsg();
	return 0;
}