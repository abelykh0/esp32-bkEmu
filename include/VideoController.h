#ifndef __VIDEOCONTROLLER__
#define __VIDEOCONTROLLER__

#include <memory>
#include <map>
#include "fabgl.h"
#include "Settings.h"
#include "bkEnvironment.h"

using namespace fabgl;

class VideoController : public VGADirectController
{
public:
    volatile uint32_t Frames = 0;

    uint8_t _mode = 0; // 0 - BK, 1 - text

    // BK Mode
    uint8_t* ScreenMode;      // 0 - 512x256, FF - 256x256
    uint8_t* ExtendedMemory;  // 0x02 - off, 0x00 - on
    uint8_t* Scroll;          
    uint8_t* VideoRam;
    bool UseColorPalette = true;
    uint32_t* Palette512x256;
    uint32_t* Palette256x256color;
    uint32_t* Palette256x256bw;

    // Text mode
    uint8_t cursor_x = 0;
    uint8_t cursor_y = 0;
    uint8_t Characters[TEXT_WIDTH * TEXT_HEIGHT];
    uint32_t* _normalAttribute;
    uint32_t* _inversedAttribute;
    uint32_t** Attributes;
    void SetAttribute(uint8_t x, uint8_t y, uint8_t foreColor, uint8_t backColor);

    void Initialize(bkEnvironment* environment);
    void Start(char const* modeline);

	void Print(const char* str);
    void print(char* str, uint8_t foreColor, uint8_t backColor);
	void SetCursorPosition(uint8_t x, uint8_t y);

    uint8_t IRAM_ATTR createRawPixel(uint8_t color);

private:
    void InitPalette(uint32_t* palette, uint8_t backColor, uint8_t foreColor);
    void InitPalette(uint32_t* palette, uint8_t* colors);
    void cursorNext();
    void print(char* str);
    void print(const char* str, uint8_t foreColor, uint8_t backColor);
    void printChar(uint16_t x, uint16_t y, uint16_t ch);
    void printChar(uint16_t x, uint16_t y, uint16_t ch, uint8_t foreColor, uint8_t backColor);
};

#endif
