/**
 * Copyright (c) 2006-2021 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "medialoader/image/pixelformat.h"

namespace medialoader
{

bool isPixelFormatCompressed(PixelFormat format)
{
	int iformat = (int) format;
	return iformat >= (int) PIXELFORMAT_DXT1 && iformat < (int) PIXELFORMAT_MAX_ENUM;
}

bool isPixelFormatDepthStencil(PixelFormat format)
{
	int iformat = (int) format;
	return iformat >= (int) PIXELFORMAT_STENCIL8 && iformat <= (int) PIXELFORMAT_DEPTH32F_STENCIL8;
}

bool isPixelFormatDepth(PixelFormat format)
{
	int iformat = (int) format;
	return iformat >= (int) PIXELFORMAT_DEPTH16 && iformat <= (int) PIXELFORMAT_DEPTH32F_STENCIL8;
}

bool isPixelFormatStencil(PixelFormat format)
{
	return format == PIXELFORMAT_STENCIL8 || format == PIXELFORMAT_DEPTH24_STENCIL8 || format == PIXELFORMAT_DEPTH32F_STENCIL8;
}

int getPixelFormatSize(PixelFormat format)
{
	switch (format)
	{
	case PIXELFORMAT_R8:
	case PIXELFORMAT_STENCIL8:
		return 1;
	case PIXELFORMAT_RG8:
	case PIXELFORMAT_R16:
	case PIXELFORMAT_R16F:
	case PIXELFORMAT_LA8:
	case PIXELFORMAT_RGBA4:
	case PIXELFORMAT_RGB5A1:
	case PIXELFORMAT_RGB565:
	case PIXELFORMAT_DEPTH16:
		return 2;
	case PIXELFORMAT_RGBA8:
	case PIXELFORMAT_sRGBA8:
	case PIXELFORMAT_RG16:
	case PIXELFORMAT_RG16F:
	case PIXELFORMAT_R32F:
	case PIXELFORMAT_RGB10A2:
	case PIXELFORMAT_RG11B10F:
	case PIXELFORMAT_DEPTH24:
	case PIXELFORMAT_DEPTH32F:
	case PIXELFORMAT_DEPTH24_STENCIL8:
		return 4;
	case PIXELFORMAT_RGBA16:
	case PIXELFORMAT_RGBA16F:
	case PIXELFORMAT_RG32F:
	case PIXELFORMAT_DEPTH32F_STENCIL8:
		return 8;
	case PIXELFORMAT_RGBA32F:
		return 16;
	default:
		// TODO: compressed formats
		return 0;
	}
}

int getPixelFormatColorComponents(PixelFormat format)
{
	switch (format)
	{
	case PIXELFORMAT_R8:
	case PIXELFORMAT_R16:
	case PIXELFORMAT_R16F:
	case PIXELFORMAT_R32F:
		return 1;
	case PIXELFORMAT_RG8:
	case PIXELFORMAT_RG16:
	case PIXELFORMAT_RG16F:
	case PIXELFORMAT_RG32F:
	case PIXELFORMAT_LA8:
		return 2;
	case PIXELFORMAT_RGB565:
	case PIXELFORMAT_RG11B10F:
		return 3;
	case PIXELFORMAT_RGBA8:
	case PIXELFORMAT_sRGBA8:
	case PIXELFORMAT_RGBA16:
	case PIXELFORMAT_RGBA16F:
	case PIXELFORMAT_RGBA32F:
	case PIXELFORMAT_RGBA4:
	case PIXELFORMAT_RGB5A1:
	case PIXELFORMAT_RGB10A2:
		return 4;
	default:
		return 0;
	}
}

} // medialoader
