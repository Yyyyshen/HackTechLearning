#ifndef _WININET_FTP_H_
#define _WININET_FTP_H_


#include <Windows.h>
#include <WinInet.h>
#pragma comment(lib, "WinInet.lib")


/*
	FTP的URL格式，如下所示：

		FTP://账号:密码@主机/子目录或文件     // 例如: ftp://admin:123456@192.168.0.1/mycode/520.zip

	注意:不要使用中文
*/

/*
	在前面打开会话和建立连接和HTTP以及HTTPS一样;
	之后的FTP文件操作, 即使用FTPOpenFile获取文件句柄之后, 对文件的操作就如同本地操作一样了.
*/



// 将数据存储为文件
// 输入：数据原文件路径、将要保存的数据内容、将要保存的数据内容长度
BOOL Ftp_SaveToFile(const char* pszFileName, BYTE* pData, DWORD dwDataSize);

// URL分解
// 输入：URL
// 输出：szScheme、szHostName、szUserName、szPassword、szUrlPath、szExtraInfo
BOOL Ftp_UrlCrack(const char* pszUrl, char* pszScheme, char* pszHostName, char* pszUserName, char* pszPassword, char* pszUrlPath, char* pszExtraInfo, DWORD dwBufferSize);

// 数据下载
// 输入：下载数据的URL路径
// 输出：下载数据内容、下载数据内容长度
BOOL FTPDownload(const char* pszDownloadUrl, BYTE** ppDownloadData, DWORD* pdwDownloadDataSize);

// 数据上传
// 输入：上传数据的URL路径、上传数据内容、上传数据内容长度
BOOL FTPUpload(const char* pszUploadUrl, BYTE* pUploadData, DWORD dwUploadDataSize);


#endif