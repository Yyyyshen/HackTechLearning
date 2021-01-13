#include "HiveAnalysis.h"
#include <stdio.h>

void ShowError(const char *pszText)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error[%d]\n", pszText, ::GetLastError());
#ifdef _DEBUG
	::MessageBox(NULL, szErr, "ERROR", MB_OK | MB_ICONERROR);
#endif
}


BOOL AnalysisHiveFile(const char *pszHiveFileName)
{
	BOOL bRet = FALSE;
	HANDLE hFile = NULL, hFileMap = NULL;
	LPVOID pMemory = NULL;

	do
	{
		// 内存映射文件
		hFile = ::CreateFile(pszHiveFileName, GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			ShowError("CreateFile");
			break;
		}
		hFileMap = ::CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
		if (NULL == hFileMap)
		{
			ShowError("CreateFileMapping");
			break;
		}
		pMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		if (NULL == pMemory)
		{
			ShowError("MapViewOfFile");
			break;
		}

		// 分析HIVE文件头
		bRet = AnalysisHiveHeader((PUCHAR)pMemory);

	} while (FALSE);

	// 释放
	if (pMemory)
	{
		UnmapViewOfFile(pMemory);
	}
	if (hFileMap)
	{
		::CloseHandle(hFileMap);
	}
	if (hFile)
	{
		::CloseHandle(hFile);
	}

	return bRet;
}


// 分析HIVE文件头
BOOL AnalysisHiveHeader(PUCHAR pMemory)
{
	BOOL bRet = FALSE;
	DWORD dwMajor = 0, dwMinor = 0;
	DWORD dwRootCellOffset = 0;
	DWORD dwLength = 0;
	WCHAR *pwszHiveName = NULL;
	PUCHAR pHBIN = NULL;

	// 获取HIVE文件主版本号
	dwMajor = *(DWORD *)(pMemory + 0x14);
	// 获取HIVE文件次版本号
	dwMinor = *(DWORD *)(pMemory + 0x18);
	// 获取RootCellOffset
	dwRootCellOffset = *(DWORD *)(pMemory + 0x24);
	// 获取HIVE文件总长度
	dwLength = *(DWORD *)(pMemory + 0x28);
	// HIVE文件名称
	pwszHiveName = (WCHAR *)(pMemory + 0x30);

	// 显示
	printf("-----------------------------------------------------------------------\n");
	printf("dwMajor=%d, dwMinor=%d\n", dwMajor, dwMinor);
	printf("RootCellOffset=0x%X, dwLength=0x%X\n", dwRootCellOffset, dwLength);
	printf("szHiveName=%S\n", pwszHiveName);
	printf("-----------------------------------------------------------------------\n\n");

	// 分析hbin
	pHBIN = pMemory + 0x1000;
	// 分析NK
	bRet = HiveNK(pHBIN, (pHBIN + dwRootCellOffset));

	return bRet;
}


// 分析NK
BOOL HiveNK(PUCHAR pHBIN, PUCHAR pNode)
{
	char *pszNodeName = NULL;
	USHORT usSignature = 0;
	DWORD dwSubNodeCount = 0;
	DWORD dwSubNodeOffset = 0;
	DWORD dwValueCount = 0;
	DWORD dwValueOffset = 0;
	DWORD dwNodeNameLength = 0;
	DWORD dwValueOffsetList = NULL;
	DWORD i = 0;

	// 获取 签名
	usSignature = *(USHORT *)(pNode + 0x4);
	if (0x6B6E != usSignature)  // nk
	{
		return FALSE;
	}
	// 获取 子键数量
	dwSubNodeCount = *(DWORD *)(pNode + 0x18);
	// 获取 子键索引
	dwSubNodeOffset = *(DWORD *)(pNode + 0x20);
	// 获取 键值数量
	dwValueCount = *(DWORD *)(pNode + 0x28);
	// 获取 键值索引
	dwValueOffset = *(DWORD *)(pNode + 0x2C);
	// 获取 键名长度
	dwNodeNameLength = *(DWORD *)(pNode + 0x4C);
	// 获取 键名
	pszNodeName = (char *)(pNode + 0x50);

	// 显示
	for (i = 0; i < dwNodeNameLength; i++)
	{
		printf("%c", pszNodeName[i]);
	}
	printf("\n");

	// 遍历键值
	for (i = 0; i < dwValueCount; i++)
	{
		// 分析VK 
		DWORD dwOffset = *(DWORD *)(pHBIN + dwValueOffset + 4 * (1 + i));
		HiveVK(pHBIN, (pHBIN + dwOffset));
	}

	// 遍历子键
	for (i = 0; i < dwSubNodeCount; i++)
	{
		// 分析LIST
		HiveList(pHBIN, (pHBIN + dwSubNodeOffset));
	}

	return TRUE;
}


// 分析VK
BOOL HiveVK(PUCHAR pHBIN, PUCHAR pValue)
{
	char *pszValueName = NULL;
	USHORT usSignature = 0;
	USHORT usValueNameLength = 0;
	DWORD dwValueDataLength = 0;
	DWORD dwValueData = 0;
	DWORD dwValueType = 0;
	DWORD i = 0;

	// 获取 签名
	usSignature = *(USHORT *)(pValue + 0x4);
	if (0x6B76 != usSignature) //vk
	{
		return FALSE;
	}
	// 获取 键值名称长度
	usValueNameLength = *(USHORT *)(pValue + 0x6);
	// 获取 键值数据长度
	dwValueDataLength = *(DWORD *)(pValue + 0x8);
	// 获取 键值数据
	dwValueData = *(DWORD *)(pValue + 0xC);
	// 获取 键值数据类型
	dwValueType = *(DWORD *)(pValue + 0x10);
	// 获取 键值名称
	pszValueName = (char *)(pValue + 0x18);

	// 显示
	// 键值数据名称
	for (i = 0; i < usValueNameLength; i++)
	{
		printf("%c", pszValueName[i]);
	}
	// 键值数据类型
	printf(", %d\n", dwValueType);

	/*
	printf(", %d, ", dwValueType);
	// 键值数据
	// 判断是否是驻留数据
	if (0x80000000 < dwValueDataLength)
	{
		// 驻留数据
		dwValueDataLength = dwValueDataLength - 0x80000000;
		switch (dwValueDataLength)
		{
		case 1:
		{
			dwValueData = dwValueData & 0x000000FF;
			break;
		}
		case 2:
		{
			dwValueData = dwValueData & 0x0000FFFF;
			break;
		}
		case 3:
		{
			dwValueData = dwValueData & 0x00FFFFFF;
			break;
		}
		case 4:
		{
			dwValueData = dwValueData & 0xFFFFFFFF;
			break;
		}
		default:
			break;
		}
		printf("%X\n", dwValueData);
	}
	else
	{
		// 非驻留数据
		for (i = 0; i < dwValueDataLength; i++)
		{
			printf("0x%X ", *(UCHAR *)(pHBIN + dwValueData + 0x4 + i));
		}
		printf("\n");
	}
	**/
	return TRUE;
}


// 分析LIST
BOOL HiveList(PUCHAR pHBIN, PUCHAR pList)
{
	USHORT usSignature = 0;
	USHORT usCount = 0;
	DWORD dwOffset = 0;
	USHORT i = 0;

	// 获取 签名
	usSignature = *(USHORT *)(pList + 0x4);
	// 获取 数量
	usCount = *(USHORT *)(pList + 0x6);

	// 判断
	if (0x666C == usSignature ||      // lf      
		0x686C == usSignature)        // lh
	{
		// 获取偏移量
		for (i = 0; i < usCount; i++)
		{
			dwOffset = *(DWORD *)(pList + 0x8 + 8 * i);
			HiveNK(pHBIN, (pHBIN + dwOffset));
		}
	}
	else if (0x696C == usSignature || // li
		     0x6972 == usSignature)   // ri
	{
		// 获取偏移量
		for (i = 0; i < usCount; i++)
		{
			dwOffset = *(DWORD *)(pList + 0x8 + 4 * i);
			HiveList(pHBIN, (pHBIN + dwOffset));
		}
	}
	else if (0x6264 == usSignature)   // db
	{
		// 获取偏移量
		dwOffset = *(DWORD *)(pList + 0x8);
		HiveVK(pHBIN, (pHBIN + dwOffset));
	}

	return TRUE;
}