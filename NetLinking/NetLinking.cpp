// NetLinking.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

/**
 * 传输技术
 * 之前一直用的boost网络库，正好借这个看下原生接口
 */

 /**
  * TCP
  */
#include "TcpClient.h"
void tcp_client()
{
	// 连接到服务器
	if (FALSE == Connection("127.0.0.1", 12345))
	{
		printf("Connection Error.\n");
		return;
	}
	printf("Connection OK.\n");

	// 发送信息
	char szSendBuf[MAX_PATH] = { 0 };
	while (TRUE)
	{
		gets_s(szSendBuf);
		// 发送数据
		SendMsg(szSendBuf);
	}
}

/**
 * UDP
 * 不需要建立连接，两端bind端口后直接向目标发送数据
 */
#include "UDPTest.h"

void udp_test()
{
	char szIp[MAX_PATH] = { 0 };
	int iPort = 0;
	// 输入程序UDP绑定的IP和端口
	printf("Input IP and Port:\n");
	scanf("%s%d", szIp, &iPort);
	getchar();

	// 绑定地址
	if (FALSE == Bind(szIp, iPort))
	{
		printf("Bind Error.\n");
	}
	printf("Bind OK.\n");

	// 输入发送数据目的主机的IP和端口
	printf("Input Dest IP and Dest Port:\n");
	scanf("%s%d", szIp, &iPort);
	getchar();
	// 发送数据
	char szBuf[MAX_PATH] = { 0 };
	while (TRUE)
	{
		gets_s(szBuf);
		SendMsg(szBuf, szIp, iPort);
	}
}

/**
 * FTP上传下载
 * 微软提供了WinInet网络库，简化了网络协议的编程
 */
#include "WinInet_Ftp.h"
void ftp_upload()
{
	if (FALSE == FTPUpload("ftp://admin:123456789@192.168.0.1/myftpuploadtest.txt", (BYTE*)"Hello Wolrd", 12))
	{
		printf("FTP Upload Error.\n");
	}

	printf("FTP Upload OK.\n");
	system("pause");
}
void ftp_download()
{
	BYTE* pDownloadData = NULL;
	DWORD dwDownloadDataSize = 0;
	// 下载
	if (FALSE == FTPDownload("ftp://admin:123456789@192.168.0.1/Flower520.zip", &pDownloadData, &dwDownloadDataSize))
	{
		printf("FTP Download Error!\n");
	}
	// 将数据保存为文件
	Ftp_SaveToFile("myftpdownloadtest.zip", pDownloadData, dwDownloadDataSize);
	// 释放内存
	delete[]pDownloadData;
	pDownloadData = NULL;
	printf("FTP Download OK.\n");
	system("pause");
}

/**
 * 基于WinInet的HTTPS上传下载
 */
#include "HttpsInet.h"
void https_upload()
{
	char szHttpsUploadUrl[] = "https://192.168.28.137/mytest1.asp?file=520.zip";
	char szHttpsUploadFileName[] = "C:\\Users\\DemonGan\\Desktop\\520.zip";
	BYTE* pHttpsUploadData = NULL;
	DWORD dwHttpsUploadDataSize = 0;
	DWORD dwRets = 0;
	// 打开文件
	HANDLE hFiles = ::CreateFile(szHttpsUploadFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (INVALID_HANDLE_VALUE == hFiles)
	{
		return;
	}
	// 获取文件大小
	dwHttpsUploadDataSize = ::GetFileSize(hFiles, NULL);
	// 读取文件数据
	pHttpsUploadData = new BYTE[dwHttpsUploadDataSize];
	::ReadFile(hFiles, pHttpsUploadData, dwHttpsUploadDataSize, &dwRets, NULL);
	dwHttpsUploadDataSize = dwRets;
	// 上传数据
	if (FALSE == Https_Upload(szHttpsUploadUrl, pHttpsUploadData, dwHttpsUploadDataSize))
	{
		return;
	}
	// 释放内存
	delete[]pHttpsUploadData;
	pHttpsUploadData = NULL;
	::CloseHandle(hFiles);

	system("pause");
}
void https_download()
{
	char szHttpsDownloadUrl[] = "https://download.microsoft.com/download/0/2/3/02389126-40A7-46FD-9D83-802454852703/vc_mbcsmfc.exe";
	BYTE* pHttpsDownloadData = NULL;
	DWORD dwHttpsDownloadDataSize = 0;
	// HTTPS下载 
	if (FALSE == Https_Download(szHttpsDownloadUrl, &pHttpsDownloadData, &dwHttpsDownloadDataSize))
	{
		return;
	}
	// 将数据保存成文件
	Https_SaveToFile("https_downloadsavefile.exe", pHttpsDownloadData, dwHttpsDownloadDataSize);
	// 释放内存
	delete[]pHttpsDownloadData;
	pHttpsDownloadData = NULL;

	system("pause");
}

int main()
{
	//测试tcp连接
	//tcp_client();

	//测试udp
	//udp_test();

	return 0;
}

