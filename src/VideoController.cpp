#include "VideoController.h"
#include "fabutils.h"
#include "monitor.h"

#define BK_WIDTH  64
#define BK_HEIGHT 256
#define BORDER_HEIGHT 2
#define BORDER_WIDTH ((SCREEN_WIDTH - BK_WIDTH) / 2)

// BK Palette colors

#define COLOR_00 0x00
#define COLOR_01 0x30
#define COLOR_10 0x0C
#define COLOR_11 0x03

#define BW_00 0x00
#define BW_01 0x15
#define BW_10 0x2A
#define BW_11 0x3F

// Text Mode

#define BACK_COLOR 0x10
#define FORE_COLOR 0x2A
#define FONT_HEIGHT 10

static const uint8_t screenModeZero = 0;
static const uint8_t noScroll = 0330;
static const uint8_t noExtendedMemory = 0x02;

extern "C" void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine);

void VideoController::Initialize(bkEnvironment* environment)
{
    this->_environment = environment;

    this->ScreenMode = environment->GetPointer(0x0020);
    this->VideoRam = environment->GetPointer(0x4000);
    this->Scroll = environment->GetPointer(0xFFB4);
    this->ExtendedMemory = environment->GetPointer(0xFFB5);

    this->Palette512x256 = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);
    this->Palette256x256color = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);
    this->Palette256x256bw = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);

    this->Attributes = (uint32_t**)heap_caps_malloc(TEXT_WIDTH * TEXT_HEIGHT * 4, MALLOC_CAP_32BIT);
    this->_normalAttribute = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);
    this->_inversedAttribute = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);
    for (int y = 0; y < TEXT_HEIGHT; y++)
    {
        for (int x = 0; x < TEXT_WIDTH; x++)
        {
            this->Characters[y * TEXT_WIDTH + x] = ' ';
            this->Attributes[y * TEXT_WIDTH + x] = this->_normalAttribute;
        }
    }
}

void VideoController::ShowScreenshot(uint8_t* screenShot)
{
    this->ScreenMode = (uint8_t*)&screenModeZero;
    this->Scroll = (uint8_t*)&noScroll;
    this->ExtendedMemory = (uint8_t*)&noExtendedMemory;
    this->_palette512x256bw = this->Palette512x256;
    this->Palette512x256 = this->_normalAttribute;
    this->VideoRam = screenShot;
}

void VideoController::Resume()
{
    this->ScreenMode = this->_environment->GetPointer(0x0020);
    this->Scroll = this->_environment->GetPointer(0xFFB4);
    this->ExtendedMemory = this->_environment->GetPointer(0xFFB5);
    this->Palette512x256 = this->_palette512x256bw;
    this->VideoRam = this->_environment->GetPointer(0x4000);
}

void VideoController::Start(char const* modeline)
{
    this->setDrawScanlineCallback(drawScanline, this);

    this->begin();
    this->setResolution(modeline);

    // Prepare palettes
    this->InitPalette(this->Palette512x256, BW_00, BW_11);
    uint8_t colors[] = { COLOR_00, COLOR_01, COLOR_10, COLOR_11 };
    this->InitPalette(this->Palette256x256color, colors);
    uint8_t bw[] = { BW_00, BW_01, BW_10, BW_11 };
    this->InitPalette(this->Palette256x256bw, bw);

    // Text mode
    this->InitPalette(this->_normalAttribute, BACK_COLOR, FORE_COLOR);
    this->InitPalette(this->_inversedAttribute, FORE_COLOR, BACK_COLOR);
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
	if (x >= TEXT_WIDTH || y >= TEXT_HEIGHT)
	{
		// Invalid
		return;
	}

	int offset = y * TEXT_WIDTH + x;
	this->Characters[offset] = ch;
    this->SetAttribute(x, y, foreColor, backColor);
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
    if (x < TEXT_WIDTH - 1)
    {
        x++;
    }
    else
    {
        if (y < TEXT_HEIGHT - 1)
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
	if (this->cursor_x >= TEXT_WIDTH)
	{
		this->cursor_x = TEXT_WIDTH - 1;
	}
	if (this->cursor_y >= TEXT_HEIGHT)
	{
		this->cursor_y = TEXT_HEIGHT - 1;
	}
}

void VideoController::SetAttribute(uint8_t x, uint8_t y, uint8_t foreColor, uint8_t backColor)
{
    uint32_t* attribute;
    uint16_t colors = foreColor << 8 | backColor;

    if (colors == 0xFFFF)
    {
        attribute = this->_normalAttribute;
    }
    else
    {
        attribute = this->_inversedAttribute;
    }

    this->Attributes[y * TEXT_WIDTH + x] = attribute;
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

    // First 8 lines
    if (scanLine < 16)
    {
        memset(dest, borderPixel, SCREEN_WIDTH * 8);
        return;
    }

    // Left border
    // Saving time
    //memset(dest, borderPixel, BORDER_WIDTH * 8);

    uint32_t* dest32 = (uint32_t*)dest + (BORDER_WIDTH * 2);
    scanLine -= 16;
    int y = scanLine / 2;

    if (controller->_mode == 1 || scanLine < BORDER_HEIGHT * 2
        || scanLine >= (BORDER_HEIGHT + ((((*controller->ExtendedMemory) & 0x02) != 0) ? BK_HEIGHT : 64)) * 2)
    {
        // Text mode

        int fontRow = y % FONT_HEIGHT;
        y /= FONT_HEIGHT;
        int startCoord = y * TEXT_WIDTH;

        uint8_t* characters = controller->Characters + startCoord;
        uint32_t** attributes = controller->Attributes + startCoord;
        uint32_t** lastAttribute = attributes + TEXT_WIDTH - 1;
        uint8_t* fontData = (uint8_t*)monitor + 0x137E + fontRow; // 8x10 font from BK ROM

        do
        {
            uint8_t character = *characters;
            uint8_t fontPixels = fontData[character * FONT_HEIGHT];
            uint32_t* attribute = *attributes;
            dest32[0] = attribute[fontPixels & 0x0F];
            dest32[1] = attribute[fontPixels >> 4];

            dest32 += 2;
            attributes++;
            characters++;
        } while (attributes <= lastAttribute);
    }
    else
    {
        // BK Mode

        y -= BORDER_HEIGHT;

        // scroll
        y = (uint8_t)(y + *controller->Scroll - 0330);    

        uint32_t* lastDest = dest32 + (2 * BK_WIDTH) - 1;
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
        } while (dest32 <= lastDest);
    }
    
    // Right border
    // Saving time
    //memset(dest32, borderPixel, BORDER_WIDTH * 8);
}
