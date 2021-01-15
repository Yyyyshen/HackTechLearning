#include "GetMiProcessLoaderEntry.h"
#include "SSDTFunction.h"


// 从 MmLoadSystemImage 中获取对应的 MiProcessLoaderEntry 特征码
// 其中, 32和64位的 Win7, Win8.1 直接从 MmLoadSystemImage 中搜索 MiProcessLoaderEntry
// 32和64位的 Win10 需要从 MmLoadSystemImage 中搜索 MiConstructLoaderEntry, 再从 MiConstructLoaderEntry 中搜索 MiProcessLoaderEntry
PVOID GetFuncAddr_MiProcessLoaderEntry()
{
	NTSTATUS status = STATUS_SUCCESS;
	RTL_OSVERSIONINFOW osVersionInfo = { 0 };
	PVOID pMmLoadSystemImage = NULL;
	PVOID pMiConstructLoaderEntry = NULL;
	PVOID pMiProcessLoaderEntry = NULL;
	UCHAR pSpecialCode[256] = { 0 };
	ULONG ulSpecialCodeLength = 256;
	ULONG ulSearchLength = 0x1000;
	PVOID pSearchResultAddr = NULL;
	LONG lOffset = 0;
	RtlZeroMemory(pSpecialCode, ulSpecialCodeLength);

	// 从 NtSetSystemInformation 中获取 MmLoadSystemImage 函数地址
	pMmLoadSystemImage = GetFuncAddr_MmLoadSystemImage();
	if (NULL == pMmLoadSystemImage)
	{
		return pMiProcessLoaderEntry;
	}
	DbgPrint("pMmLoadSystemImage[0x%p]\n", pMmLoadSystemImage);

	// 获取系统版本信息
	RtlGetVersion(&osVersionInfo);
	if (6 == osVersionInfo.dwMajorVersion)
	{
		// Win7
		if (1 == osVersionInfo.dwMinorVersion)
		{
#ifndef _WIN64
			// 32 Bits
			pSpecialCode[0] = 0x6a;
			pSpecialCode[1] = 0x01;
			pSpecialCode[2] = 0x56;
			pSpecialCode[3] = 0xe8;
			ulSpecialCodeLength = 4;
#else
			// 64 Bits
			pSpecialCode[0] = 0xba;
			pSpecialCode[1] = 0x01;
			pSpecialCode[2] = 0x00;
			pSpecialCode[3] = 0x00;
			pSpecialCode[4] = 0x00;
			pSpecialCode[5] = 0x48;
			pSpecialCode[6] = 0x8b;
			pSpecialCode[7] = 0xcd;
			pSpecialCode[8] = 0xe8;
			ulSpecialCodeLength = 9;
#endif
		}
		// Win8
		else if (2 == osVersionInfo.dwMinorVersion)
		{
			
		}
		// Win8.1
		else if (3 == osVersionInfo.dwMinorVersion)
		{
#ifndef _WIN64
			// 32 Bits
			pSpecialCode[0] = 0x89;
			pSpecialCode[1] = 0x74;
			pSpecialCode[2] = 0x24;
			pSpecialCode[3] = 0x1c;
			pSpecialCode[4] = 0x8b;
			pSpecialCode[5] = 0xcf;
			pSpecialCode[6] = 0xe8;
			ulSpecialCodeLength = 7;
#else
			// 64 Bits
			pSpecialCode[0] = 0x41;
			pSpecialCode[1] = 0x83;
			pSpecialCode[2] = 0xcc;
			pSpecialCode[3] = 0x04;
			pSpecialCode[4] = 0xe8;
			ulSpecialCodeLength = 5;
#endif
		}
	}
	// Win10 
	else if (10 == osVersionInfo.dwMajorVersion)
	{
		// 先获取 MiConstructLoaderEntry, 再获取 MiProcessLoaderEntry
#ifndef _WIN64
		// 32 Bits
		pSpecialCode[0] = 0x8d;
		pSpecialCode[1] = 0x54;
		pSpecialCode[2] = 0x24;
		pSpecialCode[3] = 0x4c;
		pSpecialCode[4] = 0x50;
		pSpecialCode[5] = 0xe8;
		ulSpecialCodeLength = 6;
		// 搜索特征码
		pSearchResultAddr = SearchSpecialCode(pMmLoadSystemImage, ulSearchLength, pSpecialCode, ulSpecialCodeLength);
		if (NULL == pSearchResultAddr)
		{
			return pMiProcessLoaderEntry;
		}
		// 获取偏移值
		lOffset = *(LONG *)((PUCHAR)pSearchResultAddr + ulSpecialCodeLength);
		// 计算地址(跳转地址 = 下一条指令地址 + 跳转偏移)
		pMiConstructLoaderEntry = (PVOID)(((PUCHAR)pSearchResultAddr + ulSpecialCodeLength + sizeof(LONG)) + lOffset);
        // 继续搜索
		pSpecialCode[0] = 0x8b;
		pSpecialCode[1] = 0xcb;
		pSpecialCode[2] = 0x42;
		pSpecialCode[3] = 0xe8;
		ulSpecialCodeLength = 4;
		pMmLoadSystemImage = pMiConstructLoaderEntry;
#else
		// 64 Bits
		pSpecialCode[0] = 0x48;
		pSpecialCode[1] = 0x8b;
		pSpecialCode[2] = 0xcf;
		pSpecialCode[3] = 0x89;
		pSpecialCode[4] = 0x44;
		pSpecialCode[5] = 0x24;
		pSpecialCode[6] = 0x20;
		pSpecialCode[7] = 0xe8;
		ulSpecialCodeLength = 8;
		// 搜索特征码
		pSearchResultAddr = SearchSpecialCode(pMmLoadSystemImage, ulSearchLength, pSpecialCode, ulSpecialCodeLength);
		if (NULL == pSearchResultAddr)
		{
			return pMiProcessLoaderEntry;
		}
		// 获取偏移值
		lOffset = *(LONG *)((PUCHAR)pSearchResultAddr + ulSpecialCodeLength);
		// 计算地址(跳转地址 = 下一条指令地址 + 跳转偏移)
		pMiConstructLoaderEntry = (PVOID)(((PUCHAR)pSearchResultAddr + ulSpecialCodeLength + sizeof(LONG)) + lOffset);
		// 继续搜索
		pSpecialCode[0] = 0xba;
		pSpecialCode[1] = 0x01;
		pSpecialCode[2] = 0x00;
		pSpecialCode[3] = 0x00;
		pSpecialCode[4] = 0x00;
		pSpecialCode[5] = 0x48;
		pSpecialCode[6] = 0x8b;
		pSpecialCode[7] = 0xcf;
		pSpecialCode[8] = 0xe8;
		ulSpecialCodeLength = 9;
		pMmLoadSystemImage = pMiConstructLoaderEntry;
#endif
	}

	// 搜索特征码
	pSearchResultAddr = SearchSpecialCode(pMmLoadSystemImage, ulSearchLength, pSpecialCode, ulSpecialCodeLength);
	if (NULL == pSearchResultAddr)
	{
		return pMiProcessLoaderEntry;
	}
	// 获取偏移值
	lOffset = *(LONG *)((PUCHAR)pSearchResultAddr + ulSpecialCodeLength);
	// 计算地址(跳转地址 = 下一条指令地址 + 跳转偏移)
	pMiProcessLoaderEntry = (PVOID)(((PUCHAR)pSearchResultAddr + ulSpecialCodeLength + sizeof(LONG)) + lOffset);

	return pMiProcessLoaderEntry;
}


// 从 NtSetSystemInformation 中获取 MmLoadSystemImage 函数地址
PVOID GetFuncAddr_MmLoadSystemImage()
{
	NTSTATUS status = STATUS_SUCCESS;
	RTL_OSVERSIONINFOW osVersionInfo = { 0 };
	PVOID pNtSetSystemInformation = NULL;
	PVOID pMmLoadSystemImage = NULL;
	UCHAR pSpecialCode[256] = { 0 };
	ULONG ulSpecialCodeLength = 256;
	ULONG ulSearchLength = 0x1000;
	PVOID pSearchResultAddr = NULL;
	LONG lOffset = 0;
	RtlZeroMemory(pSpecialCode, ulSpecialCodeLength);

	// 从 SSDT 中获取 NtSetSystemInformation 函数地址
	pNtSetSystemInformation = GetSSDTFunction("NtSetSystemInformation");
	if (NULL == pNtSetSystemInformation)
	{
		return pMmLoadSystemImage;
	}

	// 获取系统版本信息
	RtlGetVersion(&osVersionInfo);
	if (6 == osVersionInfo.dwMajorVersion)
	{
		// Win7
		if (1 == osVersionInfo.dwMinorVersion)
		{
#ifndef _WIN64
			// 32 Bits
			pSpecialCode[0] = 0xd8;
			pSpecialCode[1] = 0x50;
			pSpecialCode[2] = 0xe8;
			ulSpecialCodeLength = 3;
#else
			// 64 Bits
			pSpecialCode[0] = 0x48;
			pSpecialCode[1] = 0x8d;
			pSpecialCode[2] = 0x4c;
			pSpecialCode[3] = 0x24;
			pSpecialCode[4] = 0x38;
			pSpecialCode[5] = 0xe8;
			ulSpecialCodeLength = 6;
#endif
		}
		// Win8
		else if (2 == osVersionInfo.dwMinorVersion)
		{
			
		}
		// Win8.1
		else if (3 == osVersionInfo.dwMinorVersion)
		{
#ifndef _WIN64
			// 32 Bits
			pSpecialCode[0] = 0x8d;
			pSpecialCode[1] = 0x85;
			pSpecialCode[2] = 0x10;
			pSpecialCode[3] = 0xff;
			pSpecialCode[4] = 0xff;
			pSpecialCode[5] = 0xff;
			pSpecialCode[6] = 0x50;
			pSpecialCode[7] = 0xe8;
			ulSpecialCodeLength = 8;
#else
			// 64 Bits
			pSpecialCode[0] = 0x48;
			pSpecialCode[1] = 0x8d;
			pSpecialCode[2] = 0x8c;
			pSpecialCode[3] = 0x24;
			pSpecialCode[4] = 0x00;
			pSpecialCode[5] = 0x02;
			pSpecialCode[6] = 0x00;
			pSpecialCode[7] = 0x00;
			pSpecialCode[8] = 0xe8;
			ulSpecialCodeLength = 9;
#endif
		}
	}
	// Win10 
	else if (10 == osVersionInfo.dwMajorVersion)
	{
#ifndef _WIN64
		// 32 Bits
		pSpecialCode[0] = 0x8d;
		pSpecialCode[1] = 0x85;
		pSpecialCode[2] = 0x04;
		pSpecialCode[3] = 0xff;
		pSpecialCode[4] = 0xff;
		pSpecialCode[5] = 0xff;
		pSpecialCode[6] = 0x50;
		pSpecialCode[7] = 0xe8;
		ulSpecialCodeLength = 8;
#else
		// 64 Bits
		pSpecialCode[0] = 0x48;
		pSpecialCode[1] = 0x8d;
		pSpecialCode[2] = 0x8c;
		pSpecialCode[3] = 0x24;
		pSpecialCode[4] = 0x48;
		pSpecialCode[5] = 0x02;
		pSpecialCode[6] = 0x00;
		pSpecialCode[7] = 0x00;
		pSpecialCode[8] = 0xe8;
		ulSpecialCodeLength = 9;
#endif
	}

	// 搜索特征码
	pSearchResultAddr = SearchSpecialCode(pNtSetSystemInformation, ulSearchLength, pSpecialCode, ulSpecialCodeLength);
	if (NULL == pSearchResultAddr)
	{
		return pMmLoadSystemImage;
	}
	// 获取偏移值
	lOffset = *(LONG *)((PUCHAR)pSearchResultAddr + ulSpecialCodeLength);
	// 计算地址(跳转地址 = 下一条指令地址 + 跳转偏移)
	pMmLoadSystemImage = (PVOID)(((PUCHAR)pSearchResultAddr + ulSpecialCodeLength + sizeof(LONG)) + lOffset);

	return pMmLoadSystemImage;
}


// 搜索特征码
PVOID SearchSpecialCode(PVOID pSearchBeginAddr, ULONG ulSearchLength, PUCHAR pSpecialCode, ULONG ulSpecialCodeLength)
{
	PVOID pDestAddr = NULL;
	PUCHAR pBeginAddr = (PUCHAR)pSearchBeginAddr;
	PUCHAR pEndAddr = pBeginAddr + ulSearchLength;
	PUCHAR i = NULL;
	ULONG j = 0;

	for (i = pBeginAddr; i <= pEndAddr; i++)
	{
		// 遍历特征码
		for (j = 0; j < ulSpecialCodeLength; j++)
		{
			// 判断地址是否有效
			if (FALSE == MmIsAddressValid((PVOID)(i + j)))
			{
				break;;
			}
			// 匹配特征码
			if (*(PUCHAR)(i + j) != pSpecialCode[j])
			{
				break;
			}
		}
		// 匹配成功
		if (j >= ulSpecialCodeLength)
		{
			pDestAddr = (PVOID)i;
			break;
		}
	}

	return pDestAddr;
}