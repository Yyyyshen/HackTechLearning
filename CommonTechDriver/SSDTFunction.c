#include "SSDTFunction.h"


// 获取 SSDT 函数地址
PVOID GetSSDTFunction(PCHAR pszFunctionName)
{
	UNICODE_STRING ustrDllFileName;
	ULONG ulSSDTFunctionIndex = 0;
	PVOID pFunctionAddress = NULL;
	PSSDTEntry pServiceDescriptorTable = NULL;
	ULONG ulOffset = 0;
	
	RtlInitUnicodeString(&ustrDllFileName, L"\\??\\C:\\Windows\\System32\\ntdll.dll");
	// 从 ntdll.dll 中获取 SSDT 函数索引号
	ulSSDTFunctionIndex = GetSSDTFunctionIndex(ustrDllFileName, pszFunctionName);

	//32位, 直接获取导出地址; 64位, 根据特征码, 从 KiSystemCall64 中获取 SSDT 地址
	pServiceDescriptorTable = GetSSDTAddress();

	// 根据索引号, 从SSDT表中获取对应函数偏移地址并计算出函数地址
#ifndef _WIN64
	// 32 Bits
	pFunctionAddress = (PVOID)pServiceDescriptorTable->ServiceTableBase[ulSSDTFunctionIndex];
#else
	// 64 Bits
	ulOffset = pServiceDescriptorTable->ServiceTableBase[ulSSDTFunctionIndex] >> 4;
	pFunctionAddress = (PVOID)((PUCHAR)pServiceDescriptorTable->ServiceTableBase + ulOffset);
#endif

	// 显示
	DbgPrint("[%s][SSDT Addr:0x%p][Index:%d][Address:0x%p]\n", pszFunctionName, pServiceDescriptorTable, ulSSDTFunctionIndex, pFunctionAddress);

	return pFunctionAddress;
}


// 32位, 直接获取导出地址; 64位, 根据特征码, 从 KiSystemCall64 中获取 SSDT 地址
PVOID GetSSDTAddress()
{
	PVOID pServiceDescriptorTable = NULL;
	PVOID pKiSystemCall64 = NULL;
	UCHAR ulCode1 = 0;
	UCHAR ulCode2 = 0;
	UCHAR ulCode3 = 0;
	// 注意使用有符号整型
	LONG lOffset = 0;

#ifndef _WIN64
	// 32 Bits
	pServiceDescriptorTable = (PVOID)(&KeServiceDescriptorTable);
#else
	// 64 Bits
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
#endif

	return pServiceDescriptorTable;
}


// 从 ntdll.dll 中获取 SSDT 函数索引号
ULONG GetSSDTFunctionIndex(UNICODE_STRING ustrDllFileName, PCHAR pszFunctionName)
{
	ULONG ulFunctionIndex = 0;
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE hFile = NULL;
	HANDLE hSection = NULL;
	PVOID pBaseAddress = NULL;

	// 内存映射文件
	status = DllFileMap(ustrDllFileName, &hFile, &hSection, &pBaseAddress);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("DllFileMap Error!\n"));
		return ulFunctionIndex;
	}

	// 根据导出表获取导出函数地址, 从而获取 SSDT 函数索引号
	ulFunctionIndex = GetIndexFromExportTable(pBaseAddress, pszFunctionName);

	// 释放
	ZwUnmapViewOfSection(NtCurrentProcess(), pBaseAddress);
	ZwClose(hSection);
	ZwClose(hFile);

	return ulFunctionIndex;
}

// 内存映射文件
NTSTATUS DllFileMap(UNICODE_STRING ustrDllFileName, HANDLE *phFile, HANDLE *phSection, PVOID *ppBaseAddress)
{
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE hFile = NULL;
	HANDLE hSection = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	PVOID pBaseAddress = NULL;
	SIZE_T viewSize = 0;
	// 打开 DLL 文件, 并获取文件句柄
	InitializeObjectAttributes(&objectAttributes, &ustrDllFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwOpenFile(&hFile, GENERIC_READ, &objectAttributes, &iosb,
		FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("ZwOpenFile Error! [error code: 0x%X]", status));
		return status;
	}
	// 创建一个节对象, 以 PE 结构中的 SectionALignment 大小对齐映射文件
	status = ZwCreateSection(&hSection, SECTION_MAP_READ | SECTION_MAP_WRITE, NULL, 0, PAGE_READWRITE, 0x1000000, hFile);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		KdPrint(("ZwCreateSection Error! [error code: 0x%X]", status));
		return status;
	}
	// 映射到内存
	status = ZwMapViewOfSection(hSection, NtCurrentProcess(), &pBaseAddress, 0, 1024, 0, &viewSize, ViewShare, MEM_TOP_DOWN, PAGE_READWRITE);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hSection);
		ZwClose(hFile);
		KdPrint(("ZwMapViewOfSection Error! [error code: 0x%X]", status));
		return status;
	}

	// 返回数据
	*phFile = hFile;
	*phSection = hSection;
	*ppBaseAddress = pBaseAddress;

	return status;
}

// 根据导出表获取导出函数地址, 从而获取 SSDT 函数索引号
ULONG GetIndexFromExportTable(PVOID pBaseAddress, PCHAR pszFunctionName)
{
	ULONG ulFunctionIndex = 0;
	// Dos Header
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBaseAddress;
	// NT Header
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
	// Export Table
	PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);
	// 有名称的导出函数个数
	ULONG ulNumberOfNames = pExportTable->NumberOfNames;
	// 导出函数名称地址表
	PULONG lpNameArray = (PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfNames);
	PCHAR lpName = NULL;
	// 开始遍历导出表
	for (ULONG i = 0; i < ulNumberOfNames; i++)
	{
		lpName = (PCHAR)((PUCHAR)pDosHeader + lpNameArray[i]);
		// 判断是否查找的函数
		if (0 == _strnicmp(pszFunctionName, lpName, strlen(pszFunctionName)))
		{
			// 获取导出函数地址
			USHORT uHint = *(USHORT *)((PUCHAR)pDosHeader + pExportTable->AddressOfNameOrdinals + 2 * i);
			ULONG ulFuncAddr = *(PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfFunctions + 4 * uHint);
			PVOID lpFuncAddr = (PVOID)((PUCHAR)pDosHeader + ulFuncAddr);
			// 获取 SSDT 函数 Index
#ifdef _WIN64
			ulFunctionIndex = *(ULONG *)((PUCHAR)lpFuncAddr + 4);
#else
			ulFunctionIndex = *(ULONG *)((PUCHAR)lpFuncAddr + 1);
#endif
			break;
		}
	}

	return ulFunctionIndex;
}