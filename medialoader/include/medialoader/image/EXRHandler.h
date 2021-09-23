
#pragma once

#include "medialoader/image/FormatHandler.h"

namespace medialoader
{

/**
 * Interface between ImageData and TinyEXR library, for decoding exr files.
 **/
class EXRHandler : public FormatHandler
{
public:

	// Implements FormatHandler.

	virtual bool canDecode(const char* data, size_t size);
	virtual bool canEncode(PixelFormat rawFormat, EncodedFormat encodedFormat);

	virtual DecodedImage decode(const char* data, size_t size);
	virtual EncodedImage encode(const DecodedImage &img, EncodedFormat format);

	virtual void freeRawPixels(unsigned char *mem);

}; // EXRHandler

} // medialoader

