#include "VideoController.h"
#include "fabutils.h"
#include "bkIO.h"

#define BACK_COLOR 0x10
#define FORE_COLOR 0x3F

// 0..7 Shift
// 9 Extended memory 0 - on, 1 - off
uint16_t port0177664;

extern uint8_t* IRAM_ATTR GetPixelPointer(uint8_t* pixels, uint16_t line);
extern "C" void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine);

VideoController::VideoController()
{
}

void VideoController::Start(char const* modeline)
{
    this->setDrawScanlineCallback(drawScanline, this);

    this->begin();
    this->setResolution(modeline);
}   

uint8_t IRAM_ATTR VideoController::createRawPixel(uint8_t color)
{
    // HACK: should call createRawPixel() instead
    return this->m_HVSync | color;
}

void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine)
{
    auto controller = static_cast<VideoController*>(arg);
    if (scanLine == 0)
    {
        controller->Frames++;
    }

    memset(dest, controller->createRawPixel(0x30), 512);
}
