#ifndef _UDP_TEST_H_
#define _UDP_TEST_H_

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>

// 绑定IP地址和端口
BOOL Bind(char *lpszIp, int iPort);
// 数据发送
void SendMsg(char *lpszText, char *lpszIp, int iPort);
// 数据接收
void UDPRecvMsg();
// 接收数据多线程
UINT UDPRecvThreadProc(LPVOID lpVoid);


#endif