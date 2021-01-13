// FileOpNTFS.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "NtfsFileLocation.h"

/**
 * 根据NTFS格式操作文件
 */
	void test_ntfs()
{
	// 输入路径
	printf("Input The File Path:\n");
	char szFilePath[MAX_PATH] = { 0 };
	gets_s(szFilePath);
	// 打开磁盘
	HANDLE hFile = NULL;
	if (!OpenDisk(hFile, szFilePath))
	{
		return;
	}
	// 获取扇区大小(2)、簇大小(1)、$MFT起始簇号(8)
	WORD wSizeOfSector = 0;
	BYTE bSizeOfCluster = 0;
	LARGE_INTEGER liClusterNumberOfMFT;
	GetDataFromDBR(hFile, wSizeOfSector, bSizeOfCluster, liClusterNumberOfMFT);

	// 计算$MFT元文件的字节偏移
	LARGE_INTEGER liMFTOffset;
	liMFTOffset.QuadPart = liClusterNumberOfMFT.QuadPart * bSizeOfCluster * wSizeOfSector;

	// 计算根目录，与$MFT相距5个目录，每个目录大小固定为1KB(0x400)
	LARGE_INTEGER liRootOffset;
	liRootOffset.QuadPart = liMFTOffset.QuadPart + 5 * 0x400;

	// 文件定位
	LocationFile(hFile, szFilePath, wSizeOfSector, bSizeOfCluster, liMFTOffset, liRootOffset);
	// 显示逻辑字节偏移和文件号
	printf("Location File:0x%llx\n", liRootOffset.QuadPart);

	// 80H属性 获取文件数据内容偏移
	FileContentOffset(hFile, wSizeOfSector, bSizeOfCluster, liMFTOffset, liRootOffset);

	printf("\n");
	// 关闭文件并退出
	::CloseHandle(hFile);
	system("pause");
}

int main()
{
	test_ntfs();
	return 0;
}
