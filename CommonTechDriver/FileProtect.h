#ifndef _FILE_PROTECT_H_
#define _FILE_PROTECT_H_


#include "IrpFile.h"


PFILE_OBJECT ProtectFile(UNICODE_STRING ustrFileName);
BOOLEAN UnprotectFile(PFILE_OBJECT pFileObject);



#endif