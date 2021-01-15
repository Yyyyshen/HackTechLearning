#include "ForceDelete.h"


// 强制删除文件
NTSTATUS ForceDeleteFile(UNICODE_STRING ustrFileName)
{
	NTSTATUS status = STATUS_SUCCESS;
	PFILE_OBJECT pFileObject = NULL;
	IO_STATUS_BLOCK iosb = { 0 };
	FILE_BASIC_INFORMATION fileBaseInfo = { 0 };
	FILE_DISPOSITION_INFORMATION fileDispositionInfo = { 0 };
	PVOID pImageSectionObject = NULL;
	PVOID pDataSectionObject = NULL;
	PVOID pSharedCacheMap = NULL;

	// 发送IRP打开文件
	status = IrpCreateFile(&pFileObject, GENERIC_READ | GENERIC_WRITE, &ustrFileName,
		&iosb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IrpCreateFile Error[0x%X]\n", status);
		return FALSE;
	}

	// 发送IRP设置文件属性, 去掉只读属性, 修改为 FILE_ATTRIBUTE_NORMAL
	RtlZeroMemory(&fileBaseInfo, sizeof(fileBaseInfo));
	fileBaseInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
	status = IrpSetInformationFile(pFileObject, &iosb, &fileBaseInfo, sizeof(fileBaseInfo), FileBasicInformation);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IrpSetInformationFile[SetInformation] Error[0x%X]\n", status);
		return status;
	}

	// 清空PSECTION_OBJECT_POINTERS结构
	if (pFileObject->SectionObjectPointer)
	{
		// 保存旧值
		pImageSectionObject = pFileObject->SectionObjectPointer->ImageSectionObject;
		pDataSectionObject = pFileObject->SectionObjectPointer->DataSectionObject;
		pSharedCacheMap = pFileObject->SectionObjectPointer->SharedCacheMap;
		// 置为空
		pFileObject->SectionObjectPointer->ImageSectionObject = NULL;
		pFileObject->SectionObjectPointer->DataSectionObject = NULL;
		pFileObject->SectionObjectPointer->SharedCacheMap = NULL;
	}

	// 发送IRP设置文件属性, 设置删除文件操作
	RtlZeroMemory(&fileDispositionInfo, sizeof(fileDispositionInfo));
	fileDispositionInfo.DeleteFile = TRUE;
	status = IrpSetInformationFile(pFileObject, &iosb, &fileDispositionInfo, sizeof(fileDispositionInfo), FileDispositionInformation);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IrpSetInformationFile[DeleteFile] Error[0x%X]\n", status);
		return status;
	}

	//还原旧值  
	if (pFileObject->SectionObjectPointer)
	{
		pFileObject->SectionObjectPointer->ImageSectionObject = pImageSectionObject;
		pFileObject->SectionObjectPointer->DataSectionObject = pDataSectionObject;
		pFileObject->SectionObjectPointer->SharedCacheMap = pSharedCacheMap;
	}

	// 关闭文件对象
	ObDereferenceObject(pFileObject);

	return status;
}



