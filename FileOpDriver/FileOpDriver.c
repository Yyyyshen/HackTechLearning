/**
 * 文件管理一直是各层非常重要的操作
 * 恶意程序也同样需要
 * 下面三种内核下的文件管理技术，一个比一个更底层
 * 分别是：
 * 内核API
 * 构造输入输出请求包（I/O Request Package）并发送IRP来操作文件
 * 根据文件系统格式NTFS，解析硬盘二进制数据
 */

 /**
  * Zw开头文件操作API
  */
#include "FileManage.h"


VOID TestKernelAPI()
{
	// 创建目录
	UNICODE_STRING ustrDirectory;
	RtlInitUnicodeString(&ustrDirectory, L"\\??\\C:\\MyCreateFolder"); //从这样的路径格式，想起来之前在资源管理器复制出来的路径前面就是带有两个??
	MyCreateFileFolder(ustrDirectory);

	// 创建文件
	UNICODE_STRING ustrCreateFile;
	RtlInitUnicodeString(&ustrCreateFile, L"\\??\\C:\\MyCreateFolder\\MyCreateFile.txt");
	MyCreateFile(ustrCreateFile);

	// 删除文件
	UNICODE_STRING ustrDeleteFile;
	RtlInitUnicodeString(&ustrDeleteFile, L"\\??\\C:\\MyCreateFolder\\MyCreateFile.txt");
	MyDeleteFileOrFileFolder(ustrDeleteFile);

	// 删除空目录
	UNICODE_STRING ustrDeleteFilder;
	RtlInitUnicodeString(&ustrDeleteFilder, L"\\??\\C:\\MyCreateFolder\\Test");
	MyDeleteFileOrFileFolder(ustrDeleteFilder);

	// 获取文件大小
	UNICODE_STRING ustrFileSize;
	RtlInitUnicodeString(&ustrFileSize, L"\\??\\C:\\MyCreateFolder\\MyCreateFile.txt");
	ULONG64 ullFileSize = MyGetFileSize(ustrFileSize);
	DbgPrint("FileSize = %I64d Bytes\n", ullFileSize);

	// 重命名目录
	UNICODE_STRING ustrOldFilder, ustrNewFilder;
	RtlInitUnicodeString(&ustrOldFilder, L"\\??\\C:\\MyCreateFolder\\Test");
	RtlInitUnicodeString(&ustrNewFilder, L"\\??\\C:\\MyCreateFolder\\TestRename");
	MyRenameFileOrFileFolder(ustrOldFilder, ustrNewFilder);

	// 重命名文件
	UNICODE_STRING ustrOldFile, ustrNewFile;
	RtlInitUnicodeString(&ustrOldFile, L"\\??\\C:\\MyCreateFolder\\MyCreateFile.txt");
	RtlInitUnicodeString(&ustrNewFile, L"\\??\\C:\\MyCreateFolder\\MyCreateFileRename.txt");
	MyRenameFileOrFileFolder(ustrOldFile, ustrNewFile);

	// 遍历文件夹和文件
	UNICODE_STRING ustrQueryFile;
	RtlInitUnicodeString(&ustrQueryFile, L"\\??\\C:\\MyCreateFolder");
	MyQueryFileAndFileFolder(ustrQueryFile);

	// 文件复制
	UNICODE_STRING ustrScrFile, ustrDestFile;
	RtlInitUnicodeString(&ustrScrFile, L"\\??\\C:\\MyCreateFolder\\MyCreateFile.txt");
	RtlInitUnicodeString(&ustrDestFile, L"\\??\\C:\\MyCreateFolder\\MyCreateFile-Copy.txt");
	MyCopyFile(ustrScrFile, ustrDestFile);

}

/**
 * 文件管理之IRP
 * 用户程序通常调用API操作文件，依次涉及Win32API、Native API、File System以及Filter Driver等层次
 * 杀软和恶意程序在各个层次都存在，对操作进行过滤和监控
 * 其中FSD是文件API函数经过Native API后到达驱动层
 * 驱动程序可以模拟操作系统向FSD发送IRP来管理文件，绕过设置在FSD上面的API HOOK等监控程序
 */
#include "IrpFile.h"
#include "FileManageWithIrp.h"
#define DEV_NAME L"\\Device\\IRP_FILE_DEV_NAME"
#define SYM_NAME L"\\DosDevices\\IRP_FILE_SYM_NAME"
#define IOCTL_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
VOID TESTIRP()
{
	// 创建文件
	UNICODE_STRING ustrCreateFile;
	RtlInitUnicodeString(&ustrCreateFile, L"C:\\TEST.txt");
	IRP_MyCreateFile(ustrCreateFile);
	DbgPrint("Create File OK.\n");

	// 获取文件大小
	UNICODE_STRING ustrFileSize;
	RtlInitUnicodeString(&ustrFileSize, L"C:\\TEST.exe");
	ULONG64 ullFileSize = IRP_MyGetFileSize(ustrFileSize);
	DbgPrint("FileSize = %I64d Bytes\n", ullFileSize);

	// 设置文件隐藏属性
	UNICODE_STRING ustrHideFile;
	RtlInitUnicodeString(&ustrHideFile, L"C:\\TEST.exe");
	IRP_MyHideFile(ustrHideFile);
	DbgPrint("Hide File OK.\n");

	// 遍历文件夹和文件
	UNICODE_STRING ustrQueryFile;
	RtlInitUnicodeString(&ustrQueryFile, L"C:\\");
	IRP_MyQueryFileAndFileFolder(ustrQueryFile);
	DbgPrint("Query File OK.\n");

	// 文件数据写入
	UNICODE_STRING ustrWriteFile;
	LARGE_INTEGER liWriteOffset = { 0 };
	UCHAR szWriteData[256] = "Who Are You? I am Demon`Gan.";
	ULONG ulWriteDataLength = 1 + strlen(szWriteData);
	RtlInitUnicodeString(&ustrWriteFile, L"C:\\TEST.txt");
	IRP_MyWriteFile(ustrWriteFile, liWriteOffset, szWriteData, &ulWriteDataLength);
	DbgPrint("MyWriteFile OK.\n");

	// 文件数据读取
	UNICODE_STRING ustrReadFile;
	LARGE_INTEGER liReadOffset = { 0 };
	UCHAR szReadData[256] = { 0 };
	ULONG ulReadDataLength = 256;
	RtlInitUnicodeString(&ustrReadFile, L"C:\\TEST.txt");
	IRP_MyReadFile(ustrReadFile, liReadOffset, szReadData, &ulReadDataLength);
	DbgPrint("MyReadFile:[%s]\n", szReadData);
}

/**
 * 根据NTFS
 * 硬盘中存储的是大量的0、1二进制数据
 * 文件系统对硬盘数据设置格式规则
 */
VOID TESTNTFS()
{
	//见另一个项目：FileOpNTFS
}

#include "ntddk.h" // 这个声明要放到后面，因为文件管理中使用了ntifs.h，放前面的话编译会出现重定义错误
//IRP操作需要
NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Enter CreateDevice\n");
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDevObj = NULL;
	UNICODE_STRING ustrDevName, ustrSymName;
	RtlInitUnicodeString(&ustrDevName, DEV_NAME);
	RtlInitUnicodeString(&ustrSymName, SYM_NAME);

	status = IoCreateDevice(pDriverObject, 0, &ustrDevName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDevObj);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IoCreateDevice Error[0x%X]\n", status);
		return status;
	}

	status = IoCreateSymbolicLink(&ustrSymName, &ustrDevName);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IoCreateSymbolicLink Error[0x%X]\n", status);
		return status;
	}

	DbgPrint("Leave CreateDevice\n");
	return status;
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Enter DriverUnload\n");

	if (pDriverObject->DeviceObject)
	{
		IoDeleteDevice(pDriverObject->DeviceObject);
	}
	UNICODE_STRING ustrSymName;
	RtlInitUnicodeString(&ustrSymName, SYM_NAME);
	IoDeleteSymbolicLink(&ustrSymName);

	DbgPrint("Leave DriverUnload\n");
}
//IRP操作用到的
NTSTATUS DriverControlHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("Enter DriverControlHandle\n");
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIoStackLocation = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulInputLen = pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
	ULONG ulOutputLen = pIoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG ulControlCode = pIoStackLocation->Parameters.DeviceIoControl.IoControlCode;
	PVOID pBuffer = pIrp->AssociatedIrp.SystemBuffer;
	ULONG ulInfo = 0;

	switch (ulControlCode)
	{
	case IOCTL_TEST:
	{
		break;
	}
	default:
		break;
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulInfo;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	DbgPrint("Leave DriverControlHandle\n");
	return status;
}

NTSTATUS DriverDefaultHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}
//驱动入口
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DbgPrint("Enter DriverEntry\n");
	NTSTATUS status = STATUS_SUCCESS;

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverDefaultHandle;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverDefaultHandle;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverControlHandle;


	//使用内核API方式操作文件
	//TestKernelAPI();

	//发送IRP操作文件
	status = CreateDevice(DriverObject);
	TESTIRP();

	DbgPrint("Leave DriverEntry\n");
	return status;
}