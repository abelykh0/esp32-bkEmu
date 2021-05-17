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

    this->Palette512x256 = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);
    this->Palette256x256color = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);
    this->Palette256x256bw = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);
}

void VideoController::Start(char const* modeline)
{
    this->setDrawScanlineCallback(drawScanline, this);

    this->begin();
    this->setResolution(modeline);

    // Prepare palettes
    this->InitPalette(this->Palette512x256, FORE_COLOR, BACK_COLOR);
    //this->InitPalette(this->Palette256x256color, 0xFF, FORE_COLOR, BACK_COLOR);
    //this->InitPalette(this->Palette256x256bw, 0xFF, FORE_COLOR, BACK_COLOR);
}   

void VideoController::InitPalette(uint32_t* palette, uint8_t foreColor, uint8_t backColor)
{
	for (uint8_t i = 0; i < 16; i++)
	{
		uint8_t value = i;
        uint32_t attributeValue;
		for (uint8_t bit = 0; bit < 4; bit++)
		{
            VGA_PIXELINROW(((uint8_t*)&attributeValue), 3 - bit) = this->createRawPixel(value & 0x08 ?  foreColor : backColor);
			value <<= 1;
		}
        
        *palette = attributeValue;
        palette++;
	}
}

void VideoController::InitPalette(uint32_t* palette, uint8_t* colors)
{

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

    int y = scanLine / 2;
    uint32_t* dest32 = (uint32_t*)dest;
    uint32_t* lastDest = dest32 + (2 * SCREEN_WIDTH);
    uint8_t* pixels = controller->VideoRam + (y * SCREEN_WIDTH);
    uint32_t* palette = controller->Palette512x256;

    do
    {
        uint8_t pixelsByte = *pixels;
        dest32[0] = palette[pixelsByte & 0x0F];
        dest32[1] = palette[pixelsByte >> 4];

        dest32 += 2;
        pixels++;
    } while (dest32 <= lastDest);
}
