// NetLinking.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

/**
 * 传输技术
 * 之前一直用的boost网络库，正好借这个看下原生接口
 */

 /**
  * TCP
  */
#include "TcpClient.h"
void tcp_client()
{
	// 连接到服务器
	if (FALSE == Connection("127.0.0.1", 12345))
	{
		printf("Connection Error.\n");
		return;
	}
	printf("Connection OK.\n");

	// 发送信息
	char szSendBuf[MAX_PATH] = { 0 };
	while (TRUE)
	{
		gets_s(szSendBuf);
		// 发送数据
		SendMsg(szSendBuf);
	}
}

/**
 * UDP
 * 不需要建立连接，两端bind端口后直接向目标发送数据
 */
#include "UDPTest.h"

void udp_test()
{
	char szIp[MAX_PATH] = { 0 };
	int iPort = 0;
	// 输入程序UDP绑定的IP和端口
	printf("Input IP and Port:\n");
	scanf("%s%d", szIp, &iPort);
	getchar();

	// 绑定地址
	if (FALSE == Bind(szIp, iPort))
	{
		printf("Bind Error.\n");
	}
	printf("Bind OK.\n");

	// 输入发送数据目的主机的IP和端口
	printf("Input Dest IP and Dest Port:\n");
	scanf("%s%d", szIp, &iPort);
	getchar();
	// 发送数据
	char szBuf[MAX_PATH] = { 0 };
	while (TRUE)
	{
		gets_s(szBuf);
		SendMsg(szBuf, szIp, iPort);
	}
}

int main()
{
	//测试tcp连接
	//tcp_client();

	//测试udp
	udp_test();

	return 0;
}

