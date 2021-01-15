#include "EnumRemove_M.h"


VOID ShowError_M(PCHAR lpszText, NTSTATUS ntStatus)
{
	DbgPrint("%s Error[0x%X]\n", lpszText, ntStatus);
}


// 遍历回调
BOOLEAN EnumNotifyRoutine_M()
{
	ULONG i = 0;
	PVOID pPspLoadImageNotifyRoutineAddress = NULL;
	PVOID pNotifyRoutineAddress = NULL;

	// 获取 PspLoadImageNotifyRoutine 数组地址
	pPspLoadImageNotifyRoutineAddress = GetPspLoadImageNotifyRoutine();
	if (NULL == pPspLoadImageNotifyRoutineAddress)
	{
		DbgPrint("GetPspLoadImageNotifyRoutine Error!\n");
		return FALSE;
	}
	DbgPrint("pPspLoadImageNotifyRoutineAddress=0x%p\n", pPspLoadImageNotifyRoutineAddress);

	// 获取回调地址并解密
#ifdef _WIN64
	for (i = 0; i < 64; i++)
	{
		pNotifyRoutineAddress = *(PVOID *)((PUCHAR)pPspLoadImageNotifyRoutineAddress + sizeof(PVOID) * i);
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
		pNotifyRoutineAddress = *(PVOID *)((PUCHAR)pPspLoadImageNotifyRoutineAddress + sizeof(PVOID) * i);
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
NTSTATUS RemoveNotifyRoutine_M(PVOID pNotifyRoutineAddress)
{
	NTSTATUS status = PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)pNotifyRoutineAddress);
	if (!NT_SUCCESS(status))
	{
		ShowError_M("PsRemoveLoadImageNotifyRoutine", status);
	}
	return status;
}


// 获取 PspLoadImageNotifyRoutine 数组地址
PVOID GetPspLoadImageNotifyRoutine()
{
	PVOID pPspLoadImageNotifyRoutineAddress = NULL;
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
			// 488D0D
			pSpecialData[0] = 0x48;
			pSpecialData[1] = 0x8D;
			pSpecialData[2] = 0x0D;
			ulSpecialDataSize = 3;
#else
			// 32 位
			// BE
			pSpecialData[0] = 0xBE;
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
			// 488D0D
			pSpecialData[0] = 0x48;
			pSpecialData[1] = 0x8D;
			pSpecialData[2] = 0x0D;
			ulSpecialDataSize = 3;
#else
			// 32 位
			// BB
			pSpecialData[0] = 0xBB;
			ulSpecialDataSize = 1;
#endif			
		}
	}
	else if (10 == osInfo.dwMajorVersion)
	{
		// Win10
#ifdef _WIN64
		// 64 位
		// 488D0D
		pSpecialData[0] = 0x48;
		pSpecialData[1] = 0x8D;
		pSpecialData[2] = 0x0D;
		ulSpecialDataSize = 3;
#else
		// 32 位
		// BF
		pSpecialData[0] = 0xBF;
		ulSpecialDataSize = 1;
#endif
	}

	// 根据特征码获取地址
	pPspLoadImageNotifyRoutineAddress = SearchPspLoadImageNotifyRoutine(pSpecialData, ulSpecialDataSize);
	return pPspLoadImageNotifyRoutineAddress;
}


// 根据特征码获取 PspLoadImageNotifyRoutine 数组地址
PVOID SearchPspLoadImageNotifyRoutine(PUCHAR pSpecialData, ULONG ulSpecialDataSize)
{
	UNICODE_STRING ustrFuncName;
	PVOID pAddress = NULL;
	LONG lOffset = 0;
	PVOID pPsSetLoadImageNotifyRoutine = NULL;
	PVOID pPspLoadImageNotifyRoutine = NULL;

	// 先获取 PsSetLoadImageNotifyRoutine 函数地址
	RtlInitUnicodeString(&ustrFuncName, L"PsSetLoadImageNotifyRoutine");
	pPsSetLoadImageNotifyRoutine = MmGetSystemRoutineAddress(&ustrFuncName);
	if (NULL == pPsSetLoadImageNotifyRoutine)
	{
		ShowError_M("MmGetSystemRoutineAddress", 0);
		return pPspLoadImageNotifyRoutine;
	}

	// 然后, 查找 PspSetCreateProcessNotifyRoutine 函数地址
	pAddress = SearchMemory_M(pPsSetLoadImageNotifyRoutine,
		(PVOID)((PUCHAR)pPsSetLoadImageNotifyRoutine + 0xFF),
		pSpecialData, ulSpecialDataSize);
	if (NULL == pAddress)
	{
		ShowError_M("SearchMemory", 0);
		return pPspLoadImageNotifyRoutine;
	}

	// 获取地址
#ifdef _WIN64
	// 64 位先获取偏移, 再计算地址
	lOffset = *(PLONG)pAddress;
	pPspLoadImageNotifyRoutine = (PVOID)((PUCHAR)pAddress + sizeof(LONG) + lOffset);
#else
	// 32 位直接获取地址
	pPspLoadImageNotifyRoutine = *(PVOID *)pAddress;
#endif

	return pPspLoadImageNotifyRoutine;
}


// 指定内存区域的特征码扫描
PVOID SearchMemory_M(PVOID pStartAddress, PVOID pEndAddress, PUCHAR pMemoryData, ULONG ulMemoryDataSize)
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