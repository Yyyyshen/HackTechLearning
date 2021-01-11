#include "Cryptopp_tool.h"
// 计算文件的 SHA1 值
string CalSHA1_ByFile(const char* pszFileName)
{
	string value;
	SHA1 sha1;
	FileSource(pszFileName, true, new HashFilter(sha1, new HexEncoder(new StringSink(value))));
	return value;
}


// 计算数据的 SHA1 值
string CalSHA1_ByMem(PBYTE pData, DWORD dwDataSize)
{
	string value;
	SHA1 sha1;
	StringSource(pData, dwDataSize, true, new HashFilter(sha1, new HexEncoder(new StringSink(value))));
	return value;
}

// 计算文件的 SHA256 值
string CalSHA256_ByFile(const char* pszFileName)
{
	string value;
	SHA256 sha256;
	FileSource(pszFileName, true, new HashFilter(sha256, new HexEncoder(new StringSink(value))));
	return value;
}


// 计算数据的 SHA256 值
string CalSHA256_ByMem(PBYTE pData, DWORD dwDataSize)
{
	string value;
	SHA256 sha256;
	StringSource(pData, dwDataSize, true, new HashFilter(sha256, new HexEncoder(new StringSink(value))));
	return value;
}

// 计算文件的 MD5 值
string CalMD5_ByFile(const char* pszFileName)
{
	string value;
	MD5 md5;
	FileSource(pszFileName, true, new HashFilter(md5, new HexEncoder(new StringSink(value))));
	return value;
}


// 计算数据的 MD5 值
string CalMD5_ByMem(PBYTE pData, DWORD dwDataSize)
{
	string value;
	MD5 md5;
	StringSource(pData, dwDataSize, true, new HashFilter(md5, new HexEncoder(new StringSink(value))));
	return value;
}

// 密钥生成
// 输出：AES密钥、AES密钥长度
BOOL GenerateAESKey(BYTE* pAESKey, DWORD* pdwAESKeyLength, DWORD dwBufferSize)
{
	srand(time(NULL));

	if (AES::MIN_KEYLENGTH > *pdwAESKeyLength)
	{
		*pdwAESKeyLength = AES::MIN_KEYLENGTH;
	}
	else if (AES::MAX_KEYLENGTH < *pdwAESKeyLength)
	{
		*pdwAESKeyLength = AES::MAX_KEYLENGTH;
	}


	// 密钥长度大于缓冲区
	if (dwBufferSize < *pdwAESKeyLength)
	{
		return FALSE;
	}

	// 随机生成密钥(大小写字母、数字、字符等可显示字符)
	// 33 - 126
	int i = 0;
	::RtlZeroMemory(pAESKey, dwBufferSize);
	for (i = 0; i < *pdwAESKeyLength; i++)
	{
		pAESKey[i] = 33 + (rand() % 94);
	}

	return TRUE;
}


// 加密
// 输入：原文内容、原文内容长度、密钥内容、密钥内容长度
// 输出：密文内容、密文内容长度
BOOL AES_Encrypt(BYTE* pOriginalData, DWORD dwOriginalDataSize, BYTE* pAESKey, DWORD dwAESKeySize, BYTE** ppEncryptData, DWORD* pdwEncryptData)
{
	// 加密器
	AESEncryption aesEncryptor;
	// 加密原文数据块
	unsigned char inBlock[AES::BLOCKSIZE];
	// 加密后密文数据块
	unsigned char outBlock[AES::BLOCKSIZE];
	// 必须设定全为0
	unsigned char xorBlock[AES::BLOCKSIZE];

	DWORD dwOffset = 0;
	BYTE* pEncryptData = NULL;
	DWORD dwEncryptDataSize = 0;

	// 计算原文长度, 并按 128位 即 16字节 对齐, 不够则 填充0 对齐
	// 商
	DWORD dwQuotient = dwOriginalDataSize / AES::BLOCKSIZE;
	// 余数
	DWORD dwRemaind = dwOriginalDataSize % AES::BLOCKSIZE;
	if (0 != dwRemaind)
	{
		dwQuotient++;
	}

	// 申请动态内存
	dwEncryptDataSize = dwQuotient * AES::BLOCKSIZE;
	pEncryptData = new BYTE[dwEncryptDataSize];
	if (NULL == pEncryptData)
	{
		return FALSE;
	}

	// 设置密钥
	aesEncryptor.SetKey(pAESKey, dwAESKeySize);

	do
	{
		// 置零
		::RtlZeroMemory(inBlock, AES::BLOCKSIZE);
		::RtlZeroMemory(xorBlock, AES::BLOCKSIZE);
		::RtlZeroMemory(outBlock, AES::BLOCKSIZE);

		// 获取加密块
		if (dwOffset <= (dwOriginalDataSize - AES::BLOCKSIZE))
		{
			::RtlCopyMemory(inBlock, (PVOID)(pOriginalData + dwOffset), AES::BLOCKSIZE);
		}
		else
		{
			::RtlCopyMemory(inBlock, (PVOID)(pOriginalData + dwOffset), (dwOriginalDataSize - dwOffset));
		}

		// 加密
		aesEncryptor.ProcessAndXorBlock(inBlock, xorBlock, outBlock);

		// 构造
		::RtlCopyMemory((PVOID)(pEncryptData + dwOffset), outBlock, AES::BLOCKSIZE);

		// 更新数据
		dwOffset = dwOffset + AES::BLOCKSIZE;
		dwQuotient--;
	} while (0 < dwQuotient);

	// 返回数据
	*ppEncryptData = pEncryptData;
	*pdwEncryptData = dwEncryptDataSize;

	return TRUE;
}


// 解密
// 输入：密文内容、密文内容长度、密钥内容、密钥内容长度
// 输出：解密后明文内容、解密后明文内容长度
BOOL AES_Decrypt(BYTE* pEncryptData, DWORD dwEncryptData, BYTE* pAESKey, DWORD dwAESKeySize, BYTE** ppDecryptData, DWORD* pdwDecryptData)
{
	// 解密器
	AESDecryption aesDecryptor;
	// 解密密文数据块
	unsigned char inBlock[AES::BLOCKSIZE];
	// 解密后后明文数据块
	unsigned char outBlock[AES::BLOCKSIZE];
	// 必须设定全为0
	unsigned char xorBlock[AES::BLOCKSIZE];
	DWORD dwOffset = 0;
	BYTE* pDecryptData = NULL;
	DWORD dwDecryptDataSize = 0;

	// 计算密文长度, 并按 128位 即 16字节 对齐, 不够则填充0对齐
	// 商
	DWORD dwQuotient = dwEncryptData / AES::BLOCKSIZE;
	// 余数
	DWORD dwRemaind = dwEncryptData % AES::BLOCKSIZE;
	if (0 != dwRemaind)
	{
		dwQuotient++;
	}

	// 申请动态内存
	dwDecryptDataSize = dwQuotient * AES::BLOCKSIZE;
	pDecryptData = new BYTE[dwDecryptDataSize];
	if (NULL == pDecryptData)
	{
		return FALSE;
	}

	// 设置密钥
	aesDecryptor.SetKey(pAESKey, dwAESKeySize);

	do
	{
		// 置零
		::RtlZeroMemory(inBlock, AES::BLOCKSIZE);
		::RtlZeroMemory(xorBlock, AES::BLOCKSIZE);
		::RtlZeroMemory(outBlock, AES::BLOCKSIZE);

		// 获取解密块
		if (dwOffset <= (dwDecryptDataSize - AES::BLOCKSIZE))
		{
			::RtlCopyMemory(inBlock, (PVOID)(pEncryptData + dwOffset), AES::BLOCKSIZE);
		}
		else
		{
			::RtlCopyMemory(inBlock, (PVOID)(pEncryptData + dwOffset), (dwEncryptData - dwOffset));
		}

		// 解密
		aesDecryptor.ProcessAndXorBlock(inBlock, xorBlock, outBlock);

		// 构造
		::RtlCopyMemory((PVOID)(pDecryptData + dwOffset), outBlock, AES::BLOCKSIZE);

		// 更新数据
		dwOffset = dwOffset + AES::BLOCKSIZE;
		dwQuotient--;
	} while (0 < dwQuotient);

	// 返回数据
	*ppDecryptData = pDecryptData;
	*pdwDecryptData = dwDecryptDataSize;

	return TRUE;
}


// 显示数据
// 输入：输出数据内容、输出数据内容长度
void ShowData(BYTE* pData, DWORD dwSize)
{
	for (int i = 0; i < dwSize; i++)
	{
		if ((0 != i) &&
			(0 == i % 16))
		{
			printf("\n");
		}
		else if ((0 != i) &&
			(0 == i % 8))
		{
			printf(" ");
		}

		printf("%02X ", pData[i]);
	}
	printf("\n");
}


// 生成RSA密钥对
/*
	函数void GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed, DWORD dwSeedLength)是用来产生密钥和公钥文件.
	参数keyLength是密钥长度, PrivFilename是存放密钥的文件名, pubFilename是存放公钥的文件名, seed时产生密钥的种子, dwSeedLength是seed时产生密钥的种子长度.
	该函数:
	1. 首先用类RandomPool的方法Put()产生种子seed的byte型伪随机数;
	2. RSAES_OAEP_SHA_Decryptor是一个解密的公钥密码系统在文件rsa.h 有如下定义：
	   typedef RSAES<OAEP<SHA> >::Decryptor RSAES_OAEP_SHA_Decryptor; 就是在这个类用前面产生的伪随机数和密钥长度keyLength生解密的密钥;
	3. 然后通过类FileSink打开文件szPrivateKeyFileName实行序列化操作，用HexEncoder把它转换为十六进制;
	4. 最后用方法DEREncode()把上面处理好的密码对象写入文件.
	这样就得到私钥密码的密钥文件了。
	产生公钥文件的方法和产生私钥密钥文件不同的地方就是使用了RSAES_OAEP_SHA_Encryptor是一个加密的公钥密码系统, 在文件rsa.h 有如下定义：
	typedef RSAES<OAEP<SHA> >::Encryptor RSAES_OAEP_SHA_Encryptor; 是用上面产生的密钥密码系统priv来生成相应公钥.
*/
BOOL GenerateRSAKey(DWORD dwRSAKeyLength, char* pszPrivateKeyFileName, char* pszPublicKeyFileName, BYTE* pSeed, DWORD dwSeedLength)
{
	RandomPool randPool;
	//randPool.Put(pSeed, dwSeedLength);
	randPool.IncorporateEntropy(pSeed, dwSeedLength);//高版本api有变动

	// 生成RSA私钥
	RSAES_OAEP_SHA_Decryptor priv(randPool, dwRSAKeyLength);
	HexEncoder privFile(new FileSink(pszPrivateKeyFileName));	// 打开文件实行序列化操作

	//priv.DEREncode(privFile);
	priv.AccessMaterial().Save(privFile);
	privFile.MessageEnd();

	// 生成RSA公钥
	RSAES_OAEP_SHA_Encryptor pub(priv);
	HexEncoder pubFile(new FileSink(pszPublicKeyFileName));		// 打开文件实行序列化操作

	//pub.DEREncode(pubFile);										// 写密码对象pub到文件对象pubFile里
	priv.AccessMaterial().Save(privFile);
	pubFile.MessageEnd();

	return TRUE;
}


// RSA加密字符串
/*
	加密函数string RSA_Encrypt(char *pszOriginaString, char *pszPublicKeyFileName, BYTE *pSeed, DWORD dwSeedLength) 中pszPublicKeyFileName是公钥文件，pSeed是加密种子，pszOriginaString是要加密的字符串。
	这个函数的基本流程是：
	1. 首先用类RandomPool在种子seed下用方法Put()产生伪随机数，Seed可以任取;;
	2. 用类FileSource对公钥文件pubFilename进行一定的转换放入临时缓冲区，并把它从十六进制转换为byte型;
	3. 然后用FileSource的对象pubFile 实例化公钥密码系统RSAES_OAEP_SHA_Encryptor生成对象pub;
	4. 用类StringSink 把outstr添加到一个String对象，接着用HexEncoder把这个对象转换为十六进制;
	5. 然后用伪随机数randPool、公钥密码系统pub和十六进制的String对象实例化一个公钥密码加密的过滤器，再用这个过滤器对字符串message进行加密把结果放到十六进制的字符串result里，这样就完成了对字符串的加密。
*/
string RSA_Encrypt_ByFile(char* pszOriginaString, char* pszPublicKeyFileName, BYTE* pSeed, DWORD dwSeedLength)
{
	RandomPool randPool;
	randPool.IncorporateEntropy(pSeed, dwSeedLength);

	FileSource pubFile(pszPublicKeyFileName, TRUE, new HexDecoder);
	RSAES_OAEP_SHA_Encryptor pub(pubFile);

	// 加密
	string strEncryptString;
	StringSource(pszOriginaString, TRUE, new PK_EncryptorFilter(randPool, pub, new HexEncoder(new StringSink(strEncryptString))));

	return strEncryptString;
}


// RSA解密字符串
/*
	解密函数的基本流程跟加密函数的基本流程差不多，就使用了几个不同的类，但是这些类跟加密函数的对应类的功能是相对的，很容易理解所以就不多加以解释
*/
string RSA_Decrypt_ByFile(char* pszEncryptString, char* pszPrivateKeyFileName)
{
	FileSource privFile(pszPrivateKeyFileName, TRUE, new HexDecoder);
	RSAES_OAEP_SHA_Decryptor priv(privFile);

	string strDecryptString;
	StringSource(pszEncryptString, TRUE, new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(strDecryptString))));

	return strDecryptString;
}


RandomPool& GlobalRNG()
{
	static RandomPool randomPool;

	return randomPool;
}


// RSA加密字符串
string RSA_Encrypt_ByMem(char* pszOriginaString, char* pszMemPublicKey, BYTE* pSeed, DWORD dwSeedLength)
{
	RandomPool randPool;
	randPool.IncorporateEntropy(pSeed, dwSeedLength);

	StringSource pubStr(pszMemPublicKey, TRUE, new HexDecoder);
	RSAES_OAEP_SHA_Encryptor pub(pubStr);

	// 加密
	string strEncryptString;
	StringSource(pszOriginaString, TRUE, new PK_EncryptorFilter(randPool, pub, new HexEncoder(new StringSink(strEncryptString))));

	return strEncryptString;
}


// RSA解密字符串
string RSA_Decrypt_ByMem(char* pszEncryptString, char* pszMemPrivateKey)
{
	StringSource privStr(pszMemPrivateKey, TRUE, new HexDecoder);
	RSAES_OAEP_SHA_Decryptor priv(privStr);

	string strDecryptString;
	StringSource(pszEncryptString, TRUE, new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(strDecryptString))));

	return strDecryptString;
}