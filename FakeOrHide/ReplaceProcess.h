#ifndef _REPLACE_PROCESS_H_
#define _REPLACE_PROCESS_H_


#include <Windows.h>


// 创建进程并替换进程内存数据, 更改执行顺序
BOOL ReplaceProcess(const char* pszFilePath, PVOID pReplaceData, DWORD dwReplaceDataSize, DWORD dwRunOffset);


#endif