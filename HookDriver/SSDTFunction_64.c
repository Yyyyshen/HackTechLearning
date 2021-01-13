#include "SSDTFunction_64.h"
#include "SSDTFunctionIndex.h"

// 获取 SSDT 函数地址
PVOID GetSSDTFunction_64(PCHAR pszFunctionName)
{
	UNICODE_STRING ustrDllFileName;
	ULONG ulSSDTFunctionIndex = 0;
	PVOID pFunctionAddress = NULL;
	PSSDTEntry_64 pServiceDescriptorTable = NULL;
	ULONG ulOffset = 0;

	RtlInitUnicodeString(&ustrDllFileName, L"\\??\\C:\\Windows\\System32\\ntdll.dll");
	// 从 ntdll.dll 中获取 SSDT 函数索引号
	ulSSDTFunctionIndex = GetSSDTFunctionIndex(ustrDllFileName, pszFunctionName);

	// 根据特征码, 从 KiSystemCall64 中获取 SSDT 地址
	pServiceDescriptorTable = GetSSDTAddress();

	// 根据索引号, 从SSDT表中获取对应函数偏移地址并计算出函数地址
	ulOffset = pServiceDescriptorTable->ServiceTableBase[ulSSDTFunctionIndex] >> 4;
	pFunctionAddress = (PVOID)((PUCHAR)pServiceDescriptorTable->ServiceTableBase + ulOffset);

	// 显示
	DbgPrint("[%s][SSDT Addr:0x%p][Index:%d][Address:0x%p]\n", pszFunctionName, pServiceDescriptorTable, ulSSDTFunctionIndex, pFunctionAddress);

	return pFunctionAddress;
}


// 根据特征码, 从 KiSystemCall64 中获取 SSDT 地址
PVOID GetSSDTAddress()
{
	PVOID pServiceDescriptorTable = NULL;
	PVOID pKiSystemCall64 = NULL;
	UCHAR ulCode1 = 0;
	UCHAR ulCode2 = 0;
	UCHAR ulCode3 = 0;
	// 注意使用有符号整型
	LONG lOffset = 0;

	// 获取 KiSystemCall64 函数地址
	pKiSystemCall64 = (PVOID)__readmsr(0xC0000082);
	// 搜索特征码 4C8D15
	for (ULONG i = 0; i < 1024; i++)
	{
		// 获取内存数据
		ulCode1 = *((PUCHAR)((PUCHAR)pKiSystemCall64 + i));
		ulCode2 = *((PUCHAR)((PUCHAR)pKiSystemCall64 + i + 1));
		ulCode3 = *((PUCHAR)((PUCHAR)pKiSystemCall64 + i + 2));
		// 判断
		if (0x4C == ulCode1 &&
			0x8D == ulCode2 &&
			0x15 == ulCode3)
		{
			// 获取偏移
			lOffset = *((PLONG)((PUCHAR)pKiSystemCall64 + i + 3));
			// 根据偏移计算地址
			pServiceDescriptorTable = (PVOID)(((PUCHAR)pKiSystemCall64 + i) + 7 + lOffset);
			break;
		}
	}

	return pServiceDescriptorTable;
}

