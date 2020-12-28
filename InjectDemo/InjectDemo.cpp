// InjectDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "windows.h"
/**
 * 常见注入方式
 * 全局钩子
 * 远线程猴子
 * 突破session 0隔离的远线程注入
 * APC注入
 */


 /**
  * 全局钩子：主要使用SetWindowsHookEx函数
  * Windows是基于消息队列驱动的，所以使用WH_GETMESSAGE类型钩子
  * 为了将钩子句柄传递给其他进程，使用了共享内存
  * user32.dll导出gSharedInfo全局变量可枚举系统中所有全局钩子信息，PE结构节属性Characteristics若包含IMAGE_SCN_MEM_SHARED标志，则表示该节在内存中共享
  */
#pragma data_seg("s_data") //创建一个数据段,名称必须小于8字节 否则会被截断(这个只要头、尾一致截断后也是一致的，没有什么问题)
HHOOK g_hHook = NULL; //共享数据必须初始化，否则微软编译器会把没有初始化的数据放到.BSS段中，从而导致多个进程之间的共享行为失败。
#pragma data_seg() 
#pragma comment(linker,"/SECTION:s_data,RWS") //将数据段设置为可读/可写/可共享的共享数据段
HMODULE g_hDllModule = NULL;
LRESULT GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	std::cout << "===================hook===================" << std::endl;
	return ::CallNextHookEx(g_hHook, code, wParam, lParam);
}
BOOL SetGlobalHook()
{
	g_hHook = ::SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, g_hDllModule, 0);
	if (g_hHook == NULL)
	{
		return FALSE;
	}
	return TRUE;
}
//卸载钩子
BOOL UnsetGlobalHook()
{
	if (g_hHook)
	{
		::UnhookWindowsHookEx(g_hHook);
	}
	return TRUE;
}
HMODULE GetCurrentModule()
{	//获取当前进程句柄
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule);

	return hModule;
}

/**
 * 远线程注入：一个进程在其他进程中创建线程
 * 通过LoadLibrary函数地址加载DLL
 * Windows中有基址随机化ASLR机制，每次开机系统dll加载基址不同
 * 但例如kernel32.dll的加载基址，要求启动后固定，所以虽然重启后基址不同，在不同进程中，其加载基址是相同的，所以自己程序中获取的地址就是所有进程的
 */
BOOL CreateRemoteThreadInjectDll(DWORD dwProcessId, const char* pszDllFileName)
{
	HANDLE hProcess = NULL;
	DWORD dwSize = 0;
	LPVOID pDllAddr = NULL;
	FARPROC pFuncProcAddr = NULL;
	//获取要注入进程句柄
	hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (NULL == hProcess)
	{
		return FALSE;
	}
	//申请内存
	dwSize = 1 + ::lstrlen(pszDllFileName);
	pDllAddr = ::VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if (pDllAddr == NULL)
	{
		return FALSE;
	}
	//写入数据
	if (FALSE == ::WriteProcessMemory(hProcess, pDllAddr, pszDllFileName, dwSize, NULL))
	{
		return FALSE;
	}
	//获取LoadLibrary函数地址
	//HMODULE kernel32 = ::GetModuleHandle("kernel32.dll");
	//if (kernel32 == NULL)
	//{
	//	return FALSE;
	//}
	//pFuncProcAddr = ::GetProcAddress(kernel32, "LoadLibraryA");
	pFuncProcAddr = ::GetProcAddress(::GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	if (pFuncProcAddr == NULL)
	{
		return FALSE;
	}
	//创建远线程
	HANDLE hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFuncProcAddr, pDllAddr, 0, NULL);
	if (NULL == hRemoteThread)
	{
		return FALSE;
	}
	//关闭句柄
	::CloseHandle(hProcess);
	return TRUE;
}

/**
 * 突破session 0隔离的远线程注入
 * 想注入高权限进程时，由于系统存在session 0安全隔离机制，无法用普通方法远线程注入
 * 可以通过ZwCreateThreadEx方法突破限制，CreateRemoteThread最终其实也是调用这个函数
 * 但通过CreateRemoteThread调用到ZwCreateThreadEx时，CreateThreadFloags值为1，导致线程创建后一直挂起，将此参数置0即可
 */
BOOL ZwCreateThreadExInjectDll(DWORD dwProcessId, const char* pszDllFileName)
{
	HANDLE hProcess = NULL;
	SIZE_T dwSize = 0;
	LPVOID pDllAddr = NULL;
	FARPROC pFuncProcAddr = NULL;
	HANDLE hRemoteThread = NULL;
	DWORD dwStatus = 0;
	// 前面部分一致
	// 打开注入进程，获取进程句柄
	hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (NULL == hProcess)
	{
		return FALSE;
	}
	// 在注入进程中申请内存
	dwSize = 1 + ::lstrlen(pszDllFileName);
	pDllAddr = ::VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == pDllAddr)
	{
		return FALSE;
	}
	// 向申请的内存中写入数据
	if (FALSE == ::WriteProcessMemory(hProcess, pDllAddr, pszDllFileName, dwSize, NULL))
	{
		return FALSE;
	}
	// 加载 ntdll.dll
	HMODULE hNtdllDll = ::LoadLibrary("ntdll.dll");
	if (NULL == hNtdllDll)
	{
		return FALSE;
	}
	// 获取LoadLibraryA函数地址
	pFuncProcAddr = ::GetProcAddress(::GetModuleHandle("Kernel32.dll"), "LoadLibraryA");
	if (NULL == pFuncProcAddr)
	{
		return FALSE;
	}
	// 获取ZwCreateThread函数地址,不同位数参数声明不同
#ifdef _WIN64
	typedef DWORD(WINAPI* typedef_ZwCreateThreadEx)(
		PHANDLE ThreadHandle,
		ACCESS_MASK DesiredAccess,
		LPVOID ObjectAttributes,
		HANDLE ProcessHandle,
		LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter,
		ULONG CreateThreadFlags,
		SIZE_T ZeroBits,
		SIZE_T StackSize,
		SIZE_T MaximumStackSize,
		LPVOID pUnkown);
#else
	typedef DWORD(WINAPI* typedef_ZwCreateThreadEx)(
		PHANDLE ThreadHandle,
		ACCESS_MASK DesiredAccess,
		LPVOID ObjectAttributes,
		HANDLE ProcessHandle,
		LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter,
		BOOL CreateSuspended,
		DWORD dwStackSize,
		DWORD dw1,
		DWORD dw2,
		LPVOID pUnkown);
#endif
	typedef_ZwCreateThreadEx ZwCreateThreadEx = (typedef_ZwCreateThreadEx)::GetProcAddress(hNtdllDll, "ZwCreateThreadEx");
	if (NULL == ZwCreateThreadEx)
	{
		return FALSE;
	}
	// 使用 ZwCreateThreadEx 创建远线程, 实现 DLL 注入
	dwStatus = ZwCreateThreadEx(&hRemoteThread, PROCESS_ALL_ACCESS, NULL, hProcess, (LPTHREAD_START_ROUTINE)pFuncProcAddr, pDllAddr, 0, 0, 0, 0, NULL);
	if (NULL == hRemoteThread)
	{
		return FALSE;
	}
	// 关闭句柄
	::CloseHandle(hProcess);
	::FreeLibrary(hNtdllDll);

	return TRUE;
}

int main()
{
	//g_hDllModule = GetCurrentModule();
	//SetGlobalHook();
	//CreateRemoteThreadInjectDll(18804, "‪C:\\workspaceTools\\filelist\\filelist.dll");
	ZwCreateThreadExInjectDll(6624, "C:\\workspaceTools\\filelist\\filelist.dll");
	std::cout << "Hello World!\n";
	system("pause");
	UnsetGlobalHook();
}

