#ifndef _IRP_FILE_H_
#define _IRP_FILE_H_


#include <ntifs.h>


typedef struct _AUX_ACCESS_DATA {
	PPRIVILEGE_SET PrivilegesUsed;
	GENERIC_MAPPING GenericMapping;
	ACCESS_MASK AccessesToAudit;
	ACCESS_MASK MaximumAuditMask;
	ULONG Unknown[256];
} AUX_ACCESS_DATA, *PAUX_ACCESS_DATA;


NTSTATUS SeCreateAccessState(
	PACCESS_STATE AccessState,
	PVOID AuxData,
	ACCESS_MASK DesiredAccess,
	PGENERIC_MAPPING GenericMapping
	);

NTSTATUS ObCreateObject(
	__in KPROCESSOR_MODE ProbeMode,            // 决定是否要验证参数
	__in POBJECT_TYPE ObjectType,              // 对象类型指针
	__in POBJECT_ATTRIBUTES ObjectAttributes,  // 对象的属性, 最终会转化成ObAllocateObject需要的OBJECT_CREATE_INFORMATION结构
	__in KPROCESSOR_MODE OwnershipMode,        // 内核对象?用户对象? 同上
	__inout_opt PVOID ParseContext,            // 这参数没用
	__in ULONG ObjectBodySize,                 // 对象体大小
	__in ULONG PagedPoolCharge,                // ...
	__in ULONG NonPagedPoolCharge,             // ...
	__out PVOID *Object                        // 接收对象体的指针
	);




// 完成实例, 设置事件信号量, 并释放IRP
NTSTATUS MyCompleteRoutine(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP pIrp,
	IN PVOID Context);


// 创建或者打开文件
// ZwCreateFile
NTSTATUS IrpCreateFile(
	OUT PFILE_OBJECT *ppFileObject,
	IN ACCESS_MASK DesiredAccess,
	IN PUNICODE_STRING pustrFilePath,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN PLARGE_INTEGER AllocationSize OPTIONAL,
	IN ULONG FileAttributes,
	IN ULONG ShareAccess,
	IN ULONG CreateDisposition,
	IN ULONG CreateOptions,
	IN PVOID EaBuffer OPTIONAL,
	IN ULONG EaLength);

// 文件遍历
// ZwQueryDirectoryFile
NTSTATUS IrpQueryDirectoryFile(
	IN PFILE_OBJECT pFileObject,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID FileInformation,
	IN ULONG Length,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	IN PUNICODE_STRING FileName OPTIONAL);

// 获取文件的信息
// ZwQueryInformationFile
NTSTATUS IrpQueryInformationFile(
	IN PFILE_OBJECT pFileObject,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID FileInformation,
	IN ULONG Length,
	IN FILE_INFORMATION_CLASS FileInformationClass
	);


// 设置文件信息
// ZwSetInformationFile
NTSTATUS IrpSetInformationFile(
	IN PFILE_OBJECT pFileObject,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN PVOID FileInformation,
	IN ULONG Length,
	IN FILE_INFORMATION_CLASS FileInformationClass
	);

// 读文件
// ZwReadFile
NTSTATUS IrpReadFile(
	IN PFILE_OBJECT pFileObject,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID Buffer,
	IN ULONG Length,
	IN PLARGE_INTEGER ByteOffset OPTIONAL
	);

// 写文件
// ZwWriteFile
NTSTATUS IrpWriteFile(
	IN PFILE_OBJECT pFileObject,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN PVOID Buffer,
	IN ULONG Length,
	IN PLARGE_INTEGER ByteOffset OPTIONAL
	);


#endif