#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

//#include <Winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>


// 连接到服务器
BOOL Connection(const char* lpszServerIp, int iServerPort);
// 发送数据
void SendMsg(const char* pszSend);
// 创建接收数据多线程
void RecvMsg();
// 接收数据多线程
UINT RecvThreadProc(LPVOID lpVoid);


#endif