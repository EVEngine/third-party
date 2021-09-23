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

#pragma once

// LOVE
#include "medialoader/image/FormatHandler.h"

// STL
#include <string>

namespace medialoader
{

/**
 * Interface between CompressedImageData and the ddsparse library.
 **/
class DDSHandler : public FormatHandler
{
public:
	virtual ~DDSHandler() {}

	// Implements FormatHandler.
	bool canDecode(const char* data, size_t size) override;
	DecodedImage decode(const char* data, size_t size) override;
	bool canParseCompressed(const char* data, size_t size) override;
	CompressedMemory* parseCompressed(const char* data, size_t size,
	        std::vector<CompressedSlice*> &images,
	        PixelFormat &format, bool &sRGB) override;

}; // DDSHandler

} // medialoader
