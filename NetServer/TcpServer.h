#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_


//#include <Winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>

// 绑定端口并监听
BOOL SocketBindAndListen(const char *lpszIp, int iPort);
// 发送数据
void SendMsg(const char *pszSend);
// 创建接收数据多线程
void AcceptRecvMsg();
// 接收数据多线程
UINT RecvThreadProc(LPVOID lpVoid);


#endif