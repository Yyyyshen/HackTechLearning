// NetLinking.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

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

int main()
{
	//测试tcp连接
	tcp_client();

	return 0;
}

