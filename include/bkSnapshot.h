#ifndef __BKSNAPSHOT_INCLUDED__
#define __BKSNAPSHOT_INCLUDED__

#include <stdint.h>
#include "fatfs.h"

namespace bk
{

bool LoadSnapshot(FIL* file);
bool SaveSnapshot(FIL* file);

}

#endif
