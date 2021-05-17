#include "VideoController.h"
#include "fabutils.h"

// 512x256
#define BACK_COLOR 0x10
#define FORE_COLOR 0x3F

// 256x256
#define COLOR_00 0x00
#define COLOR_01 0x30
#define COLOR_10 0x0C
#define COLOR_11 0x03

// 256x256
#define BW_00 0x00
#define BW_01 0x15
#define BW_10 0x2A
#define BW_11 0x3F

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
    uint8_t colors[] = { COLOR_00, COLOR_01, COLOR_10, COLOR_11 };
    this->InitPalette(this->Palette256x256color, colors);
    uint8_t bw[] = { BW_00, BW_01, BW_10, BW_11 };
    this->InitPalette(this->Palette256x256bw, bw);
}   

void VideoController::InitPalette(uint32_t* palette, uint8_t foreColor, uint8_t backColor)
{
	for (uint8_t i = 0; i < 16; i++)
	{
		uint8_t value = i;
        uint32_t attributeValue;
		for (uint8_t bit = 0; bit < 4; bit++)
		{
            VGA_PIXELINROW(((uint8_t*)&attributeValue), bit) = this->createRawPixel(value & 0x01 ?  foreColor : backColor);
			value >>= 1;
		}
        
        *palette = attributeValue;
        palette++;
	}
}

void VideoController::InitPalette(uint32_t* palette, uint8_t* colors)
{
	for (uint8_t i = 0; i < 16; i++)
	{
		uint8_t value = i;
        uint32_t attributeValue;
        uint8_t color = colors[value & 0x03];
        VGA_PIXELINROW(((uint8_t*)&attributeValue), 0) = this->createRawPixel(color);
        VGA_PIXELINROW(((uint8_t*)&attributeValue), 1) = this->createRawPixel(color);

        value >>= 2;
        color = colors[value & 0x03];
        VGA_PIXELINROW(((uint8_t*)&attributeValue), 2) = this->createRawPixel(color);
        VGA_PIXELINROW(((uint8_t*)&attributeValue), 3) = this->createRawPixel(color);

        *palette = attributeValue;
        palette++;
	}
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
    uint32_t* palette;
    if (*controller->ScreenMode == 0)
    {
        palette = controller->Palette512x256;
    }
    else
    {
        palette = controller->Palette256x256color;
    }

    do
    {
        uint8_t pixelsByte = *pixels;
        dest32[0] = palette[pixelsByte & 0x0F];
        dest32[1] = palette[pixelsByte >> 4];

        dest32 += 2;
        pixels++;
    } while (dest32 < lastDest);
}
