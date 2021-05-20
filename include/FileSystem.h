#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "FS.h"

void FileSystemInitialize(fs::FS* fileSystem);

bool loadSnapshotSetup(const char* path);
bool loadSnapshotLoop();

//bool saveSnapshotSetup(const char* path);
//bool saveSnapshotLoop();

bool ReadFromFile(const char* fileName, uint8_t* buffer, size_t size);

#endif
