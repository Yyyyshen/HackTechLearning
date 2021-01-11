#include "EnumInfo.h"


void ShowError(const char* lpszText)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error[%d]\n", lpszText, ::GetLastError());
#ifdef _DEBUG
	::MessageBox(NULL, szErr, "ERROR", MB_OK);
#endif
}


BOOL EnumProcess()
{
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);
	// 获取全部进程快照
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hProcessSnap)
	{
		ShowError("CreateToolhelp32Snapshot");
		return FALSE;
	}

	// 获取快照中第一条信息
	BOOL bRet = ::Process32First(hProcessSnap, &pe32);
	while (bRet)
	{
		// 显示 Process ID
		printf("[%d]\t", pe32.th32ProcessID);

		// 显示 进程名称
		printf("[%s]\n", pe32.szExeFile);

		// 获取快照中下一条信息
		bRet = ::Process32Next(hProcessSnap, &pe32);
	}

	// 关闭句柄
	::CloseHandle(hProcessSnap);

	return TRUE;
}


BOOL EnumThread()
{
	THREADENTRY32 te32 = { 0 };
	te32.dwSize = sizeof(THREADENTRY32);
	// 获取全部线程快照
	HANDLE hThreadSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (INVALID_HANDLE_VALUE == hThreadSnap)
	{
		ShowError("CreateToolhelp32Snapshot");
		return FALSE;
	}

	// 获取快照中第一条信息
	BOOL bRet = ::Thread32First(hThreadSnap, &te32);
	while (bRet)
	{
		// 显示 Owner Process ID
		printf("[%d]\t", te32.th32OwnerProcessID);

		// 显示 Thread ID
		printf("[%d]\n", te32.th32ThreadID);

		// 获取快照中下一条信息
		bRet = ::Thread32Next(hThreadSnap, &te32);
	}

	// 关闭句柄
	::CloseHandle(hThreadSnap);

	return TRUE;
}


BOOL EnumProcessModule(DWORD dwProcessId)
{
	MODULEENTRY32 me32 = { 0 };
	me32.dwSize = sizeof(MODULEENTRY32);
	// 获取指定进程全部模块的快照
	HANDLE hModuleSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
	if (INVALID_HANDLE_VALUE == hModuleSnap)
	{
		ShowError("CreateToolhelp32Snapshot");
		return FALSE;
	}

	// 获取快照中第一条信息
	BOOL bRet = ::Module32First(hModuleSnap, &me32);
	while (bRet)
	{
		// 显示 Process ID
		printf("[%d]\t", me32.th32ProcessID);

		// 显示 模块加载基址
		printf("[0x%p]\t", me32.modBaseAddr);

		// 显示 模块名称
		printf("[%s]\n", me32.szModule);

		// 获取快照中下一条信息
		bRet = ::Module32Next(hModuleSnap, &me32);
	}

	// 关闭句柄
	::CloseHandle(hModuleSnap);

	return TRUE;
}