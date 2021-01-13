#ifndef _FILE_MANAGE_H_
#define _FILE_MANAGE_H


#include <ntifs.h>


// 创建文件
BOOLEAN MyCreateFile(UNICODE_STRING ustrFilePath);

// 创建目录
BOOLEAN MyCreateFileFolder(UNICODE_STRING ustrFileFolderPath);

// 删除文件或是空目录
BOOLEAN MyDeleteFileOrFileFolder(UNICODE_STRING ustrFileName);

// 获取文件大小
ULONG64 MyGetFileSize(UNICODE_STRING ustrFileName);

// 重命名文件或文件夹
BOOLEAN MyRenameFileOrFileFolder(UNICODE_STRING ustrSrcFileName, UNICODE_STRING ustrDestFileName);

// 遍历文件夹和文件
BOOLEAN MyQueryFileAndFileFolder(UNICODE_STRING ustrPath);

// 读取文件数据
BOOLEAN MyReadFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pReadData, PULONG pulReadDataSize);

// 向文件写入数据
BOOLEAN MyWriteFile(UNICODE_STRING ustrFileName, LARGE_INTEGER liOffset, PUCHAR pWriteData, PULONG pulWriteDataSize);

// 文件复制
BOOLEAN MyCopyFile(UNICODE_STRING ustrScrFile, UNICODE_STRING ustrDestFile);


#endif