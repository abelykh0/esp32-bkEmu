/*

#include "bkscreen.h"
#include <string.h>
#include "bkEmu.h"

//   0020 Screen mode 0 - 512x256, FF - 256x256
//   0021 Screen inversion 0 - off, FF - on

// 0..7 Shift
// 9 Extended memory 0 - on, 1 - off
uint16_t port0177664;

namespace bk
{

BkScreen::BkScreen(VideoSettings* settings) :
		BkScreen(settings, 0,
				settings->Timing->video_end_line
						- settings->Timing->video_start_line)
{
}

BkScreen::BkScreen(VideoSettings* settings, uint16_t startLine, uint16_t height)
	: Screen(settings, startLine, height)
{
	port0177664 = 01330;

	this->_hResolution = 560;
	this->_horizontalBorder = (this->_hResolution - this->_hResolutionNoBorder) / 2;
	this->_attributeCount = 0;
}

void BkScreen::InvertColor()
{
}

//__attribute__((section(".ramcode")))
Rasterizer::RasterInfo BkScreen::rasterize(
		unsigned cycles_per_pixel, unsigned line_number, Pixel *target)
{
	uint8_t borderColor = *this->Settings->BorderColor;
	bool isNarrow = (RamBuffer[0x0020] == 0);
	uint8_t divider = (isNarrow ? 1 : 2);
	uint16_t scaledLine = (line_number - this->_startLine) / 2;
	uint16_t scaledResolution = this->_hResolution / divider;

	Rasterizer::RasterInfo result;
	result.offset = 125;
	result.cycles_per_pixel = cycles_per_pixel * divider;
	result.length = scaledResolution;

	uint16_t bottomBorderStart;
	if ((port0177664 & 0x0200) == 0)
	{
		// Extended memory on
		bottomBorderStart = this->_verticalBorder + 7 * 8;
	}
	else
	{
		// Standard mode
		bottomBorderStart = this->_vResolution - this->_verticalBorder;
	}

	if (scaledLine == 0)
	{
		this->_frames++;

		uint32_t fill = borderColor << 8 | borderColor;
		fill |= fill << 16;
		for (uint32_t* ptr = (uint32_t*)target; ptr <= (uint32_t*)target + scaledResolution / 4; ptr++)
		{
			*ptr = fill;
		}
		result.repeat_lines = this->_verticalBorder * 2 - 1;
	}
	else if (scaledLine == bottomBorderStart)
	{
		uint32_t fill = borderColor << 8 | borderColor;
		fill |= fill << 16;
		for (uint32_t* ptr = (uint32_t*)target; ptr <= (uint32_t*)target + scaledResolution / 4; ptr++)
		{
			*ptr = fill;
		}
		result.repeat_lines = (this->_vResolution - bottomBorderStart) * 2 - 1;
	}
	else
	{
		uint8_t vline = scaledLine - this->_verticalBorder;

		// scroll
		vline += (port0177664 - 0330);

		uint32_t* bitmap = (uint32_t*)this->GetPixelPointer(vline);
		uint8_t* dest = &target[this->_horizontalBorder / divider];
		if (!isNarrow)
		{
			// don't know why we need this
			dest++;
		}

		if (isNarrow)
		{
			rast::unpack_1bpp_impl(bitmap, ((const uint8_t*)this->Settings->Attributes) + 4, dest, 16);
		}
		else
		{
			for (int x = 0; x < 16; x++)
			{
				Draw2Color(*bitmap, (uint8_t*)this->Settings->Attributes, dest);
				dest += 16;
				bitmap++;
			}
		}

		result.repeat_lines = 1;
	}

	return result;
}
}
*/