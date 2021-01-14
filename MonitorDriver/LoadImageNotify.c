#include "LoadImageNotify.h"


NTSTATUS SetNotifyRoutine()
{
	NTSTATUS status = STATUS_SUCCESS;
	status = PsSetLoadImageNotifyRoutine(LoadImageNotifyRoutine);
	return status;
}


NTSTATUS RemoveNotifyRoutine()
{
	NTSTATUS status = STATUS_SUCCESS;
	status = PsRemoveLoadImageNotifyRoutine(LoadImageNotifyRoutine);
	return status;
}


VOID LoadImageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	DbgPrint("[%d][%wZ][%d][0x%p]\n", ProcessId, FullImageName, ImageInfo->ImageSize, ImageInfo->ImageBase);

	HANDLE hThread = NULL;
	CHAR szTemp[1024] = { 0 };
	U2C(FullImageName, szTemp, 1024);
	if (NULL != strstr(szTemp, "wininet.dll") || NULL != strstr(szTemp, "MySYS.sys"))
	{
		// EXE或者DLL
		if (0 != ProcessId)
		{
			// 创建多线程, 延时1秒钟后再卸载模块
			PMY_DATA pMyData = ExAllocatePool(NonPagedPool, sizeof(MY_DATA));
			pMyData->ProcessId = ProcessId;
			pMyData->pImageBase = ImageInfo->ImageBase;
			PsCreateSystemThread(&hThread, 0, NULL, NtCurrentProcess(), NULL, ThreadProc, pMyData);
			DbgPrint("Deny Load DLL\n");
		}
		// 驱动
		else
		{
			DenyLoadDriver(ImageInfo->ImageBase);
			DbgPrint("Deny Load Driver\n");
		}
	}
}


// 拒绝加载驱动
NTSTATUS DenyLoadDriver(PVOID pImageBase)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMDL pMdl = NULL;
	PVOID pVoid = NULL;
	ULONG ulShellcodeLength = 16;
	//将入口函数前几个字节数据修改为 mov eax,0xC0000022 （STATUS_ACCESS_DENIED）
	//								 ret
	//对应机器码为B8 22 00 00 C0 C3
	UCHAR pShellcode[16] = {0xB8, 0x22, 0x00, 0x00, 0xC0, 0xC3, 0x90, 0x90,
		                    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
	PIMAGE_DOS_HEADER pDosHeader = pImageBase;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
	PVOID pDriverEntry = (PVOID)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.AddressOfEntryPoint);
	
	pMdl = MmCreateMdl(NULL, pDriverEntry, ulShellcodeLength);
	MmBuildMdlForNonPagedPool(pMdl);
	pVoid = MmMapLockedPages(pMdl, KernelMode);
	RtlCopyMemory(pVoid, pShellcode, ulShellcodeLength);
	MmUnmapLockedPages(pVoid, pMdl);
	IoFreeMdl(pMdl);

	return status;
}


// 调用 MmUnmapViewOfSection 函数来卸载已经加载的 DLL 模块
NTSTATUS DenyLoadDll(HANDLE ProcessId, PVOID pImageBase)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pEProcess = NULL;

	status = PsLookupProcessByProcessId(ProcessId, &pEProcess);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsLookupProcessByProcessId Error[0x%X]\n", status);
		return status;
	}

	// 卸载模块
	status = MmUnmapViewOfSection(pEProcess, pImageBase);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("MmUnmapViewOfSection Error[0x%X]\n", status);
		return status;
	}

	return status;
}


VOID ThreadProc(_In_ PVOID StartContext)
{
	PMY_DATA pMyData = (PMY_DATA)StartContext;
	LARGE_INTEGER liTime = { 0 };
	// 延时 1 秒
	liTime.QuadPart = -10 * 1000 * 1000;      // 100纳秒为单位时间, 1秒==1000毫秒==1000*1000微秒==1000*1000*1000纳秒, 负值表示相对时间
	KeDelayExecutionThread(KernelMode, FALSE, &liTime);
	// 卸载
	DenyLoadDll(pMyData->ProcessId, pMyData->pImageBase);

	ExFreePool(pMyData);
}


NTSTATUS U2C(PUNICODE_STRING pustrSrc, PCHAR pszDest, ULONG ulDestLength)
{
	NTSTATUS status = STATUS_SUCCESS;
	ANSI_STRING strTemp;

	RtlZeroMemory(pszDest, ulDestLength);
	RtlUnicodeStringToAnsiString(&strTemp, pustrSrc, TRUE);
	if (ulDestLength > strTemp.Length)
	{
		RtlCopyMemory(pszDest, strTemp.Buffer, strTemp.Length);
	}
	RtlFreeAnsiString(&strTemp);

	return status;
}