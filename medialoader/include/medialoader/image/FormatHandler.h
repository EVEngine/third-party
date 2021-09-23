
#pragma once

#include "medialoader/image/pixelformat.h"
#include "medialoader/image/CompressedSlice.h"
#include <cstddef>
#include <vector>

namespace medialoader
{


/**
 * Base class for all ImageData and CompressedImageData encoder/decoder library
 * interfaces. We inherit from love::Object to take advantage of ref coounting.
 **/
class FormatHandler 
{
public:

	enum EncodedFormat
	{
		ENCODED_TGA,
		ENCODED_PNG,
		ENCODED_MAX_ENUM
	};

	// Raw RGBA pixel data.
	struct DecodedImage
	{
		PixelFormat format = PIXELFORMAT_RGBA8;
		int width   = 0;
		int height  = 0;
		size_t size = 0;
		unsigned char *data = nullptr;
	};

	// Pixel data encoded in a particular format.
	struct EncodedImage
	{
		size_t size = 0;
		unsigned char *data = nullptr;
	};

	/**
	 * The default constructor is called when the Image module is initialized.
	 **/
	FormatHandler();
	virtual ~FormatHandler();

	/**
	 * Whether this format handler can decode the given Data into raw pixels.
	 **/
	virtual bool canDecode(const char* data, size_t size);

	/**
	 * Whether this format handler can encode raw pixels to a particular format.
	 **/
	virtual bool canEncode(PixelFormat rawFormat, EncodedFormat encodedFormat);

	/**
	 * Decodes an image from its encoded form into raw pixel data.
	 **/
	virtual DecodedImage decode(const char* data, size_t size);

	/**
	 * Encodes an image from raw pixel data into a particular format.
	 **/
	virtual EncodedImage encode(const DecodedImage &img, EncodedFormat format);

	/**
	 * Whether this format handler can parse the given Data into a
	 * CompressedImageData object.
	 **/
	virtual bool canParseCompressed(const char* data, size_t size);

	/**
	 * Parses compressed image data into a list of sub-images and returns a
	 * single block of memory containing all the images.
	 *
	 * @param[in] filedata The data to parse.
	 * @param[out] images The list of sub-images generated. Byte data is a
	 *             pointer to the returned data.
	 * @param[out] format The format of the Compressed Data.
	 * @param[out] sRGB Whether the texture is sRGB-encoded.
	 *
	 * @return The single block of memory containing the parsed images.
	 **/
	virtual CompressedMemory* parseCompressed(const char* data, size_t size,
	        std::vector<CompressedSlice*> &images,
	        PixelFormat &format, bool &sRGB);

	/**
	 * Frees raw pixel memory allocated by the format handler.
	 **/
	virtual void freeRawPixels(unsigned char *mem);

}; // FormatHandler

} // medialoader
