#include "FileManage.h"


VOID ShowError(CHAR *lpszText, NTSTATUS status)
{
	DbgPrint("%s Error! Error Code: 0x%08X\n", lpszText, status);
}


// 创建文件
BOOLEAN MyCreateFile(UNICODE_STRING ustrFilePath)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	// 创建文件
	InitializeObjectAttributes(&objectAttributes, &ustrFilePath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, GENERIC_READ, &objectAttributes, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwCreateFile", status);
		return FALSE;
	}
	// 关闭句柄
	ZwClose(hFile);

	return TRUE;
}


// 创建目录
BOOLEAN MyCreateFileFolder(UNICODE_STRING ustrFileFolderPath)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	// 创建目录
	InitializeObjectAttributes(&objectAttributes, &ustrFileFolderPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, GENERIC_READ, &objectAttributes, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE, FILE_DIRECTORY_FILE, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwCreateFile", status);
		return FALSE;
	}
	// 关闭句柄
	ZwClose(hFile);

	return TRUE;
}


// 删除文件或是空目录
BOOLEAN MyDeleteFileOrFileFolder(UNICODE_STRING ustrFileName)
{
	NTSTATUS status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };

	InitializeObjectAttributes(&objectAttributes, &ustrFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	// 执行删除操作
	status = ZwDeleteFile(&objectAttributes);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwDeleteFile", status);
		return FALSE;
	}

	return TRUE;
}


// 获取文件大小
ULONG64 MyGetFileSize(UNICODE_STRING ustrFileName)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	FILE_STANDARD_INFORMATION fsi = { 0 };

	// 获取文件句柄
	InitializeObjectAttributes(&objectAttributes, &ustrFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, GENERIC_READ, &objectAttributes, &iosb, NULL, 0,
		FILE_SHARE_READ, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwCreateFile", status);
		return 0;
	}
	// 获取文件大小信息
	status = ZwQueryInformationFile(hFile, &iosb, &fsi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		ShowError("ZwQueryInformationFile", status);
		return 0;
	}

	return fsi.EndOfFile.QuadPart;
}


// 重命名文件或文件夹
BOOLEAN MyRenameFileOrFileFolder(UNICODE_STRING ustrSrcFileName, UNICODE_STRING ustrDestFileName)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	PFILE_RENAME_INFORMATION pRenameInfo = NULL;
	ULONG ulLength = (1024 + sizeof(FILE_RENAME_INFORMATION));

	// 申请内存
	pRenameInfo = (PFILE_RENAME_INFORMATION)ExAllocatePool(NonPagedPool, ulLength);
	if (NULL == pRenameInfo)
	{
		ShowError("ExAllocatePool", 0);
		return FALSE;
	}
	// 设置重命名信息
	RtlZeroMemory(pRenameInfo, ulLength);
	pRenameInfo->FileNameLength = ustrDestFileName.Length;
	wcscpy(pRenameInfo->FileName, ustrDestFileName.Buffer);
	pRenameInfo->ReplaceIfExists = 0;
	pRenameInfo->RootDirectory = NULL;
	// 设置源文件信息并获取句柄
	InitializeObjectAttributes(&objectAttributes, &ustrSrcFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, SYNCHRONIZE | DELETE, &objectAttributes,
		&iosb, NULL, 0, FILE_SHARE_READ, FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NO_INTERMEDIATE_BUFFERING, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(pRenameInfo);
		ShowError("ZwCreateFile", status);
		return FALSE;
	}
	// 利用ZwSetInformationFile来设置文件信息
	status = ZwSetInformationFile(hFile, &iosb, pRenameInfo, ulLength, FileRenameInformation);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		ExFreePool(pRenameInfo);
		ShowError("ZwSetInformationFile", status);
		return FALSE;
	}
	// 释放内存, 关闭句柄
	ExFreePool(pRenameInfo);
	ZwClose(hFile);

	return TRUE;
}


// 遍历文件夹和文件
BOOLEAN MyQueryFileAndFileFolder(UNICODE_STRING ustrPath)
{
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;

	// 获取文件句柄
	InitializeObjectAttributes(&objectAttributes, &ustrPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, FILE_LIST_DIRECTORY | SYNCHRONIZE | FILE_ANY_ACCESS,
		&objectAttributes, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
		NULL, 0);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwCreateFile", status);
		return FALSE;
	}

	// 遍历文件
	// 注意此处的大小!!!一定要申请足够内存，否则后面ExFreePool会蓝屏
	ULONG ulLength = (2 * 4096 + sizeof(FILE_BOTH_DIR_INFORMATION)) * 0x2000;              
	PFILE_BOTH_DIR_INFORMATION pDir = ExAllocatePool(PagedPool, ulLength);
	// 保存pDir的首地址，用来释放内存使用!!!
	PFILE_BOTH_DIR_INFORMATION pBeginAddr = pDir;                                      
	// 获取信息
	status = ZwQueryDirectoryFile(hFile, NULL, NULL, NULL, &iosb, pDir, ulLength,
		FileBothDirectoryInformation, FALSE, NULL, FALSE);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(pDir);
		ZwClose(hFile);
		ShowError("ZwQueryDirectoryFile", status);
		return FALSE;
	}
	// 遍历
	UNICODE_STRING ustrTemp;
	UNICODE_STRING ustrOne;
	UNICODE_STRING ustrTwo;
	RtlInitUnicodeString(&ustrOne, L".");
	RtlInitUnicodeString(&ustrTwo, L"..");
	WCHAR wcFileName[1024] = { 0 };
	while (TRUE)
	{
		// 判断是否是上级目录或是本目录
		RtlZeroMemory(wcFileName, 1024);
		RtlCopyMemory(wcFileName, pDir->FileName, pDir->FileNameLength);
		RtlInitUnicodeString(&ustrTemp, wcFileName);
		if ((0 != RtlCompareUnicodeString(&ustrTemp, &ustrOne, TRUE)) &&
			(0 != RtlCompareUnicodeString(&ustrTemp, &ustrTwo, TRUE)))
		{
			if (pDir->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// 目录
				DbgPrint("[DIRECTORY]\t%wZ\n", &ustrTemp);
			}
			else
			{
				// 文件
				DbgPrint("[FILE]\t\t%wZ\n", &ustrTemp);
			}
		}
		// 遍历完毕
		if (0 == pDir->NextEntryOffset)
		{
			DbgPrint("\n[QUERY OVER]\n\n");
			break;
		}
		// pDir指向的地址改变了，所以下面ExFreePool(pDir)会出错！！！所以，必须保存首地址
		pDir = (PFILE_BOTH_DIR_INFORMATION)((PUCHAR)pDir + pDir->NextEntryOffset);
	}
	// 释放内存, 关闭文件句柄
	ExFreePool(pBeginAddr);
	ZwClose(hFile);

	return TRUE;
}


// 读取文件数据
BOOLEAN MyReadFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pReadData, PULONG pulReadDataSize)
{
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK iosb = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	NTSTATUS status = STATUS_SUCCESS;

	// 打开文件
	InitializeObjectAttributes(&objectAttributes, &ustrFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, GENERIC_READ, &objectAttributes, &iosb, NULL,
		FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwCreateFile", status);
		return FALSE;
	}
	// 读取文件数据
	RtlZeroMemory(&iosb, sizeof(iosb));
	status = ZwReadFile(hFile, NULL, NULL, NULL, &iosb,
		pReadData, *pulReadDataSize, &liOffset, NULL);
	if (!NT_SUCCESS(status))
	{
		*pulReadDataSize = iosb.Information;
		ZwClose(hFile);
		ShowError("ZwCreateFile", status);
		return FALSE;
	}
	// 获取实际读取的数据
	*pulReadDataSize = iosb.Information;
	// 关闭句柄
	ZwClose(hFile);

	return TRUE;
}


// 向文件写入数据
BOOLEAN MyWriteFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pWriteData, PULONG pulWriteDataSize)
{
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK iosb = { 0 };
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	NTSTATUS status = STATUS_SUCCESS;

	// 打开文件
	InitializeObjectAttributes(&objectAttributes, &ustrFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hFile, GENERIC_WRITE, &objectAttributes, &iosb, NULL,
		FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN_IF,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwCreateFile", status);
		return FALSE;
	}
	// 读取文件数据
	RtlZeroMemory(&iosb, sizeof(iosb));
	status = ZwWriteFile(hFile, NULL, NULL, NULL, &iosb,
		pWriteData, *pulWriteDataSize, &liOffset, NULL);
	if (!NT_SUCCESS(status))
	{
		*pulWriteDataSize = iosb.Information;
		ZwClose(hFile);
		ShowError("ZwCreateFile", status);
		return FALSE;
	}
	// 获取实际写入的数据
	*pulWriteDataSize = iosb.Information;
	// 关闭句柄
	ZwClose(hFile);

	return TRUE;
}


// 文件复制
BOOLEAN MyCopyFile(UNICODE_STRING ustrScrFile, UNICODE_STRING ustrDestFile)
{
	ULONG ulBufferSize = 40960;
	ULONG ulReadDataSize = ulBufferSize;
	LARGE_INTEGER liOffset = { 0 };
	PUCHAR pBuffer = ExAllocatePool(NonPagedPool, ulBufferSize);
	
	// 一边读取, 一边写入, 实现文件复制
	do
	{
		// 读取文件
		ulReadDataSize = ulBufferSize;
		MyReadFile(ustrScrFile, liOffset, pBuffer, &ulReadDataSize);
		// 若读取的数据为空的时候, 结束复制操作
		if (0 >= ulReadDataSize)
		{
			break;
		}

		// 写入文件
		MyWriteFile(ustrDestFile, liOffset, pBuffer, &ulReadDataSize);
		
		// 更新偏移
		liOffset.QuadPart = liOffset.QuadPart + ulReadDataSize;

	} while (TRUE);

	// 释放内存
	ExFreePool(pBuffer);

	return TRUE;
}