// EncrypTech.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

/**
 * 加密技术
 * 恶意程序在采集信息时，除了压缩数据，为了不被发现传输内容
 * 还需要对数据进行加密
 */

 /**
  * Windows自带加密库
  * 使用CryptoAPI函数
  * 主要有HASH、AES、RSA加解密
  */

  //Hash是把任意长度通过算法变为固定长度，不同输入可能得到相同结果，所以不能利用hash确定唯一，但经常用来校验完整性
void ShowError(const char* pszText)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error[%d]\n", pszText, ::GetLastError());
#ifdef _DEBUG
	::MessageBox(NULL, szErr, "ERROR", MB_OK | MB_ICONERROR);
#endif
}


BOOL GetFileData(const char* pszFilePath, BYTE** ppFileData, DWORD* pdwFileDataLength)
{
	BOOL bRet = TRUE;
	BYTE* pFileData = NULL;
	DWORD dwFileDataLength = 0;
	HANDLE hFile = NULL;
	DWORD dwTemp = 0;

	do
	{
		hFile = ::CreateFile(pszFilePath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			bRet = FALSE;
			ShowError("CreateFile");
			break;
		}

		dwFileDataLength = ::GetFileSize(hFile, NULL);

		pFileData = new BYTE[dwFileDataLength];
		if (NULL == pFileData)
		{
			bRet = FALSE;
			ShowError("new");
			break;
		}
		::RtlZeroMemory(pFileData, dwFileDataLength);

		::ReadFile(hFile, pFileData, dwFileDataLength, &dwTemp, NULL);

		// 返回
		*ppFileData = pFileData;
		*pdwFileDataLength = dwFileDataLength;

	} while (FALSE);

	if (hFile)
	{
		::CloseHandle(hFile);
	}

	return bRet;
}


BOOL CalculateHash(BYTE* pData, DWORD dwDataLength, ALG_ID algHashType, BYTE** ppHashData, DWORD* pdwHashDataLength)
{
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTHASH hCryptHash = NULL;
	BYTE* pHashData = NULL;
	DWORD dwHashDataLength = 0;
	DWORD dwTemp = 0;
	BOOL bRet = FALSE;


	do
	{
		// 获得指定CSP的密钥容器的句柄
		bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
		if (FALSE == bRet)
		{
			ShowError("CryptAcquireContext");
			break;
		}

		// 创建一个HASH对象, 指定HASH算法
		bRet = ::CryptCreateHash(hCryptProv, algHashType, NULL, NULL, &hCryptHash);
		if (FALSE == bRet)
		{
			ShowError("CryptCreateHash");
			break;
		}

		// 计算HASH数据
		bRet = ::CryptHashData(hCryptHash, pData, dwDataLength, 0);
		if (FALSE == bRet)
		{
			ShowError("CryptHashData");
			break;
		}

		// 获取HASH结果的大小
		dwTemp = sizeof(dwHashDataLength);
		bRet = ::CryptGetHashParam(hCryptHash, HP_HASHSIZE, (BYTE*)(&dwHashDataLength), &dwTemp, 0);
		if (FALSE == bRet)
		{
			ShowError("CryptGetHashParam");
			break;
		}

		// 申请内存
		pHashData = new BYTE[dwHashDataLength];
		if (NULL == pHashData)
		{
			bRet = FALSE;
			ShowError("new");
			break;
		}
		::RtlZeroMemory(pHashData, dwHashDataLength);

		// 获取HASH结果数据
		bRet = ::CryptGetHashParam(hCryptHash, HP_HASHVAL, pHashData, &dwHashDataLength, 0);
		if (FALSE == bRet)
		{
			ShowError("CryptGetHashParam");
			break;
		}

		// 返回数据
		*ppHashData = pHashData;
		*pdwHashDataLength = dwHashDataLength;

	} while (FALSE);

	// 释放关闭
	if (FALSE == bRet)
	{
		if (pHashData)
		{
			delete[]pHashData;
			pHashData = NULL;
		}
	}
	if (hCryptHash)
	{
		::CryptDestroyHash(hCryptHash);
	}
	if (hCryptProv)
	{
		::CryptReleaseContext(hCryptProv, 0);
	}

	return bRet;
}

void test_hash()
{
	BYTE* pData = NULL;
	DWORD dwDataLength = 0;
	DWORD i = 0;
	BYTE* pHashData = NULL;
	DWORD dwHashDataLength = 0;

	// 读取文件数据
	GetFileData("C:\\workspaceKernel\\HackTechLearning\\Debug\\HackTechLearning.exe", &pData, &dwDataLength);

	// MD5
	CalculateHash(pData, dwDataLength, CALG_MD5, &pHashData, &dwHashDataLength);
	printf("MD5[%d]\n", dwHashDataLength);
	for (i = 0; i < dwHashDataLength; i++)
	{
		printf("%x", pHashData[i]);
	}
	printf("\n\n");
	if (pHashData)
	{
		delete[]pHashData;
		pHashData = NULL;
	}

	// SHA1
	CalculateHash(pData, dwDataLength, CALG_SHA1, &pHashData, &dwHashDataLength);
	printf("SHA1[%d]\n", dwHashDataLength);
	for (i = 0; i < dwHashDataLength; i++)
	{
		printf("%x", pHashData[i]);
	}
	printf("\n\n");
	if (pHashData)
	{
		delete[]pHashData;
		pHashData = NULL;
	}

	// SHA256
	CalculateHash(pData, dwDataLength, CALG_SHA_256, &pHashData, &dwHashDataLength);
	printf("SHA256[%d]\n", dwHashDataLength);
	for (i = 0; i < dwHashDataLength; i++)
	{
		printf("%x", pHashData[i]);
	}
	printf("\n\n");
	if (pHashData)
	{
		delete[]pHashData;
		pHashData = NULL;
	}

	system("pause");
}

//AES是常见对称加密算法，为分组密码，即把明文分成长度相同(128位-16字节)的组，每次加密一组数据
// AES加密
BOOL AesEncrypt(BYTE* pPassword, DWORD dwPasswordLength, BYTE* pData, DWORD& dwDataLength, DWORD dwBufferLength)
{
	BOOL bRet = TRUE;
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTHASH hCryptHash = NULL;
	HCRYPTKEY hCryptKey = NULL;

	do
	{
		// 获取CSP句柄
		bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
		if (FALSE == bRet)
		{
			ShowError("CryptAcquireContext");
			break;
		}

		// 创建HASH对象
		bRet = ::CryptCreateHash(hCryptProv, CALG_MD5, NULL, 0, &hCryptHash);
		if (FALSE == bRet)
		{
			ShowError("CryptCreateHash");
			break;
		}

		// 对密钥进行HASH计算
		bRet = ::CryptHashData(hCryptHash, pPassword, dwPasswordLength, 0);
		if (FALSE == bRet)
		{
			ShowError("CryptHashData");
			break;
		}

		// 使用HASH来生成密钥
		bRet = ::CryptDeriveKey(hCryptProv, CALG_AES_128, hCryptHash, CRYPT_EXPORTABLE, &hCryptKey);
		if (FALSE == bRet)
		{
			ShowError("CryptDeriveKey");
			break;
		}

		// 加密数据
		bRet = ::CryptEncrypt(hCryptKey, NULL, TRUE, 0, pData, &dwDataLength, dwBufferLength);
		if (FALSE == bRet)
		{
			ShowError("CryptEncrypt");
			break;
		}

	} while (FALSE);

	// 关闭释放
	if (hCryptKey)
	{
		::CryptDestroyKey(hCryptKey);
	}
	if (hCryptHash)
	{
		::CryptDestroyHash(hCryptHash);
	}
	if (hCryptProv)
	{
		::CryptReleaseContext(hCryptProv, 0);
	}

	return bRet;
}


// AES解密
BOOL AesDecrypt(BYTE* pPassword, DWORD dwPasswordLength, BYTE* pData, DWORD& dwDataLength, DWORD dwBufferLength)
{
	BOOL bRet = TRUE;
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTHASH hCryptHash = NULL;
	HCRYPTKEY hCryptKey = NULL;

	do
	{
		// 获取CSP句柄
		bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
		if (FALSE == bRet)
		{
			ShowError("CryptAcquireContext");
			break;
		}

		// 创建HASH对象
		bRet = ::CryptCreateHash(hCryptProv, CALG_MD5, NULL, 0, &hCryptHash);
		if (FALSE == bRet)
		{
			ShowError("CryptCreateHash");
			break;
		}

		// 对密钥进行HASH计算
		bRet = ::CryptHashData(hCryptHash, pPassword, dwPasswordLength, 0);
		if (FALSE == bRet)
		{
			ShowError("CryptHashData");
			break;
		}

		// 使用HASH来生成密钥
		bRet = ::CryptDeriveKey(hCryptProv, CALG_AES_128, hCryptHash, CRYPT_EXPORTABLE, &hCryptKey);
		if (FALSE == bRet)
		{
			ShowError("CryptDeriveKey");
			break;
		}

		// 解密数据
		bRet = ::CryptDecrypt(hCryptKey, NULL, TRUE, 0, pData, &dwDataLength);
		if (FALSE == bRet)
		{
			ShowError("CryptDecrypt");
			break;
		}

	} while (FALSE);

	// 关闭释放
	if (hCryptKey)
	{
		::CryptDestroyKey(hCryptKey);
	}
	if (hCryptHash)
	{
		::CryptDestroyHash(hCryptHash);
	}
	if (hCryptProv)
	{
		::CryptReleaseContext(hCryptProv, 0);
	}

	return bRet;
}

void test_aes()
{
	BYTE pData[MAX_PATH] = { 0 };
	DWORD dwDataLength = 0, dwBufferLength = MAX_PATH;
	DWORD i = 0;

	::RtlZeroMemory(pData, dwBufferLength);
	::lstrcpy((char*)pData, "What is your name? DemonGan");
	dwDataLength = 1 + ::lstrlen((char*)pData);

	printf("Text[%d]\n", dwDataLength);
	for (i = 0; i < dwDataLength; i++)
	{
		printf("%x ", pData[i]);
	}
	printf("\n\n");

	// AES 加密
	AesEncrypt((BYTE*)"DemonGanDemonGan", 16, pData, dwDataLength, dwBufferLength);
	printf("AES Encrypt[%d]\n", dwDataLength);
	for (i = 0; i < dwDataLength; i++)
	{
		printf("%x ", pData[i]);
	}
	printf("\n\n");

	// AES 解密
	AesDecrypt((BYTE*)"DemonGanDemonGan", 16, pData, dwDataLength, dwBufferLength);
	printf("AES Decrypt[%d]\n", dwDataLength);
	for (i = 0; i < dwDataLength; i++)
	{
		printf("%x ", pData[i]);
	}
	printf("\n\n");

	system("pause");
}

//RSA是非对称加密算法，安全性高但速度有缺陷，所以一般用于少量数据加密
// 生成公钥和私钥
BOOL GenerateKey(BYTE** ppPublicKey, DWORD* pdwPublicKeyLength, BYTE** ppPrivateKey, DWORD* pdwPrivateKeyLength)
{
	BOOL bRet = TRUE;
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTKEY hCryptKey = NULL;
	BYTE* pPublicKey = NULL;
	DWORD dwPublicKeyLength = 0;
	BYTE* pPrivateKey = NULL;
	DWORD dwPrivateKeyLength = 0;

	do
	{
		// 获取CSP句柄
		bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0);
		if (FALSE == bRet)
		{
			ShowError("CryptAcquireContext");
			break;
		}

		// 生成公私密钥对
		bRet = ::CryptGenKey(hCryptProv, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &hCryptKey);
		if (FALSE == bRet)
		{
			ShowError("CryptGenKey");
			break;
		}

		// 获取公钥密钥的长度和内容
		bRet = ::CryptExportKey(hCryptKey, NULL, PUBLICKEYBLOB, 0, NULL, &dwPublicKeyLength);
		if (FALSE == bRet)
		{
			ShowError("CryptExportKey");
			break;
		}
		pPublicKey = new BYTE[dwPublicKeyLength];
		::RtlZeroMemory(pPublicKey, dwPublicKeyLength);
		bRet = ::CryptExportKey(hCryptKey, NULL, PUBLICKEYBLOB, 0, pPublicKey, &dwPublicKeyLength);
		if (FALSE == bRet)
		{
			ShowError("CryptExportKey");
			break;
		}

		// 获取私钥密钥的长度和内容
		bRet = ::CryptExportKey(hCryptKey, NULL, PRIVATEKEYBLOB, 0, NULL, &dwPrivateKeyLength);
		if (FALSE == bRet)
		{
			ShowError("CryptExportKey");
			break;
		}
		pPrivateKey = new BYTE[dwPrivateKeyLength];
		::RtlZeroMemory(pPrivateKey, dwPrivateKeyLength);
		bRet = ::CryptExportKey(hCryptKey, NULL, PRIVATEKEYBLOB, 0, pPrivateKey, &dwPrivateKeyLength);
		if (FALSE == bRet)
		{
			ShowError("CryptExportKey");
			break;
		}

		// 返回数据
		*ppPublicKey = pPublicKey;
		*pdwPublicKeyLength = dwPublicKeyLength;
		*ppPrivateKey = pPrivateKey;
		*pdwPrivateKeyLength = dwPrivateKeyLength;

	} while (FALSE);

	// 释放关闭
	if (hCryptKey)
	{
		::CryptDestroyKey(hCryptKey);
	}
	if (hCryptProv)
	{
		::CryptReleaseContext(hCryptProv, 0);
	}

	return bRet;
}


// 公钥加密数据
BOOL RsaEncrypt(BYTE* pPublicKey, DWORD dwPublicKeyLength, BYTE* pData, DWORD& dwDataLength, DWORD dwBufferLength)
{
	BOOL bRet = TRUE;
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTKEY hCryptKey = NULL;

	do
	{
		// 获取CSP句柄
		bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0);
		if (FALSE == bRet)
		{
			ShowError("CryptAcquireContext");
			break;
		}

		// 导入公钥
		bRet = ::CryptImportKey(hCryptProv, pPublicKey, dwPublicKeyLength, NULL, 0, &hCryptKey);
		if (FALSE == bRet)
		{
			ShowError("CryptImportKey");
			break;
		}

		// 加密数据
		bRet = ::CryptEncrypt(hCryptKey, NULL, TRUE, 0, pData, &dwDataLength, dwBufferLength);
		if (FALSE == bRet)
		{
			ShowError("CryptImportKey");
			break;
		}

	} while (FALSE);

	// 释放并关闭
	if (hCryptKey)
	{
		::CryptDestroyKey(hCryptKey);
	}
	if (hCryptProv)
	{
		::CryptReleaseContext(hCryptProv, 0);
	}

	return bRet;
}


// 私钥解密数据
BOOL RsaDecrypt(BYTE* pPrivateKey, DWORD dwProvateKeyLength, BYTE* pData, DWORD& dwDataLength)
{
	BOOL bRet = TRUE;
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTKEY hCryptKey = NULL;

	do
	{
		// 获取CSP句柄
		bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0);
		if (FALSE == bRet)
		{
			ShowError("CryptAcquireContext");
			break;
		}

		// 导入私钥
		bRet = ::CryptImportKey(hCryptProv, pPrivateKey, dwProvateKeyLength, NULL, 0, &hCryptKey);
		if (FALSE == bRet)
		{
			ShowError("CryptImportKey");
			break;
		}

		// 解密数据
		bRet = ::CryptDecrypt(hCryptKey, NULL, TRUE, 0, pData, &dwDataLength);
		if (FALSE == bRet)
		{
			ShowError("CryptDecrypt");
			break;
		}

	} while (FALSE);

	// 释放并关闭
	if (hCryptKey)
	{
		::CryptDestroyKey(hCryptKey);
	}
	if (hCryptProv)
	{
		::CryptReleaseContext(hCryptProv, 0);
	}

	return bRet;
}

void test_rsa()
{
	BYTE* pPublicKey = NULL;
	DWORD dwPublicKeyLength = 0;
	BYTE* pPrivateKey = NULL;
	DWORD dwPrivateKeyLength = 0;
	BYTE* pData = NULL;
	DWORD dwDataLength = 0;
	DWORD dwBufferLength = 4096;
	DWORD i = 0;

	pData = new BYTE[dwBufferLength];
	if (NULL == pData)
	{
		return;
	}
	::RtlZeroMemory(pData, dwBufferLength);
	::lstrcpy((char*)pData, "What is your name? DemonGan");
	dwDataLength = 1 + ::lstrlen((char*)pData);
	printf("Text[%d]\n", dwDataLength);
	for (i = 0; i < dwDataLength; i++)
	{
		printf("%x", pData[i]);
	}
	printf("\n\n");

	// 生成公钥和私钥
	GenerateKey(&pPublicKey, &dwPublicKeyLength, &pPrivateKey, &dwPrivateKeyLength);
	printf("Public Key[%d]\n", dwPublicKeyLength);
	for (i = 0; i < dwPublicKeyLength; i++)
	{
		printf("%.2x", pPublicKey[i]);
	}
	printf("\n");
	printf("Private Key[%d]\n", dwPrivateKeyLength);
	for (i = 0; i < dwPrivateKeyLength; i++)
	{
		printf("%.2x", pPrivateKey[i]);
	}
	printf("\n\n");

	// 公钥加密
	RsaEncrypt(pPublicKey, dwPublicKeyLength, pData, dwDataLength, dwBufferLength);
	printf("RSA Encrypt[%d]\n", dwDataLength);
	for (i = 0; i < dwDataLength; i++)
	{
		printf("%x", pData[i]);
	}
	printf("\n\n");

	// 私钥解密
	RsaDecrypt(pPrivateKey, dwPrivateKeyLength, pData, dwDataLength);
	printf("RSA Decrypt[%d]\n", dwDataLength);
	for (i = 0; i < dwDataLength; i++)
	{
		printf("%x", pData[i]);
	}
	printf("\n\n");

	// 释放
	if (pData)
	{
		delete[]pData;
		pData = NULL;
	}
	if (pPrivateKey)
	{
		delete[]pPrivateKey;
		pPrivateKey = NULL;
	}
	if (pPublicKey)
	{
		delete[]pPublicKey;
		pPublicKey = NULL;
	}

	system("pause");
}

int main()
{
	//hash
	//test_hash();
	//test_aes();
	//test_rsa();

	return 0;
}

