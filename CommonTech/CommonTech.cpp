// CommonTech.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

/**
 * 恶意程序常见功能实现
 */

 /**
  * 进程遍历
  * 可以通过进程信息分析用户环境
  * 方式：
  * 进程快照、Win32API函数ZwQuerySystemInformation或者EnumProcesses函数、通过PowerShell、WMI获取
  *
  * 这里选取最常见的进程快照方式
  */
#include "EnumInfo.h"
void test_processes()
{
	// 遍历进程
	if (FALSE == EnumProcess())
	{
		printf("Enum Process Error!\n");
	}

	system("pause");
	system("cls");

	// 遍历线程
	if (FALSE == EnumThread())
	{
		printf("Enum Thread Error!\n");
	}

	system("pause");
	system("cls");

	// 遍历指定进程模块
	if (FALSE == EnumProcessModule(15452))
	{
		printf("Enum Process Module Error!\n");
	}

	system("pause");
}

/**
 * 遍历文件
 */
#include "FileSearch.h"
void test_files()
{
	SearchFile("C:\\workspaceKernel\\HackTechLearning\\Debug");

	system("pause");
}

/**
 * 桌面截屏
 * 这个技术在市面上，不止截屏，甚至能做到完全的远程控制
 */
#include "ScreenCapture.h"
void test_screen()
{
	if (FALSE == ScreenCapture())
	{
		printf("Screen Cpature Error.\n");
		return;
	}

	printf("Screen Cpature OK.\n");
	system("pause");
}

int main()
{
	//进程遍历
	//test_processes();

	//文件遍历
	//test_files();

	//桌面截屏
	test_screen();

	return 0;
}

