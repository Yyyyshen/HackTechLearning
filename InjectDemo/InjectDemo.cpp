// InjectDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
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
	auto ret = ::GetLastError();//注入程序是64位，则自己的程序也要是64位，否则会注入失败，返回错误码5
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
	//printf("%x", LoadLibraryA);//验证函数地址相同
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
	DWORD ret = GetLastError();
	// 关闭句柄
	::CloseHandle(hProcess);
	::FreeLibrary(hNtdllDll);

	return TRUE;
}

/**
 * APC注入
 * 指函数在特定线程中被异步执行，每个线程都有自己的APC队列使用QueueUserAPC把一个函数压入队列
 * 一个进程包含多个线程，应向目标进程所有线程都插入相同APC函数，保证注入
 */

 // 根据进程名称获取PID
DWORD GetProcessIdByProcessName(const char* pszProcessName)
{
	DWORD dwProcessId = 0;
	PROCESSENTRY32 pe32 = { 0 };
	HANDLE hSnapshot = NULL;
	BOOL bRet = FALSE;
	::RtlZeroMemory(&pe32, sizeof(pe32));//用0填充一块内存
	pe32.dwSize = sizeof(pe32);

	// 获取进程快照
	hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (NULL == hSnapshot)
	{
		return dwProcessId;
	}

	// 获取第一条进程快照信息
	bRet = ::Process32First(hSnapshot, &pe32);
	while (bRet)
	{
		// 获取快照信息
		if (0 == ::lstrcmpi(pe32.szExeFile, pszProcessName))
		{
			dwProcessId = pe32.th32ProcessID;
			break;
		}

		// 遍历下一个进程快照信息
		bRet = ::Process32Next(hSnapshot, &pe32);
	}

	return dwProcessId;
}


// 根据PID获取所有的相应线程ID
BOOL GetAllThreadIdByProcessId(DWORD dwProcessId, DWORD** ppThreadId, DWORD* pdwThreadIdLength)
{
	DWORD* pThreadId = NULL;
	DWORD dwThreadIdLength = 0;
	DWORD dwBufferLength = 1000;
	THREADENTRY32 te32 = { 0 };
	HANDLE hSnapshot = NULL;
	BOOL bRet = TRUE;

	do
	{
		// 申请内存
		pThreadId = new DWORD[dwBufferLength];
		if (NULL == pThreadId)
		{
			bRet = FALSE;
			break;
		}
		::RtlZeroMemory(pThreadId, (dwBufferLength * sizeof(DWORD)));

		// 获取线程快照
		::RtlZeroMemory(&te32, sizeof(te32));
		te32.dwSize = sizeof(te32);
		hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (NULL == hSnapshot)
		{
			bRet = FALSE;
			break;
		}

		// 获取第一条线程快照信息
		bRet = ::Thread32First(hSnapshot, &te32);
		while (bRet)
		{
			// 获取进程对应的线程ID
			if (te32.th32OwnerProcessID == dwProcessId)
			{
				pThreadId[dwThreadIdLength] = te32.th32ThreadID;
				dwThreadIdLength++;
			}

			// 遍历下一个线程快照信息
			bRet = ::Thread32Next(hSnapshot, &te32);
		}

		// 返回
		*ppThreadId = pThreadId;
		*pdwThreadIdLength = dwThreadIdLength;
		bRet = TRUE;

	} while (FALSE);

	if (FALSE == bRet)
	{
		if (pThreadId)
		{
			delete[]pThreadId;
			pThreadId = NULL;
		}
	}

	return bRet;
}

BOOL APCInjectDll(const char* pszProcessName, const char* pszDllName)
{
	BOOL bRet = FALSE;
	DWORD dwProcessId = 0;
	DWORD* pThreadId = NULL;
	DWORD dwThreadIdLength = 0;
	HANDLE hProcess = NULL, hThread = NULL;
	PVOID pBaseAddress = NULL;
	PVOID pLoadLibraryAFunc = NULL;
	SIZE_T dwRet = 0, dwDllPathLen = 1 + ::lstrlen(pszDllName);
	DWORD i = 0;

	do
	{
		// 根据进程名称获取PID
		dwProcessId = GetProcessIdByProcessName(pszProcessName);
		if (0 >= dwProcessId)
		{
			bRet = FALSE;
			break;
		}

		// 根据PID获取所有的相应线程ID
		bRet = GetAllThreadIdByProcessId(dwProcessId, &pThreadId, &dwThreadIdLength);
		if (FALSE == bRet)
		{
			bRet = FALSE;
			break;
		}

		// 打开注入进程
		hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
		if (NULL == hProcess)
		{
			bRet = FALSE;
			break;
		}

		// 在注入进程空间申请内存
		pBaseAddress = ::VirtualAllocEx(hProcess, NULL, dwDllPathLen, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (NULL == pBaseAddress)
		{
			bRet = FALSE;
			break;
		}
		// 向申请的空间中写入DLL路径数据 
		::WriteProcessMemory(hProcess, pBaseAddress, pszDllName, dwDllPathLen, &dwRet);
		if (dwRet != dwDllPathLen)
		{
			bRet = FALSE;
			break;
		}

		// 获取 LoadLibrary 地址
		pLoadLibraryAFunc = ::GetProcAddress(::GetModuleHandle("kernel32.dll"), "LoadLibraryA");
		if (NULL == pLoadLibraryAFunc)
		{
			bRet = FALSE;
			break;
		}

		// 遍历线程, 插入APC
		for (i = 0; i < dwThreadIdLength; i++)
		{
			// 打开线程
			hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, pThreadId[i]);
			if (hThread)
			{
				// 插入APC
				::QueueUserAPC((PAPCFUNC)pLoadLibraryAFunc, hThread, (ULONG_PTR)pBaseAddress);
				// 关闭线程句柄
				::CloseHandle(hThread);
				hThread = NULL;
			}
		}

		bRet = TRUE;

	} while (FALSE);

	// 释放内存
	if (hProcess)
	{
		::CloseHandle(hProcess);
		hProcess = NULL;
	}
	if (pThreadId)
	{
		delete[]pThreadId;
		pThreadId = NULL;
	}

	return bRet;
}

int main()
{
	BOOL bRet = FALSE;
	//g_hDllModule = GetCurrentModule();
	//SetGlobalHook();
	//bRet = CreateRemoteThreadInjectDll(19788, "C:\\workspaceKernel\\HackTechLearning\\x64\\Debug\\TestDll.dll");
	//bRet = ZwCreateThreadExInjectDll(10168, "C:\\workspaceKernel\\HackTechLearning\\x64\\Debug\\TestDll.dll");
	bRet = APCInjectDll("explorer.exe", "C:\\workspaceKernel\\HackTechLearning\\x64\\Debug\\TestDll.dll");
	if (bRet)
	{
		//注入方法执行成功但dll没有启动，调试发现内存中路径前有两个问号,可能是从属性页面复制过来多出来的BOM头，不可见
		//然后导致一直找不到模块，大坑
		std::cout << "Inject OK.\n";
	}
	else
	{
		std::cout << "Inject ERROR.\n";
	}
	//LoadLibraryA("C:\\workspaceKernel\\HackTechLearning\\x64\\Debug\\TestDll.dll");
	////加载dll过程出错，GetLastError 返回126 用单斜杠路径找不到模块，改为双斜杠即可 0x00007ff7738737bd
	//auto error = GetLastError();
	system("pause");
	UnsetGlobalHook();
}

