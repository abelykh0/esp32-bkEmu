
#include "bkSnapshot.h"
#include "settings.h"
#include "stdint.h"
#include "bkEmu.h"
#include "bkEnvironment.h"
#include "SD.h"
#include "defines.h"

//static fs::FS* _fileSystem;

extern bkEnvironment Environment;
extern pdp_regs pdp;

// First 2 bytes : start address
// Next 2 bytes : length
// Data follows
struct FileHeader
{
	uint16_t Start;
	uint16_t Size;
} __attribute__((packed));

static bool mount()
{
#ifdef SDCARD
	return SD.begin(13, SPI, 20000000U, "/sd", 1);
#else
	return true;
#endif
}

static void unmount()
{
#ifdef SDCARD
	SD.end();
#endif
}

bool LoadSnapshot(File file)
{
	mount();

	FileHeader fileHeader;
	size_t headerSize = sizeof(fileHeader);
	size_t bytesRead = file.read((uint8_t*)&fileHeader, headerSize);
	if (bytesRead != headerSize)
	{
		return false;
	}

	size_t realSize = (size_t)file.size();
	if (realSize != headerSize + fileHeader.Size
		|| fileHeader.Start + fileHeader.Size > 0x4000)
	{
		return false;
	}

	int size = fileHeader.Size;
	uint8_t* buffer = Environment.GetPointer(fileHeader.Start);
	bytesRead = file.read(buffer, size);
	if (bytesRead != size)
	{
		return false;
	}

	pdp.regs[SP] = 01000;
	pdp.regs[PC] = fileHeader.Start;

	unmount();

	return true;
}

bool SaveSnapshot(const char* fileName)
{
	return false;
}
