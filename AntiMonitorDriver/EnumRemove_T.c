#include "EnumRemove_T.h"


VOID ShowError_T(PCHAR lpszText, NTSTATUS ntStatus)
{
	DbgPrint("%s Error[0x%X]\n", lpszText, ntStatus);
}


// 遍历回调
BOOLEAN EnumNotifyRoutine_T()
{
	ULONG i = 0;
	PVOID pPspCreateThreadNotifyRoutineAddress = NULL;
	PVOID pNotifyRoutineAddress = NULL;

	// 获取 PspCreateThreadNotifyRoutine 数组地址
	pPspCreateThreadNotifyRoutineAddress = GetPspCreateThreadNotifyRoutine();
	if (NULL == pPspCreateThreadNotifyRoutineAddress)
	{
		DbgPrint("GetPspCreateThreadNotifyRoutine Error!\n");
		return FALSE;
	}
	DbgPrint("pPspCreateThreadNotifyRoutineAddress=0x%p\n", pPspCreateThreadNotifyRoutineAddress);

	// 获取回调地址并解密
#ifdef _WIN64
	for (i = 0; i < 64; i++)
	{
		pNotifyRoutineAddress = *(PVOID *)((PUCHAR)pPspCreateThreadNotifyRoutineAddress + sizeof(PVOID) * i);
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
		pNotifyRoutineAddress = *(PVOID *)((PUCHAR)pPspCreateThreadNotifyRoutineAddress + sizeof(PVOID) * i);
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
NTSTATUS RemoveNotifyRoutine_T(PVOID pNotifyRoutineAddress)
{
	NTSTATUS status = PsRemoveCreateThreadNotifyRoutine((PCREATE_THREAD_NOTIFY_ROUTINE)pNotifyRoutineAddress);
	if (!NT_SUCCESS(status))
	{
		ShowError_T("PsRemoveCreateThreadNotifyRoutine", status);
	}
	return status;
}


// 获取 PspCreateThreadNotifyRoutine 数组地址
PVOID GetPspCreateThreadNotifyRoutine()
{
	PVOID pPspCreateThreadNotifyRoutineAddress = NULL;
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
			// 488D0D
			pFirstSpecialData[0] = 0x48;
			pFirstSpecialData[1] = 0x8D;
			pFirstSpecialData[2] = 0x0D;
			ulFirstSpecialDataSize = 3;
#else
			// 32 位
			// BE
			pFirstSpecialData[0] = 0xBE;
			ulFirstSpecialDataSize = 1;
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
			// 488D0D
			pFirstSpecialData[0] = 0x48;
			pFirstSpecialData[1] = 0x8D;
			pFirstSpecialData[2] = 0x0D;
			ulFirstSpecialDataSize = 3;
#else
			// 32 位
			// BB
			pFirstSpecialData[0] = 0xBB;
			ulFirstSpecialDataSize = 1;
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
		// 488D0D
		pSecondSpecialData[0] = 0x48;
		pSecondSpecialData[1] = 0x8D;
		pSecondSpecialData[2] = 0x0D;
		ulSecondSpecialDataSize = 3;
#else
		// 32 位
		// E8
		pFirstSpecialData[0] = 0xE8;
		ulFirstSpecialDataSize = 1;
		// BF
		pSecondSpecialData[0] = 0xBF;
		ulSecondSpecialDataSize = 1;
#endif
	}

	// 根据特征码获取地址
	pPspCreateThreadNotifyRoutineAddress = SearchPspCreateThreadNotifyRoutine(pFirstSpecialData, ulFirstSpecialDataSize, pSecondSpecialData, ulSecondSpecialDataSize);
	return pPspCreateThreadNotifyRoutineAddress;
}


// 根据特征码获取 PspCreateThreadNotifyRoutine 数组地址
PVOID SearchPspCreateThreadNotifyRoutine(PUCHAR pFirstSpecialData, ULONG ulFirstSpecialDataSize, PUCHAR pSecondSpecialData, ULONG ulSecondSpecialDataSize)
{
	UNICODE_STRING ustrFuncName;
	PVOID pAddress = NULL;
	LONG lOffset = 0;
	PVOID pPsSetCreateThreadNotifyRoutine = NULL;
	PVOID pPspSetCreateThreadNotifyRoutineAddress = NULL;
	PVOID pPspCreateThreadNotifyRoutineAddress = NULL;

	// 先获取 PsSetCreateThreadNotifyRoutine 函数地址
	RtlInitUnicodeString(&ustrFuncName, L"PsSetCreateThreadNotifyRoutine");
	pPsSetCreateThreadNotifyRoutine = MmGetSystemRoutineAddress(&ustrFuncName);
	if (NULL == pPsSetCreateThreadNotifyRoutine)
	{
		ShowError_T("MmGetSystemRoutineAddress", 0);
		return pPspCreateThreadNotifyRoutineAddress;
	}

	// 然后, 对于非 Win10 系统, 则根据第一个特征码获取 PspCreateThreadNotifyRoutine 地址;
	// 对于 Win10 系统, 则需先根据第一个特征码获取获取 PspSetCreateThreadNotifyRoutine 地址, 
	// 再根据第二个特征码获取 PspCreateThreadNotifyRoutine 地址.
	pAddress = SearchMemory_T(pPsSetCreateThreadNotifyRoutine,
		(PVOID)((PUCHAR)pPsSetCreateThreadNotifyRoutine + 0xFF),
		pFirstSpecialData, ulFirstSpecialDataSize);
	if (NULL == pAddress)
	{
		ShowError_T("SearchMemory1", 0);
		return pPspCreateThreadNotifyRoutineAddress;
	}

	// 无第二个特征码, 则非 Win10 系统
	if (0 == ulSecondSpecialDataSize)
	{
		// 获取 PspCreateThreadNotifyRoutine 地址
#ifdef _WIN64
		// 64 位
		// 获取偏移数据, 并计算地址
		lOffset = *(PLONG)pAddress;
		pPspCreateThreadNotifyRoutineAddress = (PVOID)((PUCHAR)pAddress + sizeof(LONG) + lOffset);
#else
		// 32 位
		pPspCreateThreadNotifyRoutineAddress = *(PVOID *)pAddress;
#endif 
		
		// 直接返回
		return pPspCreateThreadNotifyRoutineAddress;
	}

	// 存在第二个特征码, 即 Win10 系统
	// 获取偏移数据, 并计算地址
	lOffset = *(PLONG)pAddress;
	pPspSetCreateThreadNotifyRoutineAddress = (PVOID)((PUCHAR)pAddress + sizeof(LONG) + lOffset);
	// 最后, 查找 PspCreateThreadNotifyRoutine 地址
	pAddress = SearchMemory_T(pPspSetCreateThreadNotifyRoutineAddress,
		(PVOID)((PUCHAR)pPspSetCreateThreadNotifyRoutineAddress + 0xFF),
		pSecondSpecialData, ulSecondSpecialDataSize);
	if (NULL == pAddress)
	{
		ShowError_T("SearchMemory2", 0);
		return pPspCreateThreadNotifyRoutineAddress;
	}
	// 获取 PspCreateThreadNotifyRoutine 地址
#ifdef _WIN64
	// 64 位先获取偏移, 再计算地址
	lOffset = *(PLONG)pAddress;
	pPspCreateThreadNotifyRoutineAddress = (PVOID)((PUCHAR)pAddress + sizeof(LONG) + lOffset);
#else
	// 32 位直接获取地址
	pPspCreateThreadNotifyRoutineAddress = *(PVOID *)pAddress;
#endif

	return pPspCreateThreadNotifyRoutineAddress;
}


// 指定内存区域的特征码扫描
PVOID SearchMemory_T(PVOID pStartAddress, PVOID pEndAddress, PUCHAR pMemoryData, ULONG ulMemoryDataSize)
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