#include "FileProtect.h"


PFILE_OBJECT ProtectFile(UNICODE_STRING ustrFileName)
{
	PFILE_OBJECT pFileObject = NULL;
	IO_STATUS_BLOCK iosb = { 0 };
	NTSTATUS status = STATUS_SUCCESS;

	// 创建或者打开文件
	status = IrpCreateFile(&pFileObject, DELETE | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
		&ustrFileName, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		return pFileObject;
	}

	return pFileObject;
}


BOOLEAN UnprotectFile(PFILE_OBJECT pFileObject)
{
	if (pFileObject)
	{
		ObDereferenceObject(pFileObject);
	}
	return TRUE;
}
