#ifndef _HIVE_ANALYSIS_H_
#define _HIVE_ANALYSIS_H_


#include <Windows.h>


BOOL AnalysisHiveFile(const char *pszHiveFileName);

// 分析HIVE文件头
BOOL AnalysisHiveHeader(PUCHAR pMemory);
// 分析NK
BOOL HiveNK(PUCHAR pHBIN, PUCHAR pNode);
// 分析VK
BOOL HiveVK(PUCHAR pHBIN, PUCHAR pValue);
// 分析LIST
BOOL HiveList(PUCHAR pHBIN, PUCHAR pList);

#endif