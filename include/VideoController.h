#ifndef __VIDEOCONTROLLER__
#define __VIDEOCONTROLLER__

#include <memory>
#include <map>
#include "fabgl.h"
#include "Settings.h"
#include "bkEnvironment.h"

class VideoController : public fabgl::VGADirectController
{
public:
    volatile uint32_t Frames = 0;

    uint8_t* ScreenMode;      // 0 - 512x256, FF - 256x256
    uint8_t* ScreenInversion; // 0 - off, FF - on
    uint8_t* ExtendedMemory;  // 0x02 - off, 0x00 - on
    uint8_t* Scroll;          
    uint8_t* VideoRam;
    bool UseColorPalette = true;
    uint32_t* Palette512x256;
    uint32_t* Palette256x256color;
    uint32_t* Palette256x256bw;
    uint32_t* PaletteText;

    uint8_t* BottomCharacters;

    void Initialize(bkEnvironment* environment);

    void Start(char const* modeline);
    uint8_t IRAM_ATTR createRawPixel(uint8_t color);

private:
    void InitPalette(uint32_t* palette, uint8_t backColor, uint8_t foreColor);
    void InitPalette(uint32_t* palette, uint8_t* colors);
};

#endif
