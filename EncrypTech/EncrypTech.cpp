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

/**
 * Crypto++库是一个c++密码库，在常见的加解密算法基础上实现了一个统一的、基于c++模板的编程接口
 */
#include "Cryptopp_tool.h"
void test_sha1()
{
	// 方式一 输入文件名称
	string sha11 = CalSHA1_ByFile("C:\\workspaceKernel\\HackTechLearning\\Debug\\HackTechLearning.exe");
	printf("sha11=%s\n", sha11.c_str());
	// 方式二 输入文件数据内容
	HANDLE hFile2 = ::CreateFile("C:\\workspaceKernel\\HackTechLearning\\Debug\\HackTechLearning.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
	DWORD dwFileSize2 = ::GetFileSize(hFile2, NULL);
	BYTE* pData2 = new BYTE[dwFileSize2];
	::ReadFile(hFile2, pData2, dwFileSize2, NULL, NULL);
	string sha12 = CalSHA1_ByMem(pData2, dwFileSize2);
	printf("sha12=%s\n", sha12.c_str());

	system("pause");
}

void test_sha256()
{
	// 方式一 输入文件名称
	string sha2561 = CalSHA256_ByFile("C:\\workspaceKernel\\HackTechLearning\\Debug\\HackTechLearning.exe");
	printf("sha2561=%s\n", sha2561.c_str());
	// 方式二 输入文件数据内容
	HANDLE hFile2 = ::CreateFile("C:\\workspaceKernel\\HackTechLearning\\Debug\\HackTechLearning.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
	DWORD dwFileSize2 = ::GetFileSize(hFile2, NULL);
	BYTE* pData2 = new BYTE[dwFileSize2];
	::ReadFile(hFile2, pData2, dwFileSize2, NULL, NULL);
	string sha2562 = CalSHA256_ByMem(pData2, dwFileSize2);
	printf("sha2562=%s\n", sha2562.c_str());

	system("pause");
}

void test_md5()
{
	// 方式一 输入文件名称
	string md51 = CalMD5_ByFile("C:\\workspaceKernel\\HackTechLearning\\Debug\\HackTechLearning.exe");
	printf("md51=%s\n", md51.c_str());
	// 方式二 输入文件数据内容
	HANDLE hFile2 = ::CreateFile("C:\\workspaceKernel\\HackTechLearning\\Debug\\HackTechLearning.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
	DWORD dwFileSize2 = ::GetFileSize(hFile2, NULL);
	BYTE* pData2 = new BYTE[dwFileSize2];
	::ReadFile(hFile2, pData2, dwFileSize2, NULL, NULL);
	string md52 = CalMD5_ByMem(pData2, dwFileSize2);
	printf("md52=%s\n", md52.c_str());

	system("pause");
}

void test_crypto_aes()
{
	BYTE* pEncryptData = NULL;
	DWORD dwEncryptDataSize = 0;
	BYTE* pDecryptData = NULL;
	DWORD dwDecryptDataSize = 0;
	char szOriginalData[] = "I am DemonGan";
	char szAESKey[] = "DemonGanDemonGan";
	BOOL  bRet = FALSE;

	// 加密
	bRet = AES_Encrypt((BYTE*)szOriginalData, (1 + ::lstrlen(szOriginalData)), (BYTE*)szAESKey, ::lstrlen(szAESKey), &pEncryptData, &dwEncryptDataSize);
	if (FALSE == bRet)
	{
		return;
	}

	// 解密
	bRet = AES_Decrypt(pEncryptData, dwEncryptDataSize, (BYTE*)szAESKey, ::lstrlen(szAESKey), &pDecryptData, &dwDecryptDataSize);
	if (FALSE == bRet)
	{
		return;
	}

	// 显示
	printf("原文数据:\n");
	ShowData((BYTE*)szOriginalData, (1 + ::lstrlen(szOriginalData)));
	printf("密文数据:\n");
	ShowData(pEncryptData, dwEncryptDataSize);
	printf("解密后数据:\n");
	ShowData(pDecryptData, (1 + ::lstrlen(szOriginalData)));

	// 释放内存
	delete[]pEncryptData;
	pEncryptData = NULL;
	delete[]pDecryptData;
	pDecryptData = NULL;

	system("pause");
}

char g_szPubKey[] = "30819D300D06092A864886F70D010101050003818B0030818702818100F0CE882D7CCB990323A6DB1B775EBE8F2910BFE75B4B580EF8C5089BB25FEDEEABCE2BBD2AC64A138E47F96A6C39152FE98067C0B4F5DC28F8D9394325ADB12A90A9598FF7A2A7211DEF974FC8A005D0CBCDE059FB8F7F9D214C5BAC2532CEB8EC4041AEAB19E80B8C4020F4A50102F9E738647E2384EA2FCD30C3681559CF6F020111";
char g_szPrivKey[] = "30820275020100300D06092A864886F70D01010105000482025F3082025B02010002818100F0CE882D7CCB990323A6DB1B775EBE8F2910BFE75B4B580EF8C5089BB25FEDEEABCE2BBD2AC64A138E47F96A6C39152FE98067C0B4F5DC28F8D9394325ADB12A90A9598FF7A2A7211DEF974FC8A005D0CBCDE059FB8F7F9D214C5BAC2532CEB8EC4041AEAB19E80B8C4020F4A50102F9E738647E2384EA2FCD30C3681559CF6F020111028180210D49E8203005F15F3F0F03C5170B18AB4892CF70EC39434F52426FB91C39C162E0100AE7C0DCFDAA1DF50E9B67351AA7942251AA68051EB8BE7145739A599220030CF5E35ED4DEA41DD6E955722AE46153339FE7417BD00ADF53B368EAB6E71FAE0F7F394A34C91612B0F11AEC5525DB84DD982E6BF10CE74F177FA51ADC51024100F80296900AF134CCC5AC12C58D741C735F5EE9CBDFB8C1B1EB039BF078E37B09322074193B7B0AE5A60B544DDDB9159294E91744404A2C7CDF96287F5483D691024100F8908925066C3ED9AC8EAFE63A59D56FCBEC354A3DD513489DEDA70E42338CD2AEBDEEF685148123B31A55CA27B2A59CA53E2352DA284F30585A5D6B571245FF02410091E367A0066FC4B4B083565616F901AD4728C5C3384E900E4E021F7E653A849BFF5E6269320C24871661046A09F4670AEE2EC264620D8394BFC1BD781398D891024057BA8AC1C608162EB55F896050D46972C0717C38520EF7BF46CC5914175D7CFF107F4547F2BBF157E4DC1E47594E1C55677F57C2E395C19897A76C44009D09A5024100BBB92D3E8776B52FA20303E39FE8AE862637BB75880D82C6580C3217445C4A95BFB6E94120AD62AADC313418A350FF21B0ED861848626CC0F55936F750B44FC4";

void test_crypto_rsa()
{
	char szPrivateFile[] = "privatefile";
	char szPublicFile[] = "publicfile";
	char szSeed[] = "DemonGanDemonGan";

	char szOriginalString[] = "I am DemonGan";

	/* 密钥在文件方式 */
	//// 生成RSA公私密钥对
	//GenerateRSAKey(1024, szPrivateFile, szPublicFile, (BYTE*)szSeed, ::lstrlen(szSeed));
	//// RSA公钥加密字符串
	//string strEncryptString = RSA_Encrypt_ByFile(szOriginalString, szPublicFile, (BYTE*)szSeed, ::lstrlen(szSeed));
	//// RSA私钥解密字符串
	//string strDecryptString = RSA_Decrypt_ByFile((char*)strEncryptString.c_str(), szPrivateFile);
	//// 显示
	//printf("原文字符串:\n[%d]%s\n", ::lstrlen(szOriginalString), szOriginalString);
	//printf("密文字符串:\n[%d]%s\n", strEncryptString.length(), strEncryptString.c_str());
	//printf("解密明文字符串:\n[%d]%s\n", strDecryptString.length(), strDecryptString.c_str());

	printf("\n\n");

	/* 密钥在内存方式 */
	// RSA公钥加密字符串
	string strEncryptString_Mem = RSA_Encrypt_ByMem(szOriginalString, g_szPubKey, (BYTE*)szSeed, ::lstrlen(szSeed));
	// RSA私钥解密字符串
	string strDecryptString_Mem = RSA_Decrypt_ByMem((char*)strEncryptString_Mem.c_str(), g_szPrivKey);
	// 显示
	printf("原文字符串:\n[%d]%s\n", ::lstrlen(szOriginalString), szOriginalString);
	printf("密文字符串:\n[%d]%s\n", strEncryptString_Mem.length(), strEncryptString_Mem.c_str());
	printf("解密明文字符串:\n[%d]%s\n", strDecryptString_Mem.length(), strDecryptString_Mem.c_str());

	system("pause");
}

int main()
{
	//hash
	//test_hash();
	//test_aes();
	//test_rsa();

	//crypto++
	//test_sha1();
	//test_sha256();
	//test_md5();
	//test_crypto_aes();
	test_crypto_rsa();

	return 0;
}

