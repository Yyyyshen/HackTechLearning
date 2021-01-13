#ifndef _FILE_MANAGE_WITH_IRP_H_
#define _FILE_MANAGE_WITH_IRP_H


#include <ntifs.h>
#include <ntstatus.h>

// 注意: 路径不需要加 \??\ 作为前缀


// 创建或打开文件
BOOLEAN IRP_MyCreateFile(UNICODE_STRING ustrFilePath);

// 获取文件大小
ULONG64 IRP_MyGetFileSize(UNICODE_STRING ustrFileName);

// 设置文件隐藏属性
BOOLEAN IRP_MyHideFile(UNICODE_STRING ustrFileName);

// 遍历文件夹和文件
BOOLEAN IRP_MyQueryFileAndFileFolder(UNICODE_STRING ustrPath);

// 读取文件数据
BOOLEAN IRP_MyReadFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pReadData, PULONG pulReadDataSize);

// 向文件写入数据
BOOLEAN IRP_MyWriteFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pWriteData, PULONG pulWriteDataSize);


#endif