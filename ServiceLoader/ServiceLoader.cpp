// ServiceLoader.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "ServiceOperate.h"
/**
 * 将程序以服务启动
 */

int main()
{
	BOOL bRet = FALSE;
	char szExePath[] = "C:\\workspaceKernel\\HackTechLearning\\x64\\Debug\\RunProc.exe";

	// 加载服务
	bRet = SystemServiceOperate(szExePath, 0);
	if (bRet)
	{
		printf("INSTALL OK.\n");
	}
	else
	{
		printf("INSTALL ERROR.\n");
	}
	// 启动服务
	bRet = SystemServiceOperate(szExePath, 1);
	if (bRet)
	{
		printf("START OK.\n");
	}
	else
	{
		printf("START ERROR.\n");
	}


	system("pause");

	// 停止服务
	bRet = SystemServiceOperate(szExePath, 2);
	if (bRet)
	{
		printf("STOP OK.\n");
	}
	else
	{
		printf("STOP ERROR.\n");
	}
	// 卸载服务
	bRet = SystemServiceOperate(szExePath, 3);
	if (bRet)
	{
		printf("UNINSTALL OK.\n");
	}
	else
	{
		printf("UNINSTALL ERROR.\n");
	}

	return 0;
}
