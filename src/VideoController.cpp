#include "VideoController.h"
#include "fabutils.h"

#define BACK_COLOR 0x10
#define FORE_COLOR 0x3F

extern "C" void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine);

void VideoController::Initialize(bkEnvironment* environment)
{
    this->ScreenMode = environment->GetPointer(0x0020);
    this->ScreenInversion = environment->GetPointer(0x0021);
    this->VideoRam = environment->GetPointer(0x4000);
    this->Scroll = environment->GetPointer(0xFFB4);
    this->ExtendedMemory = environment->GetPointer(0xFFB5);
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
