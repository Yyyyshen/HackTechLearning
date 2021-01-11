#ifndef _MONITOR_FILE_H_
#define _MONITOR_FILE_H_


#include <Windows.h>
#include <stdio.h>

// 宽字节字符串转多字节字符串
void W2C(wchar_t *pwszSrc, int iSrcLen, char *pszDest, int iDestLen);

// 目录监控多线程
UINT MonitorFileThreadProc(LPVOID lpVoid);

// 创建目录监控多线程
void MonitorFile(const char *pszDirectory);


#endif