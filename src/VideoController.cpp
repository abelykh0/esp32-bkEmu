#include "VideoController.h"
#include "fabutils.h"
#include "monitor.h"

#define FONT_HEIGHT 10
#define TEXT_HEIGHT 3

#define BK_WIDTH  64
#define BK_HEIGHT 256
#define BORDER_HEIGHT 16
#define BORDER_WIDTH ((SCREEN_WIDTH - BK_WIDTH) / 2)

// 512x256
#define BACK_COLOR 0x10
#define FORE_COLOR 0x2A

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

static uint8_t _bottomCharacters[SCREEN_WIDTH * 3];

extern "C" void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine);

void VideoController::Initialize(bkEnvironment* environment)
{
    this->Characters = _bottomCharacters;

    this->ScreenMode = environment->GetPointer(0x0020);
    this->ScreenInversion = environment->GetPointer(0x0021);
    this->VideoRam = environment->GetPointer(0x4000);
    this->Scroll = environment->GetPointer(0xFFB4);
    this->ExtendedMemory = environment->GetPointer(0xFFB5);

    this->PaletteText = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);
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
    this->InitPalette(this->PaletteText, BACK_COLOR, FORE_COLOR);
    this->InitPalette(this->Palette512x256, BW_00, BW_11);
    uint8_t colors[] = { COLOR_00, COLOR_01, COLOR_10, COLOR_11 };
    this->InitPalette(this->Palette256x256color, colors);
    uint8_t bw[] = { BW_00, BW_01, BW_10, BW_11 };
    this->InitPalette(this->Palette256x256bw, bw);

    for (int i = 0; i < SCREEN_WIDTH * 3; i++)
    {
        _bottomCharacters[i] = ' ';
    }
}   

void VideoController::InitPalette(uint32_t* palette, uint8_t backColor, uint8_t foreColor)
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

void VideoController::Print(const char* str)
{
	this->print((char*)str);
}


void VideoController::printChar(uint16_t x, uint16_t y, uint16_t ch)
{
	VideoController::printChar(x, y, ch, 0xFF, 0xFF);
}
void VideoController::printChar(uint16_t x, uint16_t y, uint16_t ch, uint8_t foreColor, uint8_t backColor)
{
	if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
	{
		// Invalid
		return;
	}

	int offset = y * SCREEN_WIDTH + x;
	this->Characters[offset] = ch;
    //this->SetAttribute(x, y, foreColor, backColor);
}
void VideoController::print(const char* str, uint8_t foreColor, uint8_t backColor)
{
	print((char*)str, foreColor, backColor);
}

void VideoController::print(char* str)
{
	this->print(str, 0xFF, 0xFF);
}
void VideoController::print(char* str, uint8_t foreColor, uint8_t backColor)
{
    while (*str)
    {
    	printChar(cursor_x, cursor_y, *str++, foreColor, backColor);
    	cursorNext();
    }
}

void VideoController::cursorNext()
{
    uint8_t x = cursor_x;
    uint8_t y = cursor_y;
    if (x < SCREEN_WIDTH - 1)
    {
        x++;
    }
    else
    {
        if (y < SCREEN_HEIGHT - 1)
        {
            x = 0;
            y++;
        }
    }

    this->SetCursorPosition(x, y);
}

void VideoController::SetCursorPosition(uint8_t x, uint8_t y)
{
	this->cursor_x = x;
	this->cursor_y = y;
	if (this->cursor_x >= SCREEN_WIDTH)
	{
		this->cursor_x = SCREEN_WIDTH - 1;
	}
	if (this->cursor_y >= TEXT_HEIGHT)
	{
		this->cursor_y = TEXT_HEIGHT - 1;
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

    uint8_t borderPixel = controller->createRawPixel(BACK_COLOR);

    // Border on top
    if (scanLine < BORDER_HEIGHT * 2)
    {
        memset(dest, borderPixel, SCREEN_WIDTH * 8);
        return;
    }

    // Bottom characters
    if (scanLine >= (BORDER_HEIGHT + BK_HEIGHT) * 2)
    {
        scanLine -= (BORDER_HEIGHT + BK_HEIGHT) * 2;

        if (scanLine < 2)
        {
            memset(dest, borderPixel, SCREEN_WIDTH * 8);
        }
        else
        {
            //memset(dest, borderPixel, SCREEN_WIDTH * 8);
            //return;

            scanLine -= 2;
            int y = scanLine / FONT_HEIGHT;
            int fontRow = scanLine % FONT_HEIGHT;
            int startCoord = y * SCREEN_WIDTH;

            uint8_t* characters = controller->Characters + startCoord;
            uint32_t* palette = controller->PaletteText;
            uint32_t* dest32 = (uint32_t*)dest;
            uint32_t* lastDest = dest32 + (SCREEN_WIDTH * 2);
            uint8_t* fontData = (uint8_t*)monitor + 0x137E + fontRow;
            uint8_t character;
            uint8_t fontPixels;

            do
            {
                character = *characters;
                fontPixels = fontData[character * FONT_HEIGHT];
                dest32[0] = palette[fontPixels & 0x0F];
                dest32[1] = palette[fontPixels >> 4];

                dest32 += 2;
                characters++;
            } while (dest32 < lastDest);
        }

        return;
    }

    int y = scanLine / 2;

    // Left border
    memset(dest, borderPixel, BORDER_WIDTH);
    dest += BORDER_WIDTH * 8;

    y -= BORDER_HEIGHT;

    // scroll
    y = (uint8_t)(y + *controller->Scroll - 0330);    

    uint32_t* dest32 = (uint32_t*)dest;
    uint32_t* lastDest = dest32 + (2 * BK_WIDTH);
    uint8_t* pixels = controller->VideoRam + (y * BK_WIDTH);
    uint32_t* palette;
    if (*controller->ScreenMode == 0)
    {
        palette = controller->Palette512x256;
    }
    else
    {
        palette = controller->UseColorPalette ? controller->Palette256x256color : controller->Palette256x256bw;
    }

    do
    {
        uint8_t pixelsByte = *pixels;
        dest32[0] = palette[pixelsByte & 0x0F];
        dest32[1] = palette[pixelsByte >> 4];

        dest32 += 2;
        pixels++;
    } while (dest32 < lastDest);
    
    // Right border
    memset(dest32, borderPixel, BORDER_WIDTH);
}
