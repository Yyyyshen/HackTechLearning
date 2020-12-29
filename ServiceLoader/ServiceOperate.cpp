#include "ServiceOperate.h"


void ShowError(const char *lpszText)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error!\nError Code Is:%d\n", lpszText, ::GetLastError());
#ifdef _DEBUG
	::MessageBox(NULL, szErr, "ERROR", MB_OK | MB_ICONERROR);
#endif
}


// 0 加载服务    1 启动服务    2 停止服务    3 删除服务
BOOL SystemServiceOperate(char *lpszExePath, int iOperateType)
{
	BOOL bRet = TRUE;
	char szName[MAX_PATH] = { 0 };

	::lstrcpy(szName, lpszExePath);
	// 过滤掉文件目录，获取文件名
	::PathStripPath(szName);                   

	SC_HANDLE shOSCM = NULL, shCS = NULL;
	SERVICE_STATUS ss;
	DWORD dwErrorCode = 0;
	BOOL bSuccess = FALSE;
	// 打开服务控制管理器数据库
	shOSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!shOSCM)
	{
		ShowError("OpenSCManager");
		return FALSE;
	}

	if (0 != iOperateType)
	{
		// 打开一个已经存在的服务
		shCS = OpenService(shOSCM, szName, SERVICE_ALL_ACCESS);
		if (!shCS)
		{
			ShowError("OpenService");
			::CloseServiceHandle(shOSCM);
			shOSCM = NULL;
			return FALSE;
		}
	}

	switch (iOperateType)
	{
	case 0:
	{
		// 创建服务
		// SERVICE_AUTO_START   随系统自动启动
		// SERVICE_DEMAND_START 手动启动
		shCS = ::CreateService(shOSCM, szName, szName,
			SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
			SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,
			lpszExePath, NULL, NULL, NULL, NULL, NULL);
		if (!shCS)
		{
			ShowError("CreateService");
			bRet = FALSE;
		}
		break;
	}
	case 1:
	{
		// 启动服务
		if (!::StartService(shCS, 0, NULL))
		{
			ShowError("StartService");
			bRet = FALSE;
		}
		break;
	}
	case 2:
	{
		// 停止服务
		if (!::ControlService(shCS, SERVICE_CONTROL_STOP, &ss))
		{
			ShowError("ControlService");
			bRet = FALSE;
		}
		break;
	}
	case 3:
	{
		// 删除服务
		if (!::DeleteService(shCS))
		{
			ShowError("DeleteService");
			bRet = FALSE;
		}
		break;
	}
	default:
		break;
	}
	// 关闭句柄
	if (shCS)
	{
		::CloseServiceHandle(shCS);
		shCS = NULL;
	}
	if (shOSCM)
	{
		::CloseServiceHandle(shOSCM);
		shOSCM = NULL;
	}

	return bRet;
}