#ifndef __BKSNAPSHOT_INCLUDED__
#define __BKSNAPSHOT_INCLUDED__

#include <stdint.h>
#include "FS.h"

void FileSystemInitialize(fs::FS* fileSystem);

bool LoadSnapshot(const char* fileName);
bool SaveSnapshot(const char* fileName);

#endif
