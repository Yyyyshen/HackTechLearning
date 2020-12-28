// VirusInit.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <afxcmn.h>
#include <tchar.h>
//#include "Skins\\SkinPPWTL.h"
//#pragma comment(lib, "Skins\\SkinPPWTL.lib")
#include "resource.h"
/**
 * 恶意程序常见初始化操作技术
 *
 * 互斥量运行单一实例
 * DLL延迟加载
 * 资源释放
 *
 */

 //互斥量使用
 //为保证恶意程序不会因多次运行而容易被发现，最好运行单一实例
BOOL IsAreadyRun()
{
	//互斥量句柄
	HANDLE hMutex = NULL;
	//创建互斥量，注意名称
	hMutex = ::CreateMutex(NULL, FALSE, "TEST");
	//返回值为NULL,则失败，若返回一个句柄，可能为新创建，也可能是已经存在，需要通过GetLastError进行确认
	if (hMutex)
	{
		if (ERROR_ALREADY_EXISTS == ::GetLastError())
		{
			return TRUE;
		}
	}
	return FALSE;
}

//DLL延迟加载
//使用第三方dll可以在调用接口时才加载
//书中这里讲的是直接在vs里配置延迟加载
//工具：PEView可以查看导入表信息

//资源释放
//程序中需要额外加载的dll/图片/文本等文件，都可以作为资源插入到程序中
//最终只产生一个exe文件，程序更简洁，植入就更容易
//首先，需要将资源插入，也是使用vs的功能，导入自定义rc资源，也有用代码插入的方式

//释放函数
BOOL FreeMyResource(UINT uiResName, const char* lpszResType, const char* lpszSaveFileName)
{
	//获取指定模块资源句柄
	HRSRC hRsrc = ::FindResource(NULL, MAKEINTRESOURCE(uiResName), lpszResType);
	if (NULL == hRsrc)
	{
		return FALSE;
	}
	//获取大小
	DWORD dwSize = ::SizeofResource(NULL, hRsrc);
	//加载资源到内存中
	HGLOBAL hGlobal = ::LoadResource(NULL, hRsrc);
	if (NULL == hGlobal)
	{
		return FALSE;
	}
	//锁定资源 如果被装载的资源被所住了，返回值是资源第一个字节的指针；否则为NULL。
	LPVOID lpVoid = ::LockResource(hGlobal);
	if (NULL == lpVoid)
	{
		return FALSE;
	}
	//保存到文件
	FILE* fp = NULL;
	fopen_s(&fp, lpszSaveFileName, "wb+");
	if (NULL == fp)
	{
		return FALSE;
	}
	fwrite(lpVoid, sizeof(char), dwSize, fp);
	fclose(fp);
	return TRUE;

	//另一种写文件方式
	////创建文件   实际使用时修改XXX.sys
	//HANDLE hFile = CreateFile(TEXT("XXX.sys"), GENERIC_WRITE,
	//	0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


	//if (hFile == INVALID_HANDLE_VALUE)
	//	return FALSE;


	//DWORD dw;
	////将资数据写入文件
	//if (!WriteFile(hFile, lpVoid, dwSize, &dw, NULL)) {
	//	CloseHandle(hFile);
	//	return FALSE;
	//}
	//CloseHandle(hFile);
	//return TRUE;
}

int main()
{
	if (IsAreadyRun())
	{
		std::cout << "Already running!" << std::endl;
		return 0;
	}
	//skinppExitSkin();//设置延迟加载后，pe头的导出表中就不会依赖原本的dll
	FreeMyResource(IDR_MYRES3, "MYRES", "test-res-from-exe");
	std::cout << "Hello World!\n";
	return 0;
}

