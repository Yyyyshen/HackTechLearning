#include "ForceKillProcess.h"


VOID ShowError(PCHAR lpszText, NTSTATUS ntStatus)
{
	DbgPrint("%s Error[0x%X]\n", lpszText, ntStatus);
}


// 强制结束指定进程
NTSTATUS ForceKillProcess(HANDLE hProcessId)
{
	PVOID pPspTerminateThreadByPointerAddress = NULL;
	PEPROCESS pEProcess = NULL;
	PETHREAD pEThread = NULL;
	PEPROCESS pThreadEProcess = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG i = 0;

#ifdef _WIN64
	// 64 位
	typedef NTSTATUS(__fastcall *PSPTERMINATETHREADBYPOINTER) (PETHREAD pEThread, NTSTATUS ntExitCode, BOOLEAN bDirectTerminate);
#else
	// 32 位
	typedef NTSTATUS(*PSPTERMINATETHREADBYPOINTER) (PETHREAD pEThread, NTSTATUS ntExitCode, BOOLEAN bDirectTerminate);
#endif

	// 获取 PspTerminateThreadByPointer 函数地址
	pPspTerminateThreadByPointerAddress = GetPspLoadImageNotifyRoutine();
	if (NULL == pPspTerminateThreadByPointerAddress)
	{
		ShowError("GetPspLoadImageNotifyRoutine", 0);
		return FALSE;
	}
	// 获取结束进程的进程结构对象EPROCESS
	status = PsLookupProcessByProcessId(hProcessId, &pEProcess);
	if (!NT_SUCCESS(status))
	{
		ShowError("PsLookupProcessByProcessId", status);
		return status;
	}
	// 遍历所有线程, 并结束所有指定进程的线程
	for (i = 4; i < 0x80000; i = i + 4)
	{
		status = PsLookupThreadByThreadId((HANDLE)i, &pEThread);
		if (NT_SUCCESS(status))
		{
			// 获取线程对应的进程结构对象
			pThreadEProcess = PsGetThreadProcess(pEThread);
			// 结束指定进程的线程
			if (pEProcess == pThreadEProcess)
			{
				((PSPTERMINATETHREADBYPOINTER)pPspTerminateThreadByPointerAddress)(pEThread, 0, 1);
				DbgPrint("PspTerminateThreadByPointer Thread:%d\n", i);
			}
			// 凡是Lookup...，必需Dereference，否则在某些时候会造成蓝屏
			ObDereferenceObject(pEThread);
		}
	}
	// 凡是Lookup...，必需Dereference，否则在某些时候会造成蓝屏
	ObDereferenceObject(pEProcess);

	return status;
}


// 获取 PspTerminateThreadByPointer 函数地址
PVOID GetPspLoadImageNotifyRoutine()
{
	PVOID pPspTerminateThreadByPointerAddress = NULL;
	RTL_OSVERSIONINFOW osInfo = { 0 };
	UCHAR pSpecialData[50] = { 0 };
	ULONG ulSpecialDataSize = 0;

	// 获取系统版本信息, 判断系统版本
	RtlGetVersion(&osInfo);
	if (6 == osInfo.dwMajorVersion)
	{
		if (1 == osInfo.dwMinorVersion)
		{
			// Win7
#ifdef _WIN64
			// 64 位
			// E8
			pSpecialData[0] = 0xE8;
			ulSpecialDataSize = 1;
#else
			// 32 位
			// E8
			pSpecialData[0] = 0xE8;
			ulSpecialDataSize = 1;
#endif	
		}
		else if (2 == osInfo.dwMinorVersion)
		{
			// Win8
#ifdef _WIN64
			// 64 位

#else
			// 32 位

#endif
		}
		else if (3 == osInfo.dwMinorVersion)
		{
			// Win8.1
#ifdef _WIN64
			// 64 位
			// E9
			pSpecialData[0] = 0xE9;
			ulSpecialDataSize = 1;
#else
			// 32 位
			// E8
			pSpecialData[0] = 0xE8;
			ulSpecialDataSize = 1;
#endif			
		}
	}
	else if (10 == osInfo.dwMajorVersion)
	{
		// Win10
#ifdef _WIN64
		// 64 位
		// E9
		pSpecialData[0] = 0xE9;
		ulSpecialDataSize = 1;
#else
		// 32 位
		// E8
		pSpecialData[0] = 0xE8;
		ulSpecialDataSize = 1;
#endif
	}

	// 根据特征码获取地址
	pPspTerminateThreadByPointerAddress = SearchPspTerminateThreadByPointer(pSpecialData, ulSpecialDataSize);
	return pPspTerminateThreadByPointerAddress;
}


// 根据特征码获取 PspTerminateThreadByPointer 数组地址
PVOID SearchPspTerminateThreadByPointer(PUCHAR pSpecialData, ULONG ulSpecialDataSize)
{
	UNICODE_STRING ustrFuncName;
	PVOID pAddress = NULL;
	LONG lOffset = 0;
	PVOID pPsTerminateSystemThread = NULL;
	PVOID pPspTerminateThreadByPointer = NULL;

	// 先获取 PsTerminateSystemThread 函数地址
	RtlInitUnicodeString(&ustrFuncName, L"PsTerminateSystemThread");
	pPsTerminateSystemThread = MmGetSystemRoutineAddress(&ustrFuncName);
	if (NULL == pPsTerminateSystemThread)
	{
		ShowError("MmGetSystemRoutineAddress", 0);
		return pPspTerminateThreadByPointer;
	}

	// 然后, 查找 PspTerminateThreadByPointer 函数地址
	pAddress = SearchMemory(pPsTerminateSystemThread,
		(PVOID)((PUCHAR)pPsTerminateSystemThread + 0xFF),
		pSpecialData, ulSpecialDataSize);
	if (NULL == pAddress)
	{
		ShowError("SearchMemory", 0);
		return pPspTerminateThreadByPointer;
	}

	// 先获取偏移, 再计算地址
	lOffset = *(PLONG)pAddress;
	pPspTerminateThreadByPointer = (PVOID)((PUCHAR)pAddress + sizeof(LONG) + lOffset);

	return pPspTerminateThreadByPointer;
}


// 指定内存区域的特征码扫描
PVOID SearchMemory(PVOID pStartAddress, PVOID pEndAddress, PUCHAR pMemoryData, ULONG ulMemoryDataSize)
{
	PVOID pAddress = NULL;
	PUCHAR i = NULL;
	ULONG m = 0;

	// 扫描内存
	for (i = (PUCHAR)pStartAddress; i < (PUCHAR)pEndAddress; i++)
	{
		// 判断特征码
		for (m = 0; m < ulMemoryDataSize; m++)
		{
			if (*(PUCHAR)(i + m) != pMemoryData[m])
			{
				break;
			}
		}
		// 判断是否找到符合特征码的地址
		if (m >= ulMemoryDataSize)
		{
			// 找到特征码位置, 获取紧接着特征码的下一地址
			pAddress = (PVOID)(i + ulMemoryDataSize);
			break;
		}
	}

	return pAddress;
}