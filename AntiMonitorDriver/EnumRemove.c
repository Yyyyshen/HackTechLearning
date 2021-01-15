#include "EnumRemove.h"


VOID ShowError(PCHAR lpszText, NTSTATUS ntStatus)
{
	DbgPrint("%s Error[0x%X]\n", lpszText, ntStatus);
}


// 遍历回调
BOOLEAN EnumNotifyRoutine()
{
	ULONG i = 0;
	PVOID pPspCreateProcessNotifyRoutineAddress = NULL;
	PVOID pNotifyRoutineAddress = NULL;

	// 获取 PspCreateProcessNotifyRoutine 数组地址
	pPspCreateProcessNotifyRoutineAddress = GetPspCreateProcessNotifyRoutine();
	if (NULL == pPspCreateProcessNotifyRoutineAddress)
	{
		DbgPrint("GetPspCreateProcessNotifyRoutine Error!\n");
		return FALSE;
	}
	DbgPrint("pPspCreateProcessNotifyRoutineAddress=0x%p\n", pPspCreateProcessNotifyRoutineAddress);

	// 获取回调地址并解密
#ifdef _WIN64
	for (i = 0; i < 64; i++)
	{
		pNotifyRoutineAddress = *(PVOID *)((PUCHAR)pPspCreateProcessNotifyRoutineAddress + sizeof(PVOID) * i);
		pNotifyRoutineAddress = (PVOID)((ULONG64)pNotifyRoutineAddress & 0xfffffffffffffff8);
		if (MmIsAddressValid(pNotifyRoutineAddress))
		{
			pNotifyRoutineAddress = *(PVOID *)pNotifyRoutineAddress;
			DbgPrint("[%d]ullNotifyRoutine=0x%p\n", i, pNotifyRoutineAddress);
		}
	}
#else
	for (i = 0; i < 8; i++)
	{
		pNotifyRoutineAddress = *(PVOID *)((PUCHAR)pPspCreateProcessNotifyRoutineAddress + sizeof(PVOID) * i);
		pNotifyRoutineAddress = (PVOID)((ULONG)pNotifyRoutineAddress & 0xfffffff8);
		if (MmIsAddressValid(pNotifyRoutineAddress))
		{
			pNotifyRoutineAddress = *(PVOID *)((PUCHAR)pNotifyRoutineAddress + 4);
			DbgPrint("[%d]ullNotifyRoutine=0x%p\n", i, pNotifyRoutineAddress);
		}
	}
#endif

	return TRUE;
}


// 移除回调
NTSTATUS RemoveNotifyRoutine(PVOID pNotifyRoutineAddress)
{
	NTSTATUS status = PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)pNotifyRoutineAddress, TRUE);
	if (!NT_SUCCESS(status))
	{
		ShowError("PsSetCreateProcessNotifyRoutine", status);
	}
	return status;
}


// 获取 PspCreateProcessNotifyRoutine 数组地址
PVOID GetPspCreateProcessNotifyRoutine()
{
	PVOID pPspCreateProcessNotifyRoutineAddress = NULL;
	RTL_OSVERSIONINFOW osInfo = { 0 };
	UCHAR pFirstSpecialData[50] = { 0 };
	ULONG ulFirstSpecialDataSize = 0;
	UCHAR pSecondSpecialData[50] = { 0 };
	ULONG ulSecondSpecialDataSize = 0;

	// 获取系统版本信息, 判断系统版本
	RtlGetVersion(&osInfo);
	if (6 == osInfo.dwMajorVersion)
	{
		if (1 == osInfo.dwMinorVersion)
		{
			// Win7
#ifdef _WIN64
			// 64 位
			// E9
			pFirstSpecialData[0] = 0xE9;
			ulFirstSpecialDataSize = 1;
			// 4C8D35
			pSecondSpecialData[0] = 0x4C;
			pSecondSpecialData[1] = 0x8D;
			pSecondSpecialData[2] = 0x35;
			ulSecondSpecialDataSize = 3;
#else
			// 32 位
			// E8
			pFirstSpecialData[0] = 0xE8;
			ulFirstSpecialDataSize = 1;
			// C7450C
			pSecondSpecialData[0] = 0xC7;
			pSecondSpecialData[1] = 0x45;
			pSecondSpecialData[2] = 0x0C;
			ulSecondSpecialDataSize = 3;
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
			pFirstSpecialData[0] = 0xE9;
			ulFirstSpecialDataSize = 1;
			// 4C8D3D
			pSecondSpecialData[0] = 0x4C;
			pSecondSpecialData[1] = 0x8D;
			pSecondSpecialData[2] = 0x3D;
			ulSecondSpecialDataSize = 3;
#else
			// 32 位
			// E8
			pFirstSpecialData[0] = 0xE8;
			ulFirstSpecialDataSize = 1;
			// B8
			pSecondSpecialData[0] = 0xB8;
			ulSecondSpecialDataSize = 1;
#endif			
		}
	}
	else if (10 == osInfo.dwMajorVersion)
	{
		// Win10
#ifdef _WIN64
		// 64 位
		// E9
		pFirstSpecialData[0] = 0xE9;
		ulFirstSpecialDataSize = 1;
		// 4C8D3D
		pSecondSpecialData[0] = 0x4C;
		pSecondSpecialData[1] = 0x8D;
		pSecondSpecialData[2] = 0x3D;
		ulSecondSpecialDataSize = 3;
#else
		// 32 位
		// E8
		pFirstSpecialData[0] = 0xE8;
		ulFirstSpecialDataSize = 1;
		// BB
		pSecondSpecialData[0] = 0xBB;
		ulSecondSpecialDataSize = 1;
#endif
	}

	// 根据特征码获取地址
	pPspCreateProcessNotifyRoutineAddress = SearchPspCreateProcessNotifyRoutine(pFirstSpecialData, ulFirstSpecialDataSize, pSecondSpecialData, ulSecondSpecialDataSize);
	return pPspCreateProcessNotifyRoutineAddress;
}


// 根据特征码获取 PspCreateProcessNotifyRoutine 数组地址
PVOID SearchPspCreateProcessNotifyRoutine(PUCHAR pFirstSpecialData, ULONG ulFirstSpecialDataSize, PUCHAR pSecondSpecialData, ULONG ulSecondSpecialDataSize)
{
	UNICODE_STRING ustrFuncName;
	PVOID pAddress = NULL;
	LONG lOffset = 0;
	PVOID pPsSetCteateProcessNotifyRoutine = NULL;
	PVOID pPspSetCreateProcessNotifyRoutineAddress = NULL;
	PVOID pPspCreateProcessNotifyRoutineAddress = NULL;

	// 先获取 PsSetCreateProcessNotifyRoutine 函数地址
	RtlInitUnicodeString(&ustrFuncName, L"PsSetCreateProcessNotifyRoutine");
	pPsSetCteateProcessNotifyRoutine = MmGetSystemRoutineAddress(&ustrFuncName);
	if (NULL == pPsSetCteateProcessNotifyRoutine)
	{
		ShowError("MmGetSystemRoutineAddress", 0);
		return pPspCreateProcessNotifyRoutineAddress;
	}

	// 然后, 查找 PspSetCreateProcessNotifyRoutine 函数地址
	pAddress = SearchMemory(pPsSetCteateProcessNotifyRoutine,
		(PVOID)((PUCHAR)pPsSetCteateProcessNotifyRoutine + 0xFF),
		pFirstSpecialData, ulFirstSpecialDataSize);
	if (NULL == pAddress)
	{
		ShowError("SearchMemory1", 0);
		return pPspCreateProcessNotifyRoutineAddress;
	}
	// 获取偏移数据, 并计算地址
	lOffset = *(PLONG)pAddress;
	pPspSetCreateProcessNotifyRoutineAddress = (PVOID)((PUCHAR)pAddress + sizeof(LONG) + lOffset);

	// 最后, 查找 PspCreateProcessNotifyRoutine 地址
	pAddress = SearchMemory(pPspSetCreateProcessNotifyRoutineAddress,
		(PVOID)((PUCHAR)pPspSetCreateProcessNotifyRoutineAddress + 0xFF),
		pSecondSpecialData, ulSecondSpecialDataSize);
	if (NULL == pAddress)
	{
		ShowError("SearchMemory2", 0);
		return pPspCreateProcessNotifyRoutineAddress;
	}
	// 获取地址
#ifdef _WIN64
	// 64 位先获取偏移, 再计算地址
	lOffset = *(PLONG)pAddress;
	pPspCreateProcessNotifyRoutineAddress = (PVOID)((PUCHAR)pAddress + sizeof(LONG) + lOffset);
#else
	// 32 位直接获取地址
	pPspCreateProcessNotifyRoutineAddress = *(PVOID *)pAddress;
#endif

	return pPspCreateProcessNotifyRoutineAddress;
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