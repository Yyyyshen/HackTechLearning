#include "ReplaceProcess.h"

// 创建进程并替换进程内存数据, 更改执行顺序
BOOL ReplaceProcess(const char* pszFilePath, PVOID pReplaceData, DWORD dwReplaceDataSize, DWORD dwRunOffset)
{
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	CONTEXT threadContext = { 0 };
	BOOL bRet = FALSE;
	::RtlZeroMemory(&si, sizeof(si));
	::RtlZeroMemory(&pi, sizeof(pi));
	::RtlZeroMemory(&threadContext, sizeof(threadContext));
	si.cb = sizeof(si);
	// 创建进程并挂起主线程
	bRet = ::CreateProcess(pszFilePath, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
	if (FALSE == bRet)
	{
		return FALSE;
	}
	// 读取基址的方法: 此时context中的EBX是指向PEB的指针, 而在PEB偏移是8的位置存放 PEB_LDR_DATA 的地址
	// 我们可以根据 PEB_LDR_DATA 获取进程基址以及遍历进程链获取所有进程
//	DWORD dwProcessBaseAddr = 0;
//	::ReadProcessMemory(pi.hProcess, (LPVOID)(8 + threadContext.Ebx), &dwProcessBaseAddr, sizeof(dwProcessBaseAddr), NULL);

	// 在替换的进程中申请一块内存
	LPVOID lpDestBaseAddr = ::VirtualAllocEx(pi.hProcess, NULL, dwReplaceDataSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (NULL == lpDestBaseAddr)
	{
		return FALSE;
	}
	// 写入替换的数据
	bRet = ::WriteProcessMemory(pi.hProcess, lpDestBaseAddr, pReplaceData, dwReplaceDataSize, NULL);
	if (FALSE == bRet)
	{
		return FALSE;
	}
	// 获取线程上下文
	// 注意此处标志，一定要写!!!
	threadContext.ContextFlags = CONTEXT_FULL;
	bRet = ::GetThreadContext(pi.hThread, &threadContext);
	if (FALSE == bRet)
	{
		return FALSE;
	}
	// 修改进程的PE文件的入口地址以及映像大小,先获取原来进程PE结构的加载基址
	threadContext.Eip = (DWORD)lpDestBaseAddr + dwRunOffset;
	// 设置挂起进程的线程上下文
	bRet = ::SetThreadContext(pi.hThread, &threadContext);
	if (FALSE == bRet)
	{
		return FALSE;
	}
	// 恢复挂起的进程的线程
	::ResumeThread(pi.hThread);

	return TRUE;
}