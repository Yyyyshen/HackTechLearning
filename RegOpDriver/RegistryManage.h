#ifndef _REGISTRY_MANAGE_H_
#define _REGISTRY_MANAGE_H_


#include <ntddk.h>


// 创建或者打开已存在注册表键
BOOLEAN MyCreateRegistryKey(UNICODE_STRING ustrRegistry);

// 打开注册表键
BOOLEAN MyOpenRegistryKey(UNICODE_STRING ustrRegistry);

// 添加或者修改注册表键值
BOOLEAN MySetRegistryKeyValue(UNICODE_STRING ustrRegistry, UNICODE_STRING ustrKeyValueName, ULONG ulKeyValueType, PVOID pKeyValueData, ULONG ulKeyValueDataSize);

// 删除注册表键
BOOLEAN MyDeleteRegistryKey(UNICODE_STRING ustrRegistry);

// 删除注册表键值
BOOLEAN MyDeleteRegistryKeyValue(UNICODE_STRING ustrRegistry, UNICODE_STRING ustrKeyValueName);

// 查询注册表键值
BOOLEAN MyQueryRegistryKeyValue(UNICODE_STRING ustrRegistry, UNICODE_STRING ustrKeyValueName);


#endif