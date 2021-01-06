// SelfStartUp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <Shlobj.h>
#pragma comment(lib, "shell32.lib")
#include "MyTaskSchedule.h"

/**
 * 恶意程序植入后，一般会长久驻留
 * 这就需要程序能够开机自启
 */

 /**
  * 常见的开机自启
  * 修改注册表
  */
BOOL Reg_CurrentUser(const char* lpszFileName, const char* lpszValueName)
{
	//要修改HKEY_LOCAL_MACHINE需要管理员权限
	HKEY hKey;
	//打开注册表键，默认权限，x86程序运行在x64系统时，会有重定位
	if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey))
	{
		return FALSE;
	}
	//修改键值，开机自启
	if (ERROR_SUCCESS != ::RegSetValueEx(hKey, lpszValueName, 0, REG_SZ, (BYTE*)lpszFileName, (1 + ::lstrlen(lpszFileName))))
	{
		::RegCloseKey(hKey);
		return FALSE;
	}
	::RegCloseKey(hKey);
	return TRUE;
}

/**
 * （工具Procmon.exe可以监控系统开机启动过程，寻找切入点）
 * 不修改系统数据
 * 通过快速启动目录来实现
 */
BOOL AutoRun_Startup(const char* lpszSrcFilePath, const char* lpszDestFileName)
{
	BOOL bRet = FALSE;
	char szStartupPath[MAX_PATH] = { 0 };
	char szDestFilePath[MAX_PATH] = { 0 };
	// 获取 快速启动目录 路径
	bRet = ::SHGetSpecialFolderPath(NULL, szStartupPath, CSIDL_STARTUP, TRUE);
	printf("szStartupPath=%s\n", szStartupPath);
	if (FALSE == bRet)
	{
		return FALSE;
	}
	// 构造拷贝的 目的文件路径
	::wsprintf(szDestFilePath, "%s\\%s", szStartupPath, lpszDestFileName);
	// 拷贝文件到快速启动目录下
	bRet = ::CopyFile(lpszSrcFilePath, szDestFilePath, FALSE);
	if (FALSE == bRet)
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * 可通过设定计划任务，触发条件为用户登录时执行启动指定路径程序的操作
 * 设计COM组件接口调用
 */
void test_task()
{
	CMyTaskSchedule task;
	BOOL bRet = FALSE;

	// 创建 任务计划
	bRet = task.NewTask("SSR", "C:\\Games\\ShadowsocksR-win-4.9.0\\ShadowsocksR-dotnet4.0.exe", "", "");
	if (FALSE == bRet)
	{
		printf("Create Task Schedule Error!\n");
	}

	// 暂停
	printf("Create Task Schedule OK!\n");
	system("pause");

	// 卸载 任务计划
	bRet = task.Delete("SSR");
	if (FALSE == bRet)
	{
		printf("Delete Task Schedule Error!\n");
	}

	printf("Delete Task Schedule OK!\n");
	system("pause");
}

/**
 * 将程序作为系统服务，则其会随着系统启动而启动
 * 注册服务程序main入口
 * 之后用ServiceLoader项目中的程序将其加载为系统服务
 */
char g_szServiceName[MAX_PATH] = "ServiceTest.exe";    // 服务名称 
SERVICE_STATUS_HANDLE g_ServiceStatusHandle = { 0 };
BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress)
{
	SERVICE_STATUS serviceStatus = { 0 };
	BOOL bRet = FALSE;

	::RtlZeroMemory(&serviceStatus, sizeof(serviceStatus));
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = dwState;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = dwExitCode;
	serviceStatus.dwWaitHint = 3000;

	bRet = ::SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
	return bRet;
}

void DoTask()
{
	// 自己程序实现部分代码放在这里
}
void __stdcall ServiceCtrlHandle(DWORD dwOperateCode)
{
	switch (dwOperateCode)
	{
	case SERVICE_CONTROL_PAUSE:
	{
		// 暂停
		TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
		TellSCM(SERVICE_PAUSED, 0, 0);
		break;
	}
	case SERVICE_CONTROL_CONTINUE:
	{
		// 继续
		TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
		TellSCM(SERVICE_RUNNING, 0, 0);
		break;
	}
	case SERVICE_CONTROL_STOP:
	{
		// 停止
		TellSCM(SERVICE_STOP_PENDING, 0, 1);
		TellSCM(SERVICE_STOPPED, 0, 0);
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
	g_ServiceStatusHandle = ::RegisterServiceCtrlHandler(g_szServiceName, ServiceCtrlHandle);
	TellSCM(SERVICE_START_PENDING, 0, 1);
	TellSCM(SERVICE_RUNNING, 0, 0);

	// 自己程序实现部分代码放在这里

	// !!注意!! 此处一定要为死循环, 否则在关机再开机的情况(不是点击重启), 不能创建用户进程
	while (TRUE)
	{
		Sleep(5000);
		DoTask();
	}
}

void run_as_service()
{
	SERVICE_TABLE_ENTRY stDispatchTable[] = { {g_szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain},{NULL,NULL} };
	::StartServiceCtrlDispatcher(stDispatchTable);
}

int main()
{
	//测试修改注册表
	//Reg_CurrentUser("C:\\Games\\ShadowsocksR-win-4.9.0\\ShadowsocksR-dotnet4.0.exe","SSR");

	//测试快速启动
	//AutoRun_Startup("C:\\Games\\ShadowsocksR-win-4.9.0\\ShadowsocksR-dotnet4.0.exe", "SSR.exe");

	//测试计划任务
	test_task();

	return 0;
}

