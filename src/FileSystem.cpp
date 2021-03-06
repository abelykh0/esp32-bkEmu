#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "Ff.h"

#include "FileSystem.h"
#include "Emulator.h"
#include "ps2Input.h"
#include "SD.h"
#include "ScreenArea.h"
#include "bkSnapshot.h"

using namespace fabgl;

#define DEBUG_COLUMNS 64
#define DEBUG_ROWS 26
#define FILE_COLUMNS 3
#define FILE_COLUMNWIDTH 20
#define MAX_LFN 90

static uint8_t _buffer16K_1[0x4000];
static uint8_t _buffer16K_2[0x4000];

extern VideoController* Screen;
extern ScreenArea DebugScreen;

typedef TCHAR FileName[MAX_LFN + 1];

static FileName* _fileNames = (FileName*)_buffer16K_2;
static int16_t _selectedFile = 0;
static int16_t _fileCount;
static bool _loadingSnapshot = false;
//static bool _savingSnapshot = false;
//static char* _snapshotName = ((char*)_buffer16K_1) + MAX_LFN;

static fs::FS* _fileSystem;
static const char* _rootFolder;
static int _rootFolderLength;

static FRESULT mount()
{
#ifdef SDCARD
	return SD.begin(13, SPI, 20000000U, "/sd", 1) ? FR_OK : FR_NOT_READY;
#else
	return FR_OK;
#endif
}

static void unmount()
{
#ifdef SDCARD
	SD.end();
#endif
}

static void GetFileCoord(uint16_t fileIndex, uint8_t* x, uint8_t* y)
{
	*x = 1 + fileIndex / (DEBUG_ROWS - 1) * (FILE_COLUMNWIDTH + 1);
	*y = 1 + fileIndex % (DEBUG_ROWS - 1);
}

static TCHAR* GetFileName(TCHAR* fileName)
{
	TCHAR* result = (TCHAR*)_buffer16K_1;
    strncpy(result, _rootFolder, _rootFolderLength);
	strncpy(result + _rootFolderLength, fileName, MAX_LFN);

    return result;
}

static TCHAR* FileExtension(TCHAR* fileName)
{
	TCHAR* result = (TCHAR*)_buffer16K_1;
	strncpy(result, fileName, MAX_LFN);

	result[MAX_LFN - 1] = '\0';
	char* extension = strrchr(result, '.');
    if (extension != nullptr)
    {
        for(int i = 0; extension[i]; i++)
        {
            extension[i] = tolower(extension[i]);
        }
    }
    else
    {
    	result[0] = '\0';
        extension = result;
    }

    return extension;
}

static TCHAR* TruncateFileName(TCHAR* fileName)
{
	int maxLength = FILE_COLUMNWIDTH + 1;
	TCHAR* result = (TCHAR*) _buffer16K_1;
	strncpy(result, fileName, maxLength);

	result[maxLength - 1] = '\0';
	TCHAR* extension = strrchr(result, '.');
	if (extension != nullptr)
	{
		*extension = '\0';
	}

	return result;
}

static void SetSelection(uint8_t selectedFile)
{
	if (_fileCount == 0)
	{
		return;
	}

	_selectedFile = selectedFile;

	uint8_t x, y;
	GetFileCoord(selectedFile, &x, &y);
	for (uint8_t i = x; i < x + FILE_COLUMNWIDTH; i++)
	{
		DebugScreen.SetAttribute(i, y, 0x00, 0x00); // inverse
	}
}

static void loadSnapshot(const TCHAR* fileName)
{
	FRESULT fr = mount();
	if (fr == FR_OK)
	{
		File file = _fileSystem->open(fileName, FILE_READ);
		LoadSnapshot(file);
		file.close();

		unmount();
	}
}

static bool saveSnapshot(const TCHAR* fileName)
{
	bool result = false;
	return result;
}

static int fileCompare(const void* a, const void* b)
{
	TCHAR* file1 = (TCHAR*)_buffer16K_1;
	for (int i = 0; i <= MAX_LFN; i++){
		file1[i] = tolower(((TCHAR*)a)[i]);
	}

	TCHAR* file2 = (TCHAR*)&_buffer16K_1[MAX_LFN + 2];
	for (int i = 0; i <= MAX_LFN; i++){
		file2[i] = tolower(((TCHAR*)b)[i]);
	}

	return strncmp(file1, file2, MAX_LFN + 1);
}

void FileSystemInitialize(fs::FS* fileSystem)
{
    _fileSystem = fileSystem;
}

bool saveSnapshotSetup(const char* path)
{
	return false;
}

bool saveSnapshotLoop()
{
	return false;
}

bool loadSnapshotSetup(const char* path)
{
    _rootFolder = path;
    _rootFolderLength = strlen(path);

	bool result = true;
	//saveState();

	//DebugScreen.SetPrintAttribute(0x3F10); // white on blue
	DebugScreen.Clear();
	//DebugScreen.PrintAlignRight

	//showTitle("Loading files, please wait...");

	FRESULT fr = mount();
	if (fr != FR_OK)
	{
		return false;
	}

	uint8_t maxFileCount = (DEBUG_ROWS - 1) * FILE_COLUMNS;
	_fileCount = 0;

	FF_DIR folder;
	FILINFO fileInfo;
	fr = f_opendir(&folder, (const TCHAR*) "/");
	if (fr == FR_OK)
	{
		int fileIndex = 0;
		while (fileIndex < maxFileCount)
		{
			fr = f_readdir(&folder, &fileInfo);
			if (fr != FR_OK || fileInfo.fname[0] == 0)
			{
				result = _fileCount > 0;
				break;
			}

			if (fileInfo.fattrib & AM_DIR)
			{
				continue;
			}

			// *.bin
			if (strncmp(FileExtension((TCHAR*)fileInfo.fname), ".bin", 4) != 0)
			{
				continue;
			}

			strncpy(_fileNames[fileIndex], fileInfo.fname, MAX_LFN + 1);
			_fileCount++;
			fileIndex++;
		}

		// Sort files alphabetically
		if (_fileCount > 0)
		{
			qsort(_fileNames, _fileCount, MAX_LFN + 1, fileCompare);
			Serial.printf("file count=%d\r\n", _fileCount);
		}
	}
	else
	{
		result = false;
	}

	// Unmount file system
	unmount();		

	if (result)
	{
		for (int y = 1; y < DEBUG_ROWS; y++)
		{
			DebugScreen.PrintAt(0, y, "\x97"); // ???
			DebugScreen.PrintAt(FILE_COLUMNWIDTH + 1, y, "\x97"); // ???
			DebugScreen.PrintAt(FILE_COLUMNWIDTH * 2 + 2, y, "\x97"); // ???
			DebugScreen.PrintAt(DEBUG_COLUMNS - 1, y, "\x97"); // ???
		}

		uint8_t x, y;
		for (int fileIndex = 0; fileIndex < _fileCount; fileIndex++)
		{
			GetFileCoord(fileIndex, &x, &y);
			DebugScreen.PrintAt(x, y, TruncateFileName(_fileNames[fileIndex]));
		}

		SetSelection(_selectedFile);	

		_loadingSnapshot = true;
	}

	//showTitle("Load snapshot. ENTER, ESC, \x18, \x19, \x1A, \x1B"); // ???, ???, ???, ???

	return result;
}

bool loadSnapshotLoop()
{
	if (!_loadingSnapshot)
	{
		return false;
	}

	VirtualKeyItem* virtualKeyItem = KeyboardGetNextVirtualKey();
	if (virtualKeyItem == nullptr || !virtualKeyItem->down)
	{
		return true;
	}

	uint8_t previousSelection = _selectedFile;

	switch (virtualKeyItem->vk)
	{
	case VirtualKey::VK_UP:
		if (_selectedFile > 0)
		{
			_selectedFile--;
		}
		break;

	case VirtualKey::VK_DOWN:
		if (_selectedFile < _fileCount - 1)
		{
			_selectedFile++;
		}
		break;

	case VirtualKey::VK_LEFT:
		if (_selectedFile >= DEBUG_ROWS - 1)
		{
			_selectedFile -= DEBUG_ROWS - 1;
		}
		break;

	case VirtualKey::VK_RIGHT:
		if (_selectedFile + DEBUG_ROWS <= _fileCount)
		{
			_selectedFile += DEBUG_ROWS - 1;
		}
		break;

	case VirtualKey::VK_RETURN:
	case VirtualKey::VK_KP_ENTER:
		loadSnapshot(GetFileName(_fileNames[_selectedFile]));
		_loadingSnapshot = false;
		//restoreState();
		return false;

	case VirtualKey::VK_ESCAPE:
		_loadingSnapshot = false;
		//restoreState();
		return false;

	default:
		break;
	}

	if (previousSelection == _selectedFile)
	{
		return true;
	}

	uint8_t x, y;
	GetFileCoord(previousSelection, &x, &y);
	for (uint8_t i = x; i < x + FILE_COLUMNWIDTH; i++)
	{
		DebugScreen.SetAttribute(i, y, 0xFF, 0xFF); // normal
	}

	SetSelection(_selectedFile);

	return true;
}

bool ReadFromFile(const char* fileName, uint8_t* buffer, size_t size)
{
	FRESULT fr = mount();
	if (fr != FR_OK)
	{
		return false;
	}

    bool result = false;
    File file = _fileSystem->open(fileName, FILE_READ);
    if (file)
    {
        size_t bytesRead = file.read(buffer, size);
        result = (bytesRead == size);
        file.close();
    }

	unmount();

    return result;
}