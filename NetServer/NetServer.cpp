// NetServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

/**
 * 各网络连接方式的服务端
 */

 /**
  * TCP
  */
#include "TcpServer.h"
void tcp_server()
{
	// 创建套接字并绑定地址端口进行监听
	if (FALSE == SocketBindAndListen("127.0.0.1", 12345))
	{
		printf("SocketBindAndListen Error.\n");
		return;
	}
	printf("SocketBindAndListen OK.\n");

	// 发送信息
	char szSendBuf[MAX_PATH] = { 0 };
	while (TRUE)
	{
		gets_s(szSendBuf);
		// 发送数据
		SendMsg(szSendBuf);
	}
}

int main()
{
	tcp_server();

	return 0;
}
