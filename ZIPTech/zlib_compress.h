#ifndef _ZLIB_COMPRESS_H_
#define _ZLIB_COMPRESS_H_

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
//*************************************************
//         zlib压缩库的头文件和静态库
//*************************************************
#include "zlib\\zconf.h"
#include "zlib\\zlib.h"
#ifdef _DEBUG
	#ifdef _WIN64
		#pragma comment(lib, "zlib\\x64\\debug\\zlibstat.lib")
	#else
		#pragma comment(lib, "zlib\\x86\\debug\\zlibstat.lib")
	#endif
#else
	#ifdef _WIN64
		#pragma comment(lib, "zlib\\x64\\release\\zlibstat.lib")
	#else
		#pragma comment(lib, "zlib\\x86\\release\\zlibstat.lib")
	#endif
#endif
//*************************************************


#define MAX_SRC_FILE_SIZE (100*1024*1024)			// 单个文件限制大小为 100M 


// 数据压缩
// 输入：将要压缩文件的路径
// 输出：数据压缩后的压缩数据内容、数据压缩后的压缩数据内容长度
BOOL Zlib_CompressData(const char *pszCompressFileName, BYTE **ppCompressData, DWORD *pdwCompressDataSize);


// 数据解压
// 输入：将要解压缩文件的路径
// 输出：数据解压后的数据内容、数据解压后的内容长度
BOOL Zlib_UncompressData(const char *pszUncompressFileName, BYTE **ppUncompressData, DWORD *pdwUncompressDataSize);


// 数据解压
// 输入：将要解压缩的数据内容、将要解压缩的数据内容长度
// 输出：数据解压后的数据内容、数据解压后的内容长度
BOOL Zlib_UncompressData(BYTE *pCompressData, DWORD dwCompressDataSize, BYTE **ppUncompressData, DWORD *pdwUncompressDataSize);


// 将数据存储为文件
// 输入：数据原文件路径、将要保存的数据内容、将要保存的数据内容长度
BOOL SaveToFile(const char *pszFileName, BYTE *pData, DWORD dwDataSize);


// 将数据存储为文件
// 输入：数据原文件路径、将要保存的数据内容、将要保存的数据内容长度
BOOL SaveToOriginalFile(const char *pszFileName, BYTE *pData, DWORD dwDataSize);

#endif