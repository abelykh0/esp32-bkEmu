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
    uint8_t IRAM_ATTR createRawPixel(uint8_t color);

    VideoController();

    void Start(char const* modeline);
private:
};

#endif