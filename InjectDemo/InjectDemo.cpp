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
 * 
 */


int main()
{
	g_hDllModule = GetCurrentModule();
	SetGlobalHook();
	std::cout << "Hello World!\n";
	system("pause");
	UnsetGlobalHook();
}

