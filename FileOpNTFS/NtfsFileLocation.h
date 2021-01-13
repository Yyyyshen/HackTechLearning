#ifndef _NTFS_FILE_LOCATION_H_
#define _NTFS_FILE_LOCATION_H_
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

// 打开磁盘
BOOL OpenDisk(HANDLE &hFile, char *lpszDrive);

// 从DBR中获取数据：每个扇区字节数、每个簇的扇区数、原文件$MFT的起始簇号
BOOL GetDataFromDBR(HANDLE hFile, WORD &wSizeOfSector, BYTE &bSizeOfCluster, LARGE_INTEGER &liClusterNumberOfMFT);

// 定位文件
BOOL LocationFile(HANDLE hFile, char *lpszFileName, WORD wSizeOfSector, BYTE bSizeOfCluster, LARGE_INTEGER liMFTOffset, LARGE_INTEGER &liRootOffset);

// 0x90属性的处理
BOOL HandleAttribute_90(BYTE *lpBuffer, WORD wAttributeOffset, BYTE *lpUnicode, DWORD dwLen, LARGE_INTEGER liMFTOffset, LARGE_INTEGER &liRootOffset);

// 0xA0属性的处理
BOOL HandleAttribute_A0(HANDLE hFile, BYTE *lpBuffer, WORD wSizeOfSector, BYTE bSizeOfCluster, WORD wAttributeOffset, BYTE *lpUnicode, DWORD dwLen, LARGE_INTEGER liMFTOffset, LARGE_INTEGER &liRootOffset);

// 内存比较
BOOL CompareMemory(BYTE *lpSrc, BYTE *lpDst, DWORD dwLen);

// 读取文件内容偏移
BOOL FileContentOffset(HANDLE hFile, WORD wSizeOfSector, BYTE bSizeOfCluster, LARGE_INTEGER liMFTOffset, LARGE_INTEGER liRootOffset);


#endif