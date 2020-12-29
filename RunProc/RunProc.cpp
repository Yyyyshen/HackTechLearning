// RunProc.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <UserEnv.h>
#include <WtsApi32.h>
#pragma comment(lib, "UserEnv.lib")
#pragma comment(lib, "WtsApi32.lib")
/**
 * 恶意程序植入后，一般以模块化启动
 * 这就需要创建进程并启动
 * 同样，需要突破session 0限制
 * 还可以直接加载到内存中运行
 */

/**
 * 创建进程API
 */
BOOL WinExec_Test(const char* pszExePath, UINT uiCmdShow)
{
	UINT uiRet = 0;
	uiRet = ::WinExec(pszExePath, uiCmdShow);
	if (31 < uiRet)
	{
		return TRUE;
	}
	return FALSE;
}


BOOL ShellExecute_Test(const char* pszExePath, UINT uiCmdShow)
{
	HINSTANCE hInstance = 0;
	hInstance = ::ShellExecute(NULL, NULL, pszExePath, NULL, NULL, uiCmdShow);
	if (32 < (DWORD)hInstance)
	{
		return TRUE;
	}
	return FALSE;
}


BOOL CreateProcess_Test(const char* pszExePath, UINT uiCmdShow)
{
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi;
	BOOL bRet = FALSE;
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;  //指定wShowWindow成员有效
	si.wShowWindow = uiCmdShow;
	bRet = ::CreateProcess(NULL, (LPSTR)pszExePath, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	if (bRet)
	{
		//不使用的句柄最好关掉
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
		return TRUE;
	}
	return FALSE;
}

/**
 * 恶意程序常需要把自己注入到系统进程或者伪装成系统服务进程
 * 则应该运行于Session 0会话中
 * Vista版本开始，只有服务托管于session 0中，服务中不能直接CreateProcess创建进程
 * 微软提供了一些WTS开头的函数来负责服务与应用层的交互
 */
BOOL CreateUserProcess(const char* lpszFileName)
{
	//此方法必须在session 0的进程中使用，所以测试时需要加载为服务程序
	BOOL bRet = TRUE;
	DWORD dwSessionID = 0;
	HANDLE hToken = NULL;
	HANDLE hDuplicatedToken = NULL;
	LPVOID lpEnvironment = NULL;
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);

	do
	{
		// 获得当前Session ID
		dwSessionID = ::WTSGetActiveConsoleSessionId();

		// 获得当前Session的用户令牌
		if (FALSE == ::WTSQueryUserToken(dwSessionID, &hToken))
		{
			bRet = FALSE;
			break;
		}

		// 复制令牌
		if (FALSE == ::DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
			SecurityIdentification, TokenPrimary, &hDuplicatedToken))
		{
			bRet = FALSE;
			break;
		}

		// 创建用户Session环境
		if (FALSE == ::CreateEnvironmentBlock(&lpEnvironment,
			hDuplicatedToken, FALSE))
		{
			bRet = FALSE;
			break;
		}

		// 在复制的用户Session下执行应用程序，创建进程
		if (FALSE == ::CreateProcessAsUser(hDuplicatedToken,
			lpszFileName, NULL, NULL, NULL, FALSE,
			NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
			lpEnvironment, NULL, &si, &pi))
		{
			bRet = FALSE;
			break;
		}

	} while (FALSE);
	// 关闭句柄, 释放资源
	if (lpEnvironment)
	{
		::DestroyEnvironmentBlock(lpEnvironment);
	}
	if (hDuplicatedToken)
	{
		::CloseHandle(hDuplicatedToken);
	}
	if (hToken)
	{
		::CloseHandle(hToken);
	}
	return bRet;
}
//以服务类程序启动的入口函数，并做一些测试动作
// 全局变量
char g_szServiceName[MAX_PATH] = "CreateProcessAsUser_Test.exe";    // 服务名称 
SERVICE_STATUS g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_ServiceStatusHandle = { 0 };
void ShowMessage(const char* lpszMessage, const char* lpszTitle)
{
	// 获取当前的Session ID
	DWORD dwSessionId = ::WTSGetActiveConsoleSessionId();
	// 显示消息对话框
	DWORD dwResponse = 0;
	::WTSSendMessage(WTS_CURRENT_SERVER_HANDLE, dwSessionId,
		(LPSTR)lpszTitle, (1 + ::lstrlen(lpszTitle)),
		(LPSTR)lpszMessage, (1 + ::lstrlen(lpszMessage)),
		0, 0, &dwResponse, FALSE);
}
void DoTask()
{
	// 自己程序实现部分代码放在这里
	// 显示对话框
	ShowMessage("Hi Demon·Gan\nThis Is From Session 0 Service!\n", "HELLO");
	// 创建用户桌面进程
	CreateUserProcess("C:\\workspaceTest\\PEview\\PEview.exe");
}
void __stdcall ServiceCtrlHandle(DWORD dwOperateCode)
{
	switch (dwOperateCode)
	{
	case SERVICE_CONTROL_PAUSE:
	{
		// 暂停
		g_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	}
	case SERVICE_CONTROL_CONTINUE:
	{
		// 继续
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	}
	case SERVICE_CONTROL_STOP:
	{
		// 停止
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwCheckPoint = 0;
		g_ServiceStatus.dwWaitHint = 0;
		::SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus);
		break;
	}
	case SERVICE_CONTROL_INTERROGATE:
	{
		// 询问
		break;
	}
	default:
		break;
	}
}
void __stdcall ServiceMain(DWORD dwArgc, char* lpszArgv)
{
	g_ServiceStatus.dwServiceType = SERVICE_WIN32;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;
	g_ServiceStatus.dwWaitHint = 0;

	g_ServiceStatusHandle = ::RegisterServiceCtrlHandler(g_szServiceName, ServiceCtrlHandle);

	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwCheckPoint = 0;
	g_ServiceStatus.dwWaitHint = 0;
	::SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus);

	// 自己程序实现部分代码放在这里
	DoTask();
}

int main()
{
	//测试普通应用程序启动进程方式
	//BOOL bRet = FALSE;
	//bRet = WinExec_Test("C:\\workspaceTest\\PEview\\PEview.exe", SW_SHOWNORMAL);
	//if (bRet)
	//{
	//	printf("WinExec_Test Run OK.\n");
	//}
	//else
	//{
	//	printf("WinExec_Test Run ERROR.\n");
	//}
	//bRet = ShellExecute_Test("C:\\workspaceTest\\PEview\\PEview.exe", SW_SHOWNORMAL);
	//if (bRet)
	//{
	//	printf("ShellExecute_Test Run OK.\n");
	//}
	//else
	//{
	//	printf("ShellExecute_Test Run ERROR.\n");
	//}
	//bRet = CreateProcess_Test("C:\\workspaceTest\\PEview\\PEview.exe", SW_SHOWNORMAL);
	//if (bRet)
	//{
	//	printf("CreateProcess_Test Run OK.\n");
	//}
	//else
	//{
	//	printf("CreateProcess_Test Run ERROR.\n");
	//}

	//system("pause");

	//测试服务类程序启动进程方式
	//注册函数入口为服务类程序的函数入口
	SERVICE_TABLE_ENTRY stDispatchTable[] = { { g_szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain }, { NULL, NULL } };
	::StartServiceCtrlDispatcher(stDispatchTable);

	return 0;
}

