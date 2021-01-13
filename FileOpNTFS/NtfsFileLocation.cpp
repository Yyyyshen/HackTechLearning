#include "NtfsFileLocation.h"
#include "stdio.h"
/*
定位文件路径：F:\test3\iujklm684\test1.txt

1. 读取0x0B开始的2个字节，表示每个扇区的大小:0x200
2. 读取0x0D开始的1个字节，表示每个簇由几个扇区组成:0x8
3. 读取0x30开始的8个字节，表示$MFT的起始簇号:0xc0000

可以根据以上信息计算得出，$MFT元文件的偏移是：
0xc0000 * 0x8 * 0x200 = 0xc0000000

4. 每个文件目录的大小规定是1KB(0x400)，即1024字节，而且
根目录与$MFT元文件偏移5个文件，即5号文件目录

可以根据以上信息计算得出，根目录的偏移是：
0xc0000000 + 0x400 * 5 = 0xc0001400

5. 由于文件目录的深度和存储数据的大小不同，获取定位使用的
属性也不同，要注意0x80、0x90、0xA0这三个属性；
6. 在元文件的目录头，偏移0x14开始的两个字节，是该目录第
一个属性的偏移；
7. 在属性中，前4个字节，表示属性；偏移0x4开始的4个字节，是该属性的大小

根据以上，我们可以遍历各个文件目录中的各个属性，
以此来根据不同的属性做出不同的处理

8. 属性读取的优先级是：0xA0 > 0x90 > 0x80；

0xA0读取最后8字节，
Data Run List:11 01 2c 00 00 00 00 00
(PS:Data Run可以有多个，但是记住之后的Data Run要加上之前的Data的簇大小偏移！！！)
上面的Data Run List表示数据的逻辑簇号是用1字节表示：2c
数据的长度是用1字节表示：01
计算出逻辑簇号0x2c的偏移：
0x2c * 0x8 * 0x200 = 0x2c000
来到INDX索引中，搜索UNICODE字符串，找到后，往上数5行，减去0x50得到文件号！
所在行的地址是：0x2c7c8,往上数5行后，0x2c7c8 - 0x50 = 0x2c778
读取0x2c778开始的6个字节，即可获得文件号，文件号都是以$MFT为开始的文件号，
每个文件大小都是1KB(0x400)
文件号是：2e 00 00 00 00 00
对应得文件或目录(test3目录)的偏移是：
0xc0000000 + 0x2e * 0x400 = 0xc000b800
来到新的目录下了，重复步骤8的操作；

这次读取0x90的属性，先扫描一边0x90中的常驻数据是否有我们要搜索的
文件或目录，使用UNICODE来搜索即可，若有，继续往上数5行，读取文件号即可；
若没有，则需要读取最后8字节的Data Run，继续在索引中去UNICODE搜索，重复操作；

扫描了一下，发现“iujklm684”UNICODE在0x90的属性中出现了，于是往上数5行，
获取文件号为：2f 00 00 00 00 00
计算文件的偏移：
0xc0000000 + 0x2f * 0x400 = 0xc000bc00
来到了新的目录下，继续重读步骤8的操作；

在0x90的属性中，搜索到“test1.txt”UNICODE，往上数5行，
读取前6字节，获取文件号是：35 00 00 00 00 00
计算文件的偏移：
0xc0000000 + 0x35 * 0x400 = 0xc000d400
来到了文件下了，可以根据0x30的属性来判断是否是目录还是文件！！！

9. 文件的数据存储在0x80的属性中，数据小的话，作为常驻数据直接在0x80属性中
获取；数据多的话，作为非常驻数据存储在Data Run的索引里！
*/


// 打开磁盘
BOOL OpenDisk(HANDLE &hFile, char *lpszFilePath)
{
	// 构造磁盘设备字符串
	char szDrive[7] = { '\\', '\\', '.', '\\', 'C', ':', '\0' };
	szDrive[4] = lpszFilePath[0];
	// 打开磁盘
	hFile = ::CreateFile(szDrive, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		char szErr[MAX_PATH] = { 0 };
		::wsprintf(szErr, "Create File Error!\nError Code Is:%d\n", ::GetLastError());
		printf("%s", szErr);
		system("pause");
		return FALSE;
	}

	return TRUE;
}


// 从DBR中获取数据：每个扇区字节数、每个簇的扇区数、原文件$MFT的起始簇号
BOOL GetDataFromDBR(HANDLE hFile, WORD &wSizeOfSector, BYTE &bSizeOfCluster, LARGE_INTEGER &liClusterNumberOfMFT)
{
	// 获取扇区大小(2)、簇大小(1)、$MFT起始簇号(8)
	BYTE bBuffer[512] = { 0 };
	DWORD dwRead = 0;
	// 注意：数据读取的大小最小单位是扇区!!!
	::SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	::ReadFile(hFile, bBuffer, 512, &dwRead, NULL);
	wSizeOfSector = MAKEWORD(bBuffer[0x0B], bBuffer[0x0C]);
	bSizeOfCluster = bBuffer[0x0D];
	liClusterNumberOfMFT.LowPart = MAKELONG(MAKEWORD(bBuffer[0x30], bBuffer[0x31]), MAKEWORD(bBuffer[0x32], bBuffer[0x33]));
	liClusterNumberOfMFT.HighPart = MAKELONG(MAKEWORD(bBuffer[0x34], bBuffer[0x35]), MAKEWORD(bBuffer[0x36], bBuffer[0x37]));

	return TRUE;
}


// 定位文件
BOOL LocationFile(HANDLE hFile, char *lpszFilePath, WORD wSizeOfSector, BYTE bSizeOfCluster, LARGE_INTEGER liMFTOffset, LARGE_INTEGER &liRootOffset)
{
	BYTE bBuffer[1024] = { 0 };
	DWORD dwRead = 0;
	// 分割文件路径
	char szNewFile[MAX_PATH] = { 0 };
	::lstrcpy(szNewFile, (lpszFilePath + 3));
	char szDelim[] = "\\";
	char *lpResult = strtok(szNewFile, szDelim);
	BYTE bUnicode[MAX_PATH] = { 0 };
	while (NULL != lpResult)
	{
		BOOL bFlag = FALSE;
		DWORD dwNameOffset = 0;
		// 将分割的目录转换成2字节表示的Unicode数据
		DWORD dwLen = ::lstrlen(lpResult);
		::RtlZeroMemory(bUnicode, MAX_PATH);
		for (DWORD i = 0, j = 0; i < dwLen; i++)
		{
			bUnicode[j++] = lpResult[i];
			bUnicode[j++] = 0;
		}

		// 读取目录的数据，大小为1KB
		::SetFilePointer(hFile, liRootOffset.LowPart, &liRootOffset.HighPart, FILE_BEGIN);
		::ReadFile(hFile, bBuffer, 1024, &dwRead, NULL);
		// 获取第一个属性的偏移
		WORD wAttributeOffset MAKEWORD(bBuffer[0x14], bBuffer[0x15]);
		// 遍历文件目录的属性
		DWORD dwAttribute = 0;
		DWORD dwSizeOfAttribute = 0;
		while (TRUE)
		{
			if (bFlag)
			{
				break;
			}
			// 获取当前属性
			dwAttribute = MAKELONG(MAKEWORD(bBuffer[wAttributeOffset], bBuffer[wAttributeOffset + 1]),
				MAKEWORD(bBuffer[wAttributeOffset + 2], bBuffer[wAttributeOffset + 3]));

			// 判断属性
			if (0x90 == dwAttribute)
			{
				bFlag = HandleAttribute_90(bBuffer, wAttributeOffset, bUnicode, dwLen, liMFTOffset, liRootOffset);
			}
			else if (0xA0 == dwAttribute)
			{
				bFlag = HandleAttribute_A0(hFile, bBuffer, wSizeOfSector, bSizeOfCluster, wAttributeOffset, bUnicode, dwLen, liMFTOffset, liRootOffset);
			}
			else if (0xFFFFFFFF == dwAttribute)
			{
				bFlag = TRUE;
				break;
			}
			// 获取当前属性的大小
			dwSizeOfAttribute = MAKELONG(MAKEWORD(bBuffer[wAttributeOffset + 4], bBuffer[wAttributeOffset + 5]),
				MAKEWORD(bBuffer[wAttributeOffset + 6], bBuffer[wAttributeOffset + 7]));
			// 计算下一属性的偏移
			wAttributeOffset = wAttributeOffset + dwSizeOfAttribute;
		}

		// 继续分割目录
		lpResult = strtok(NULL, szDelim);
	}

	return TRUE;
}


// 0x90属性的处理
BOOL HandleAttribute_90(BYTE *lpBuffer, WORD wAttributeOffset, BYTE *lpUnicode, DWORD dwLen, LARGE_INTEGER liMFTOffset, LARGE_INTEGER &liRootOffset)
{
	// 先遍历判断0x90属性里是否有此目录或文件(UNICODE)
	// 获取当前属性的大小
	DWORD dwSizeOfAttribute = MAKELONG(MAKEWORD(lpBuffer[wAttributeOffset + 4], lpBuffer[wAttributeOffset + 5]),
		MAKEWORD(lpBuffer[wAttributeOffset + 6], lpBuffer[wAttributeOffset + 7]));
	for (DWORD i = 0; i < dwSizeOfAttribute; i++)
	{
		if (CompareMemory(lpUnicode, (lpBuffer + wAttributeOffset + i), 2 * dwLen))
		{
			DWORD dwNameOffset = wAttributeOffset + i;
			// 计算文件号
			dwNameOffset = dwNameOffset / 8;
			dwNameOffset = 8 * dwNameOffset;
			dwNameOffset = dwNameOffset - 0x50;
			// 获取文件号(6)
			LARGE_INTEGER liNumberOfFile;
			liNumberOfFile.LowPart = MAKELONG(MAKEWORD(lpBuffer[dwNameOffset], lpBuffer[dwNameOffset + 1]),
				MAKEWORD(lpBuffer[dwNameOffset + 2], lpBuffer[dwNameOffset + 3]));
			liNumberOfFile.HighPart = MAKELONG(MAKEWORD(lpBuffer[dwNameOffset + 4], lpBuffer[dwNameOffset + 5]),
				0);

			// 计算文件号的偏移,文件号是相对$MFT为偏移说的
			liRootOffset = liNumberOfFile;
			liRootOffset.QuadPart = liMFTOffset.QuadPart + liRootOffset.QuadPart * 0x400;

			return TRUE;
		}
	}
	// 读取Data Run List，去到索引处INDX遍历UNICODE，获取文件号

	return FALSE;
}


// 0xA0属性的处理
BOOL HandleAttribute_A0(HANDLE hFile, BYTE *lpBuffer, WORD wSizeOfSector, BYTE bSizeOfCluster, WORD wAttributeOffset, BYTE *lpUnicode, DWORD dwLen, LARGE_INTEGER liMFTOffset, LARGE_INTEGER &liRootOffset)
{
	// 读取Data Run List，去到索引处INDX遍历UNICODE，获取文件号
	DWORD dwCount = 0;
	DWORD dwClusterOffet = 0;
	// 获取索引号的偏移
	WORD wIndxOffset = MAKEWORD(lpBuffer[wAttributeOffset + 0x20], lpBuffer[wAttributeOffset + 0x21]);
	// 读取Data Run List
	while (TRUE)
	{
		BYTE bTemp = lpBuffer[wAttributeOffset + wIndxOffset + dwCount];
		// 读取Data Run List,分解并计算Data Run中的信息
		BYTE bHi = bTemp >> 4;
		BYTE bLo = bTemp & 0x0F;
		if (0x0F == bHi || 0x0F == bLo || 0 == bHi || 0 == bLo)
		{
			break;
		}
		LARGE_INTEGER liDataRunSize, liDataRunOffset;
		liDataRunSize.QuadPart = 0;
		liDataRunOffset.QuadPart = 0;
		for (DWORD i = bLo; i > 0; i--)
		{
			liDataRunSize.QuadPart = liDataRunSize.QuadPart << 8;
			liDataRunSize.QuadPart = liDataRunSize.QuadPart | lpBuffer[wAttributeOffset + wIndxOffset + dwCount + i];
		}
		for (DWORD i = bHi; i > 0; i--)
		{
			liDataRunOffset.QuadPart = liDataRunOffset.QuadPart << 8;
			liDataRunOffset.QuadPart = liDataRunOffset.QuadPart | lpBuffer[wAttributeOffset + wIndxOffset + dwCount + bLo + i];
		}
		// 注意加上上一个Data Run的逻辑簇号
		liDataRunOffset.QuadPart = liDataRunOffset.QuadPart + dwClusterOffet;
		dwClusterOffet = dwClusterOffet + liDataRunOffset.LowPart;

		// 去到索引处INDX遍历UNICODE，获取文件号
		LARGE_INTEGER liIndxOffset, liIndxSize;
		liIndxOffset.QuadPart = liDataRunOffset.QuadPart * bSizeOfCluster * wSizeOfSector;
		liIndxSize.QuadPart = liDataRunSize.QuadPart * bSizeOfCluster * wSizeOfSector;
		// 读取索引的数据，大小为1KB
		BYTE *lpBuf = new BYTE[liIndxSize.QuadPart];
		DWORD dwRead = 0;
		::SetFilePointer(hFile, liIndxOffset.LowPart, &liIndxOffset.HighPart, FILE_BEGIN);
		::ReadFile(hFile, lpBuf, liIndxSize.LowPart, &dwRead, NULL);
		// 遍历Unicode数据
		for (DWORD i = 0; i < liIndxSize.LowPart; i++)
		{
			if (CompareMemory(lpUnicode, (lpBuf + i), 2 * dwLen))
			{
				DWORD dwNameOffset = i;
				// 计算文件号
				dwNameOffset = dwNameOffset / 8;
				dwNameOffset = 8 * dwNameOffset;
				dwNameOffset = dwNameOffset - 0x50;
				// 获取文件号(6)
				LARGE_INTEGER liNumberOfFile;
				liNumberOfFile.LowPart = MAKELONG(MAKEWORD(lpBuf[dwNameOffset], lpBuf[dwNameOffset + 1]),
					MAKEWORD(lpBuf[dwNameOffset + 2], lpBuf[dwNameOffset + 3]));
				liNumberOfFile.HighPart = MAKELONG(MAKEWORD(lpBuf[dwNameOffset + 4], lpBuf[dwNameOffset + 5]),
					0);

				// 计算文件号的偏移,文件号是相对$MFT为偏移说的
				liRootOffset = liNumberOfFile;
				liRootOffset.QuadPart = liMFTOffset.QuadPart + liRootOffset.QuadPart * 0x400;

				return TRUE;
			}
		}
		delete[]lpBuf;
		lpBuf = NULL;
		// 计算下一个Data Run List偏移
		dwCount = dwCount + bLo + bHi + 1;
	}

	return FALSE;
}


// 内存匹配
BOOL CompareMemory(BYTE *lpSrc, BYTE *lpDst, DWORD dwLen)
{
	if (0 == _memicmp(lpSrc, lpDst, dwLen))
	{
		return TRUE;
	}
	else
	{
		// 此方法仅用于6个字符以上
		if (12 >= dwLen)
		{
			return FALSE;
		}
		// 判断前后两个字符是否匹配
		if ((lpSrc[0] != lpDst[0]) || (lpSrc[1] != lpDst[1]) ||
			(lpSrc[dwLen - 2] != lpDst[dwLen - 2]) || (lpSrc[dwLen - 1] != lpDst[dwLen - 1]))
		{
			return FALSE;
		}
		// 前后字符匹配后，只允许2个字符不一样
		DWORD dwCount = 0;
		for (DWORD i = 0; i < dwLen; i++)
		{
			if (lpSrc[i] == lpDst[i])
			{
				dwCount++;
			}
		}
		if (2 < (dwLen - dwCount))
		{
			return FALSE;
		}
	}
	return TRUE;
}


// 读取文件内容偏移
BOOL FileContentOffset(HANDLE hFile, WORD wSizeOfSector, BYTE bSizeOfCluster, LARGE_INTEGER liMFTOffset, LARGE_INTEGER liRootOffset)
{
	BYTE bBuffer[1024] = { 0 };
	DWORD dwRead = 0;
	LARGE_INTEGER liContenOffset = liRootOffset;

	// 读取目录的数据，大小为1KB
	::SetFilePointer(hFile, liRootOffset.LowPart, &liRootOffset.HighPart, FILE_BEGIN);
	::ReadFile(hFile, bBuffer, 1024, &dwRead, NULL);
	// 获取第一个属性的偏移
	WORD wAttributeOffset MAKEWORD(bBuffer[0x14], bBuffer[0x15]);
	// 遍历文件目录的属性
	DWORD dwAttribute = 0;
	DWORD dwSizeOfAttribute = 0;
	while (TRUE)
	{
		// 获取当前属性
		dwAttribute = MAKELONG(MAKEWORD(bBuffer[wAttributeOffset], bBuffer[wAttributeOffset + 1]),
			MAKEWORD(bBuffer[wAttributeOffset + 2], bBuffer[wAttributeOffset + 3]));

		// 判断属性
		if (0x80 == dwAttribute)
		{
			// 读取偏移0x8出1字节，判断是否是常驻属性
			BYTE bFlag = bBuffer[wAttributeOffset + 0x8];
			if (0 == bFlag)        // 常驻
			{
				// 读取偏移0x14出2字节，即是内容的偏移
				WORD wContenOffset = MAKEWORD(bBuffer[wAttributeOffset + 0x14], bBuffer[wAttributeOffset + 0x15]);
				liContenOffset.QuadPart = liContenOffset.QuadPart + wAttributeOffset + wContenOffset;
				printf("File Content Offset:0x%llx\n\n", liContenOffset.QuadPart);
			}
			else                  // 非常驻
			{
				// 读取偏移0x20出2字节，即是数据运行列表偏移
				DWORD dwCount = 0;
				DWORD dwClusterOffet = 0;
				// 获取索引号的偏移
				WORD wIndxOffset = MAKEWORD(bBuffer[wAttributeOffset + 0x20], bBuffer[wAttributeOffset + 0x21]);
				// 读取Data Run List
				while (TRUE)
				{
					BYTE bTemp = bBuffer[wAttributeOffset + wIndxOffset + dwCount];
					// 读取Data Run List,分解并计算Data Run中的信息
					BYTE bHi = bTemp >> 4;
					BYTE bLo = bTemp & 0x0F;
					if (0x0F == bHi || 0x0F == bLo || 0 == bHi || 0 == bLo)
					{
						break;
					}
					LARGE_INTEGER liDataRunSize, liDataRunOffset;
					liDataRunSize.QuadPart = 0;
					liDataRunOffset.QuadPart = 0;
					for (DWORD i = bLo; i > 0; i--)
					{
						liDataRunSize.QuadPart = liDataRunSize.QuadPart << 8;
						liDataRunSize.QuadPart = liDataRunSize.QuadPart | bBuffer[wAttributeOffset + wIndxOffset + dwCount + i];
					}
					for (DWORD i = bHi; i > 0; i--)
					{
						liDataRunOffset.QuadPart = liDataRunOffset.QuadPart << 8;
						liDataRunOffset.QuadPart = liDataRunOffset.QuadPart | bBuffer[wAttributeOffset + wIndxOffset + dwCount + bLo + i];
					}
					// 注意加上上一个Data Run的逻辑簇号
					liDataRunOffset.QuadPart = liDataRunOffset.QuadPart + dwClusterOffet;
					dwClusterOffet = dwClusterOffet + liDataRunOffset.LowPart;

					// 显示逻辑簇号和大小
					liContenOffset.QuadPart = liDataRunOffset.QuadPart*wSizeOfSector*bSizeOfCluster;
					printf("File Content Offset:0x%llx\nFile Content Size:0x%llx\n", liContenOffset.QuadPart, (liDataRunSize.QuadPart*wSizeOfSector*bSizeOfCluster));

					// 计算下一个Data Run List偏移
					dwCount = dwCount + bLo + bHi + 1;
				}
			}
		}
		else if (0xFFFFFFFF == dwAttribute)
		{
			break;
		}

		// 获取当前属性的大小
		dwSizeOfAttribute = MAKELONG(MAKEWORD(bBuffer[wAttributeOffset + 4], bBuffer[wAttributeOffset + 5]),
			MAKEWORD(bBuffer[wAttributeOffset + 6], bBuffer[wAttributeOffset + 7]));
		// 计算下一属性的偏移
		wAttributeOffset = wAttributeOffset + dwSizeOfAttribute;
	}

	return TRUE;
}