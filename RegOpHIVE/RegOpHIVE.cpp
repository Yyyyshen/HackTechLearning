// RegOpHIVE.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

/**
 * 通过HIVE文件格式操作注册表
 */
#include "HiveAnalysis.h"

void test_hive()
{
	BOOL bRet = FALSE;
	//	bRet = AnalysisHiveFile("C:\\Users\\DemonGan\\Desktop\\SOFTWARE");
	bRet = AnalysisHiveFile("C:\\Users\\Yshen\\Desktop\\SECURITY");
	if (bRet)
	{
		printf("HIVE FILE ANALYSIS SUCCESS.\n");
	}
	else
	{
		printf("HIVE FILE ANALYSIS ERROR.\n");
	}
	system("pause");
}

int main()
{
	test_hive();
	return 0;
}

