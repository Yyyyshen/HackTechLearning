#ifndef _FORCE_DELETE_H_
#define _FORCE_DELETE_H_


#include "IrpFile.h"


// 强制删除文件
NTSTATUS ForceDeleteFile(UNICODE_STRING ustrFileName);


#endif