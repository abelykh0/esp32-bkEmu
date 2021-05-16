/*

#include "bkSnapshot.h"
#include "bkEmu.h"

// First 2 bytes : start address
// Next 2 bytes : length
// Data follows

struct FileHeader
{
	uint16_t Start;
	uint16_t Size;
} __attribute__((packed));

bool bk::LoadSnapshot(FIL* file)
{

	FileHeader fileHeader;
	UINT bytesRead;
	uint8_t headerSize = sizeof(fileHeader);
	FRESULT readResult = f_read(file, &fileHeader, headerSize, &bytesRead);
	if (readResult != FR_OK || bytesRead != headerSize)
	{
		return false;
	}

	FSIZE_t realSize = f_size(file);
	if (realSize != headerSize + fileHeader.Size
		|| fileHeader.Start > RAM_AVAILABLE)
	{
		return false;
	}

	int remainingBytes = fileHeader.Size;
	uint8_t* buffer = &RamBuffer[fileHeader.Start];
	do
	{
		uint32_t bytesToRead = remainingBytes < FF_MIN_SS ? remainingBytes : FF_MIN_SS;
		readResult = f_read(file, buffer, bytesToRead, &bytesRead);
		if (readResult != FR_OK || bytesRead != bytesToRead)
		{
			return false;
		}

		remainingBytes -= bytesRead;
		buffer += bytesRead;
	} while (readResult == FR_OK && remainingBytes > 0);

	pdp.regs[PC] = fileHeader.Start;

	return true;
}

bool bk::SaveSnapshot(FIL* file)
{
	return false;
}
*/