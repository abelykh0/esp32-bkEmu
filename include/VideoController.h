#ifndef __VIDEOCONTROLLER__
#define __VIDEOCONTROLLER__

#include <memory>
#include <map>
#include "fabgl.h"
#include "Settings.h"

using namespace std;

class VideoController : public fabgl::VGADirectController
{
public:
    volatile uint32_t Frames = 0;
    uint8_t ScreenMode = 0;      // 0 - 512x256, FF - 256x256
    uint8_t ScreenInversion = 0; // 0 - off, FF - on
    uint8_t Scroll = 0xD8;
    uint8_t ExtendedMemory = 0x02; // 0x02 - off, 0x00 - on
    uint8_t* VideoRam;

    VideoController();

    void Start(char const* modeline);
    uint8_t IRAM_ATTR createRawPixel(uint8_t color);
private:
};

#endif