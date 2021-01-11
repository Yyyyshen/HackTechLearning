#include "FileSearch.h"


void SearchFile(const char* pszDirectory)
{
	// 搜索指定类型文件
	DWORD dwBufferSize = 2048;
	char* pszFileName = NULL;
	char* pTempSrc = NULL;
	WIN32_FIND_DATA FileData = { 0 };
	BOOL bRet = FALSE;

	// 申请动态内存
	pszFileName = new char[dwBufferSize];
	pTempSrc = new char[dwBufferSize];

	// 构造搜索文件类型字符串, *.*表示搜索所有文件类型
	::wsprintf(pszFileName, "%s\\*.*", pszDirectory);

	// 搜索第一个文件
	HANDLE hFile = ::FindFirstFile(pszFileName, &FileData);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		do
		{
			// 要过滤掉 当前目录"." 和 上一层目录"..", 否则会不断进入死循环遍历
			if ('.' == FileData.cFileName[0])
			{
				continue;
			}
			// 拼接文件路径	
			::wsprintf(pTempSrc, "%s\\%s", pszDirectory, FileData.cFileName);
			// 判断是否是目录还是文件
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// 目录, 则继续往下递归遍历文件
				SearchFile(pTempSrc);
			}
			else
			{
				// 文件
				printf("%s\n", pTempSrc);
			}

			// 搜索下一个文件
		} while (::FindNextFile(hFile, &FileData));
	}

	// 关闭文件句柄
	::FindClose(hFile);
	// 释放内存
	delete[]pTempSrc;
	pTempSrc = NULL;
	delete[]pszFileName;
	pszFileName = NULL;
}