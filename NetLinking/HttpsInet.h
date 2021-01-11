#ifndef _HTTPS_INET_H_
#define _HTTPS_INET_H_


#include <Windows.h>
#include <WinInet.h>
#pragma comment(lib, "WinInet.lib")
#include <stdio.h>


// URL分解
// 输入：URL、输入和输出缓冲区大小
// 输出：szScheme、szHostName、szUserName、szPassword、szUrlPath、szExtraInfo
BOOL Https_UrlCrack(char *pszUrl, char *pszScheme, char *pszHostName, char *pszUserName, char *pszPassword, char *pszUrlPath, char *pszExtraInfo, DWORD dwBufferSize);


// 从响应信息头信息中获取数据内容长度大小
// 输入：响应信息头信息内容
// 输出：数据内容长度大小
BOOL Https_GetContentLength(char *pResponseHeader, DWORD *pdwContentLength);

// 数据下载
// 输入：下载数据的URL路径
// 输出：下载数据内容、下载数据内容长度
BOOL Https_Download(char *pszDownloadUrl, BYTE **ppDownloadData, DWORD *pdwDownloadDataSize);


// 数据上传
// 输入：上传数据的URL路径、上传数据内容、上传数据内容长度
BOOL Https_Upload(char *pszUploadUrl, BYTE *pUploadData, DWORD dwUploadDataSize);


// 将数据存储为文件
// 输入：数据原文件路径、将要保存的数据内容、将要保存的数据内容长度
BOOL Https_SaveToFile(const char *pszFileName, BYTE *pData, DWORD dwDataSize);


#endif