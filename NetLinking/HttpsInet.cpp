#include "HttpsInet.h"


void Https_ShowError(const char* pszText)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error[%d]\n", pszText, ::GetLastError());
#ifdef _DEBUG
	::MessageBox(NULL, szErr, NULL, MB_OK);
#endif
}


// URL分解
// 输入：URL
// 输出：szScheme、szHostName、szUserName、szPassword、szUrlPath、szExtraInfo
BOOL Https_UrlCrack(char* pszUrl, char* pszScheme, char* pszHostName, char* pszUserName, char* pszPassword, char* pszUrlPath, char* pszExtraInfo, DWORD dwBufferSize)
{
	BOOL bRet = FALSE;
	URL_COMPONENTS uc = { 0 };

	::RtlZeroMemory(&uc, sizeof(uc));
	::RtlZeroMemory(pszScheme, dwBufferSize);
	::RtlZeroMemory(pszHostName, dwBufferSize);
	::RtlZeroMemory(pszUserName, dwBufferSize);
	::RtlZeroMemory(pszPassword, dwBufferSize);
	::RtlZeroMemory(pszUrlPath, dwBufferSize);
	::RtlZeroMemory(pszExtraInfo, dwBufferSize);

	uc.dwStructSize = sizeof(uc);
	uc.dwSchemeLength = dwBufferSize - 1;
	uc.dwHostNameLength = dwBufferSize - 1;
	uc.dwUserNameLength = dwBufferSize - 1;
	uc.dwPasswordLength = dwBufferSize - 1;
	uc.dwUrlPathLength = dwBufferSize - 1;
	uc.dwExtraInfoLength = dwBufferSize - 1;
	uc.lpszScheme = pszScheme;
	uc.lpszHostName = pszHostName;
	uc.lpszUserName = pszUserName;
	uc.lpszPassword = pszPassword;
	uc.lpszUrlPath = pszUrlPath;
	uc.lpszExtraInfo = pszExtraInfo;

	// 分解
	bRet = ::InternetCrackUrl(pszUrl, 0, 0, &uc);
	if (FALSE == bRet)
	{
		Https_ShowError("InternetCrackUrl");
		return bRet;
	}

	return bRet;
}


// 从响应信息头信息中获取数据内容长度大小
// 输入：响应信息头信息内容
// 输出：数据内容长度大小
BOOL Https_GetContentLength(char* pResponseHeader, DWORD* pdwContentLength)
{
	// 从 中字段 "Content-Length: "(注意有个空格) 获取数据长度
	int i = 0;
	char szContentLength[MAX_PATH] = { 0 };
	DWORD dwContentLength = 0;
	char szSubStr[] = "Content-Length: ";
	::RtlZeroMemory(szContentLength, MAX_PATH);

	char* p = ::strstr(pResponseHeader, szSubStr);
	if (NULL == p)
	{
		return FALSE;
	}

	p = p + ::lstrlen(szSubStr);

	while (('0' <= *p) &&
		('9' >= *p))
	{
		szContentLength[i] = *p;
		p++;
		i++;
	}

	dwContentLength = ::atoi(szContentLength);

	*pdwContentLength = dwContentLength;

	return TRUE;
}


// 数据下载
// 输入：下载数据的URL路径
// 输出：下载数据内容、下载数据内容长度
BOOL Https_Download(char* pszDownloadUrl, BYTE** ppDownloadData, DWORD* pdwDownloadDataSize)
{
	// INTERNET_SCHEME_HTTPS、INTERNET_SCHEME_HTTP、INTERNET_SCHEME_FTP等
	char szScheme[MAX_PATH] = { 0 };
	char szHostName[MAX_PATH] = { 0 };
	char szUserName[MAX_PATH] = { 0 };
	char szPassword[MAX_PATH] = { 0 };
	char szUrlPath[MAX_PATH] = { 0 };
	char szExtraInfo[MAX_PATH] = { 0 };
	::RtlZeroMemory(szScheme, MAX_PATH);
	::RtlZeroMemory(szHostName, MAX_PATH);
	::RtlZeroMemory(szUserName, MAX_PATH);
	::RtlZeroMemory(szPassword, MAX_PATH);
	::RtlZeroMemory(szUrlPath, MAX_PATH);
	::RtlZeroMemory(szExtraInfo, MAX_PATH);
	// 分解URL
	if (FALSE == Https_UrlCrack(pszDownloadUrl, szScheme, szHostName, szUserName, szPassword, szUrlPath, szExtraInfo, MAX_PATH))
	{
		return FALSE;
	}

	// 数据下载
	HINTERNET hInternet = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	DWORD dwOpenRequestFlags = 0;
	BOOL bRet = FALSE;
	unsigned char* pResponseHeaderIInfo = NULL;
	DWORD dwResponseHeaderIInfoSize = 2048;
	BYTE* pBuf = NULL;
	DWORD dwBufSize = 64 * 1024;
	BYTE* pDownloadData = NULL;
	DWORD dwDownloadDataSize = 0;
	DWORD dwRet = 0;
	DWORD dwOffset = 0;

	do
	{
		// 建立会话
		hInternet = ::InternetOpen("WinInetGet/0.1", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (NULL == hInternet)
		{
			Https_ShowError("InternetOpen");
			break;
		}
		// 建立连接(与HTTP的区别 -- 端口)
		hConnect = ::InternetConnect(hInternet, szHostName, INTERNET_DEFAULT_HTTPS_PORT, szUserName, szPassword, INTERNET_SERVICE_HTTP, 0, 0);
		if (NULL == hConnect)
		{
			Https_ShowError("InternetConnect");
			break;
		}
		// 打开并发送HTTPS请求(与HTTP的区别--标志)
		dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
			INTERNET_FLAG_KEEP_CONNECTION |
			INTERNET_FLAG_NO_AUTH |
			INTERNET_FLAG_NO_COOKIES |
			INTERNET_FLAG_NO_UI |
			INTERNET_FLAG_RELOAD |
			// HTTPS SETTING
			INTERNET_FLAG_SECURE |
			INTERNET_FLAG_IGNORE_CERT_CN_INVALID;
		if (0 < ::lstrlen(szExtraInfo))
		{
			// 注意此处的连接
			::lstrcat(szUrlPath, szExtraInfo);
		}
		hRequest = ::HttpOpenRequest(hConnect, "GET", szUrlPath, NULL, NULL, NULL, dwOpenRequestFlags, 0);
		if (NULL == hRequest)
		{
			Https_ShowError("HttpOpenRequest");
			break;
		}
		// 发送请求(与HTTP的区别--对无效的证书颁发机构的处理)
		bRet = ::HttpSendRequest(hRequest, NULL, 0, NULL, 0);
		if (FALSE == bRet)
		{
			if (ERROR_INTERNET_INVALID_CA == ::GetLastError())
			{
				DWORD dwFlags = 0;
				DWORD dwBufferSize = sizeof(dwFlags);
				// 获取INTERNET_OPTION_SECURITY_FLAGS标志 
				bRet = ::InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, &dwBufferSize);
				if (bRet)
				{
					// 设置INTERNET_OPTION_SECURITY_FLAGS标志 
					// 忽略未知的证书颁发机构
					dwFlags = dwFlags | SECURITY_FLAG_IGNORE_UNKNOWN_CA;
					bRet = ::InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
					if (bRet)
					{
						// // 再次发送请求
						bRet = ::HttpSendRequest(hRequest, NULL, 0, NULL, 0);
						if (FALSE == bRet)
						{
							Https_ShowError("HttpSendRequest");
							break;
						}
					}
					else
					{
						Https_ShowError("InternetSetOption");
						break;
					}
				}
				else
				{
					Https_ShowError("InternetQueryOption");
					break;
				}
			}
			else
			{
				Https_ShowError("HttpSendRequest");
				break;
			}
		}
		// 接收响应的报文信息头(Get Response Header)
		pResponseHeaderIInfo = new unsigned char[dwResponseHeaderIInfoSize];
		if (NULL == pResponseHeaderIInfo)
		{
			break;
		}
		::RtlZeroMemory(pResponseHeaderIInfo, dwResponseHeaderIInfoSize);
		bRet = ::HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, pResponseHeaderIInfo, &dwResponseHeaderIInfoSize, NULL);
		if (FALSE == bRet)
		{
			Https_ShowError("HttpQueryInfo");
			break;
		}
#ifdef _DEBUG
		printf("[HTTPS_Download_ResponseHeaderIInfo]\n\n%s\n\n", pResponseHeaderIInfo);
#endif
		// 从 中字段 "Content-Length: "(注意有个空格) 获取数据长度
		bRet = Https_GetContentLength((char*)pResponseHeaderIInfo, &dwDownloadDataSize);
		if (FALSE == bRet)
		{
			break;
		}
		// 接收报文主体内容(Get Response Body)
		pBuf = new BYTE[dwBufSize];
		if (NULL == pBuf)
		{
			break;
		}
		pDownloadData = new BYTE[dwDownloadDataSize];
		if (NULL == pDownloadData)
		{
			break;
		}
		::RtlZeroMemory(pDownloadData, dwDownloadDataSize);
		do
		{
			::RtlZeroMemory(pBuf, dwBufSize);
			bRet = ::InternetReadFile(hRequest, pBuf, dwBufSize, &dwRet);
			if (FALSE == bRet)
			{
				Https_ShowError("InternetReadFile");
				break;
			}

			::RtlCopyMemory((pDownloadData + dwOffset), pBuf, dwRet);
			dwOffset = dwOffset + dwRet;

		} while (dwDownloadDataSize > dwOffset);

		// 返回数据
		*ppDownloadData = pDownloadData;
		*pdwDownloadDataSize = dwDownloadDataSize;

	} while (FALSE);

	// 关闭 释放
	if (NULL != pBuf)
	{
		delete[]pBuf;
		pBuf = NULL;
	}
	if (NULL != pResponseHeaderIInfo)
	{
		delete[]pResponseHeaderIInfo;
		pResponseHeaderIInfo = NULL;
	}
	if (NULL != hRequest)
	{
		::InternetCloseHandle(hRequest);
		hRequest = NULL;
	}
	if (NULL != hConnect)
	{
		::InternetCloseHandle(hConnect);
		hConnect = NULL;
	}
	if (NULL != hInternet)
	{
		::InternetCloseHandle(hInternet);
		hInternet = NULL;
	}

	return bRet;
}


// 数据上传
// 输入：上传数据的URL路径、上传数据内容、上传数据内容长度
BOOL Https_Upload(char* pszUploadUrl, BYTE* pUploadData, DWORD dwUploadDataSize)
{
	// INTERNET_SCHEME_HTTPS、INTERNET_SCHEME_HTTP、INTERNET_SCHEME_FTP等
	char szScheme[MAX_PATH] = { 0 };
	char szHostName[MAX_PATH] = { 0 };
	char szUserName[MAX_PATH] = { 0 };
	char szPassword[MAX_PATH] = { 0 };
	char szUrlPath[MAX_PATH] = { 0 };
	char szExtraInfo[MAX_PATH] = { 0 };
	::RtlZeroMemory(szScheme, MAX_PATH);
	::RtlZeroMemory(szHostName, MAX_PATH);
	::RtlZeroMemory(szUserName, MAX_PATH);
	::RtlZeroMemory(szPassword, MAX_PATH);
	::RtlZeroMemory(szUrlPath, MAX_PATH);
	::RtlZeroMemory(szExtraInfo, MAX_PATH);
	// 分解URL
	if (FALSE == Https_UrlCrack(pszUploadUrl, szScheme, szHostName, szUserName, szPassword, szUrlPath, szExtraInfo, MAX_PATH))
	{
		return FALSE;
	}

	// 数据上传
	HINTERNET hInternet = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	DWORD dwOpenRequestFlags = 0;
	BOOL bRet = FALSE;
	DWORD dwRet = 0;
	unsigned char* pResponseHeaderIInfo = NULL;
	DWORD dwResponseHeaderIInfoSize = 2048;
	BYTE* pBuf = NULL;
	DWORD dwBufSize = 64 * 1024;
	BYTE* pResponseBodyData = NULL;
	DWORD dwResponseBodyDataSize = 0;
	DWORD dwOffset = 0;
	DWORD dwPostDataSize = dwUploadDataSize;
	INTERNET_BUFFERS internetBuffers = { 0 };

	do
	{
		// 建立会话
		hInternet = ::InternetOpen("WinInetPost/0.1", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (NULL == hInternet)
		{
			Https_ShowError("InternetOpen");
			break;
		}
		// 建立连接
		hConnect = ::InternetConnect(hInternet, szHostName, INTERNET_DEFAULT_HTTPS_PORT, szUserName, szPassword, INTERNET_SERVICE_HTTP, 0, 0);
		if (NULL == hConnect)
		{
			Https_ShowError("InternetConnect");
			break;
		}
		// 打开并发送HTTP请求
		dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
			INTERNET_FLAG_KEEP_CONNECTION |
			INTERNET_FLAG_NO_AUTH |
			INTERNET_FLAG_NO_COOKIES |
			INTERNET_FLAG_NO_UI |
			INTERNET_FLAG_RELOAD |
			// HTTPS SETTING
			INTERNET_FLAG_SECURE |
			INTERNET_FLAG_IGNORE_CERT_CN_INVALID;
		if (0 < ::lstrlen(szExtraInfo))
		{
			// 注意此处的连接
			::lstrcat(szUrlPath, szExtraInfo);
		}
		hRequest = ::HttpOpenRequest(hConnect, "POST", szUrlPath, NULL, NULL, NULL, dwOpenRequestFlags, 0);
		if (NULL == hRequest)
		{
			Https_ShowError("HttpOpenRequest");
			break;
		}
		// 发送请求, 告诉服务器传输数据的总大小
		::RtlZeroMemory(&internetBuffers, sizeof(internetBuffers));
		internetBuffers.dwStructSize = sizeof(internetBuffers);
		internetBuffers.dwBufferTotal = dwPostDataSize;
		bRet = ::HttpSendRequestEx(hRequest, &internetBuffers, NULL, 0, 0);
		if (FALSE == bRet)
		{
			if (ERROR_INTERNET_INVALID_CA == ::GetLastError())
			{
				DWORD dwFlags = 0;
				DWORD dwBufferSize = sizeof(dwFlags);
				// 获取INTERNET_OPTION_SECURITY_FLAGS标志 
				bRet = ::InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, &dwBufferSize);
				if (bRet)
				{
					// 设置INTERNET_OPTION_SECURITY_FLAGS标志 
					// 忽略未知的证书颁发机构
					dwFlags = dwFlags | SECURITY_FLAG_IGNORE_UNKNOWN_CA;
					bRet = ::InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
					if (bRet)
					{
						// 再次发送请求
						bRet = ::HttpSendRequestEx(hRequest, &internetBuffers, NULL, 0, 0);
						if (FALSE == bRet)
						{
							Https_ShowError("HttpSendRequestEx");
							break;
						}
					}
					else
					{
						Https_ShowError("InternetSetOption");
						break;
					}
				}
				else
				{
					Https_ShowError("InternetQueryOption");
					break;
				}
			}
			else
			{
				Https_ShowError("HttpSendRequestEx");
				break;
			}
		}
		// 发送数据
		bRet = ::InternetWriteFile(hRequest, pUploadData, dwUploadDataSize, &dwRet);
		if (FALSE == bRet)
		{
			Https_ShowError("InternetWriteFile2");
			break;
		}
		// 发送完毕, 结束请求
		bRet = ::HttpEndRequest(hRequest, NULL, 0, 0);
		if (FALSE == bRet)
		{
			Https_ShowError("HttpEndRequest");
			break;
		}

		// 接收来自服务器响应的数据
		// 接收响应的报文信息头(Get Response Header)
		pResponseHeaderIInfo = new unsigned char[dwResponseHeaderIInfoSize];
		if (NULL == pResponseHeaderIInfo)
		{
			break;
		}
		::RtlZeroMemory(pResponseHeaderIInfo, dwResponseHeaderIInfoSize);
		bRet = ::HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, pResponseHeaderIInfo, &dwResponseHeaderIInfoSize, NULL);
		if (FALSE == bRet)
		{
			Https_ShowError("HttpQueryInfo");
			break;
		}
#ifdef _DEBUG
		printf("[HTTPS_Upload_ResponseHeaderIInfo]\n\n%s\n\n", pResponseHeaderIInfo);
#endif
		// 从 中字段 "Content-Length: "(注意有个空格) 获取数据长度
		bRet = Https_GetContentLength((char*)pResponseHeaderIInfo, &dwResponseBodyDataSize);
		if (FALSE == bRet)
		{
			break;
		}
		// 接收报文主体内容(Get Response Body)
		pBuf = new BYTE[dwBufSize];
		if (NULL == pBuf)
		{
			break;
		}
		pResponseBodyData = new BYTE[dwResponseBodyDataSize];
		if (NULL == pResponseBodyData)
		{
			break;
		}
		::RtlZeroMemory(pResponseBodyData, dwResponseBodyDataSize);
		do
		{
			::RtlZeroMemory(pBuf, dwBufSize);
			bRet = ::InternetReadFile(hRequest, pBuf, dwBufSize, &dwRet);
			if (FALSE == bRet)
			{
				Https_ShowError("InternetReadFile");
				break;
			}

			::RtlCopyMemory((pResponseBodyData + dwOffset), pBuf, dwRet);
			dwOffset = dwOffset + dwRet;

		} while (dwResponseBodyDataSize > dwOffset);

	} while (FALSE);

	// 关闭 释放
	if (NULL != pResponseBodyData)
	{
		delete[]pResponseBodyData;
		pResponseBodyData = NULL;
	}
	if (NULL != pBuf)
	{
		delete[]pBuf;
		pBuf = NULL;
	}
	if (NULL != pResponseHeaderIInfo)
	{
		delete[]pResponseHeaderIInfo;
		pResponseHeaderIInfo = NULL;
	}

	if (NULL != hRequest)
	{
		::InternetCloseHandle(hRequest);
		hRequest = NULL;
	}
	if (NULL != hConnect)
	{
		::InternetCloseHandle(hConnect);
		hConnect = NULL;
	}
	if (NULL != hInternet)
	{
		::InternetCloseHandle(hInternet);
		hInternet = NULL;
	}

	return bRet;
}


// 将数据存储为文件
// 输入：数据原文件路径、将要保存的数据内容、将要保存的数据内容长度
BOOL Https_SaveToFile(const char* pszFileName, BYTE* pData, DWORD dwDataSize)
{
	HANDLE hFile = ::CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		Https_ShowError("CreateFile");
		return FALSE;
	}

	DWORD dwRet = 0;
	::WriteFile(hFile, pData, dwDataSize, &dwRet, NULL);

	::CloseHandle(hFile);

	return TRUE;
}



