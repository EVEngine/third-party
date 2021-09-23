#pragma once

#include "medialoader/image/FormatHandler.h"

namespace medialoader
{

/**
 * Interface between ImageData and the stb_image library, for decoding JPEG,
 * TGA, and BMP images.
 *
 * We could use stb_image to decode PNG as well, but performance and
 * comprehensive format support is lacking compared to some alternatives.
 **/
class STBHandler final : public FormatHandler
{
public:

	// Implements FormatHandler.

	bool canDecode(const char* data, size_t size) override;
	bool canEncode(PixelFormat rawFormat, EncodedFormat encodedFormat) override;

	DecodedImage decode(const char* data, size_t size) override;
	EncodedImage encode(const DecodedImage &img, EncodedFormat format) override;

	void freeRawPixels(unsigned char *mem) override;

}; // STBHandler

} // medialoader

