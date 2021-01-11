#include "WinInet_Ftp.h"


void Ftp_ShowError(const char* lpszText)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error[%d]\n", lpszText, ::GetLastError());
#ifdef _DEBUG
	::MessageBox(NULL, szErr, "ERROR", MB_OK);
#endif
}


// 将数据存储为文件
// 输入：数据原文件路径、将要保存的数据内容、将要保存的数据内容长度
BOOL Ftp_SaveToFile(const char* pszFileName, BYTE* pData, DWORD dwDataSize)
{
	HANDLE hFile = ::CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		Ftp_ShowError("CreateFile");
		return FALSE;
	}

	DWORD dwRet = 0;
	::WriteFile(hFile, pData, dwDataSize, &dwRet, NULL);

	::CloseHandle(hFile);

	return TRUE;
}


// URL分解
// 输入：URL
// 输出：szScheme、szHostName、szUserName、szPassword、szUrlPath、szExtraInfo
BOOL Ftp_UrlCrack(const char* pszUrl, char* pszScheme, char* pszHostName, char* pszUserName, char* pszPassword, char* pszUrlPath, char* pszExtraInfo, DWORD dwBufferSize)
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
		Ftp_ShowError("InternetCrackUrl");
		return bRet;
	}

	return bRet;
}


// 数据下载
// 输入：下载数据的URL路径
// 输出：下载数据内容、下载数据内容长度
BOOL FTPDownload(const char* pszDownloadUrl, BYTE** ppDownloadData, DWORD* pdwDownloadDataSize)
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
	if (FALSE == Ftp_UrlCrack(pszDownloadUrl, szScheme, szHostName, szUserName, szPassword, szUrlPath, szExtraInfo, MAX_PATH))
	{
		return FALSE;
	}
	if (0 < ::lstrlen(szExtraInfo))
	{
		// 注意此处的连接
		::lstrcat(szUrlPath, szExtraInfo);
	}

	HINTERNET hInternet = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hFTPFile = NULL;
	BYTE* pDownloadData = NULL;
	DWORD dwDownloadDataSize = 0;
	DWORD dwBufferSize = 4096;
	BYTE* pBuf = NULL;
	DWORD dwBytesReturn = 0;
	DWORD dwOffset = 0;
	BOOL bRet = FALSE;

	do
	{
		// 建立会话
		hInternet = ::InternetOpen("WinInet Ftp Download V1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (NULL == hInternet)
		{
			Ftp_ShowError("InternetOpen");
			break;
		}
		// 建立连接
		hConnect = ::InternetConnect(hInternet, szHostName, INTERNET_INVALID_PORT_NUMBER, szUserName, szPassword, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
		if (NULL == hConnect)
		{
			Ftp_ShowError("InternetConnect");
			break;
		}
		// 打开FTP文件, 文件操作和本地操作相似
		hFTPFile = ::FtpOpenFile(hConnect, szUrlPath, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, NULL);
		if (NULL == hFTPFile)
		{
			Ftp_ShowError("FtpOpenFile");
			break;;
		}
		// 获取文件大小
		dwDownloadDataSize = ::FtpGetFileSize(hFTPFile, NULL);
		// 申请动态内存
		pDownloadData = new BYTE[dwDownloadDataSize];
		if (NULL == pDownloadData)
		{
			break;
		}
		::RtlZeroMemory(pDownloadData, dwDownloadDataSize);
		pBuf = new BYTE[dwBufferSize];
		if (NULL == pBuf)
		{
			break;
		}
		::RtlZeroMemory(pBuf, dwBufferSize);
		// 接收数据
		do
		{
			bRet = ::InternetReadFile(hFTPFile, pBuf, dwBufferSize, &dwBytesReturn);
			if (FALSE == bRet)
			{
				Ftp_ShowError("InternetReadFile");
				break;
			}
			::RtlCopyMemory((pDownloadData + dwOffset), pBuf, dwBytesReturn);
			dwOffset = dwOffset + dwBytesReturn;

		} while (dwDownloadDataSize > dwOffset);

	} while (FALSE);
	// 返回数据
	if (FALSE == bRet)
	{
		delete[]pDownloadData;
		pDownloadData = NULL;
		dwDownloadDataSize = 0;
	}
	*ppDownloadData = pDownloadData;
	*pdwDownloadDataSize = dwDownloadDataSize;

	// 释放内存并关闭句柄
	if (NULL != pBuf)
	{
		delete[]pBuf;
		pBuf = NULL;
	}
	if (NULL != hFTPFile)
	{
		::InternetCloseHandle(hFTPFile);
	}
	if (NULL != hConnect)
	{
		::InternetCloseHandle(hConnect);
	}
	if (NULL != hInternet)
	{
		::InternetCloseHandle(hInternet);
	}

	return bRet;
}


// 数据上传
// 输入：上传数据的URL路径、上传数据内容、上传数据内容长度
BOOL FTPUpload(const char* pszUploadUrl, BYTE* pUploadData, DWORD dwUploadDataSize)
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
	if (FALSE == Ftp_UrlCrack(pszUploadUrl, szScheme, szHostName, szUserName, szPassword, szUrlPath, szExtraInfo, MAX_PATH))
	{
		return FALSE;
	}
	if (0 < ::lstrlen(szExtraInfo))
	{
		// 注意此处的连接
		::lstrcat(szUrlPath, szExtraInfo);
	}

	HINTERNET hInternet = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hFTPFile = NULL;
	DWORD dwBytesReturn = 0;
	BOOL bRet = FALSE;

	do
	{
		// 建立会话
		hInternet = ::InternetOpen("WinInet Ftp Upload V1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (NULL == hInternet)
		{
			Ftp_ShowError("InternetOpen");
			break;
		}
		// 建立连接
		hConnect = ::InternetConnect(hInternet, szHostName, INTERNET_INVALID_PORT_NUMBER, szUserName, szPassword, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
		if (NULL == hConnect)
		{
			Ftp_ShowError("InternetConnect");
			break;
		}
		// 打开FTP文件, 文件操作和本地操作相似
		hFTPFile = ::FtpOpenFile(hConnect, szUrlPath, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, NULL);
		if (NULL == hFTPFile)
		{
			Ftp_ShowError("FtpOpenFile");
			break;;
		}
		// 上传数据
		bRet = ::InternetWriteFile(hFTPFile, pUploadData, dwUploadDataSize, &dwBytesReturn);
		if (FALSE == bRet)
		{
			break;
		}

	} while (FALSE);

	// 释放内存并关闭句柄
	if (NULL != hFTPFile)
	{
		::InternetCloseHandle(hFTPFile);
	}
	if (NULL != hConnect)
	{
		::InternetCloseHandle(hConnect);
	}
	if (NULL != hInternet)
	{
		::InternetCloseHandle(hInternet);
	}

	return bRet;
}