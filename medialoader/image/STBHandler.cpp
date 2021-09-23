
#include "medialoader/image/STBHandler.h"
#include "medialoader/image/Color.h"
#include "medialoader/Exception.h"


static void loveSTBIAssert(bool test, const char *teststr)
{
	if (!test)
		throw medialoader::Exception("Could not decode image (stb_image assertion '%s' failed)", teststr);
}


// stb_image
#define STBI_ONLY_JPEG
// #define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_ONLY_HDR
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(A) loveSTBIAssert((A), #A)
#include "stb/stb_image.h"

// C
#include <cstdlib>


namespace medialoader
{

static_assert(sizeof(Color32) == 4, "sizeof(Color32) must equal 4 bytes!");

bool STBHandler::canDecode(const char* data, size_t size)
{
	int w = 0;
	int h = 0;
	int comp = 0;

	int status = stbi_info_from_memory((const stbi_uc *) data,
	                                   (int) size, &w, &h, &comp);

	return status == 1 && w > 0 && h > 0;
}

bool STBHandler::canEncode(PixelFormat rawFormat, EncodedFormat encodedFormat)
{
	return encodedFormat == ENCODED_TGA && rawFormat == PIXELFORMAT_RGBA8;
}

FormatHandler::DecodedImage STBHandler::decode(const char* data, size_t size)
{
	DecodedImage img;

	const stbi_uc *buffer = (const stbi_uc *) data;
	int bufferlen = (int) size;
	int comp = 0;

	if (stbi_is_hdr_from_memory(buffer, bufferlen))
	{
		img.data = (unsigned char *) stbi_loadf_from_memory(buffer, bufferlen, &img.width, &img.height, &comp, 4);
		img.size = img.width * img.height * 4 * sizeof(float);
		img.format = PIXELFORMAT_RGBA32F;
	}
	else
	{
		img.data = stbi_load_from_memory(buffer, bufferlen, &img.width, &img.height, &comp, 4);
		img.size = img.width * img.height * 4;
		img.format = PIXELFORMAT_RGBA8;
	}

	if (img.data == nullptr || img.width <= 0 || img.height <= 0)
	{
		const char *err = stbi_failure_reason();
		if (err == nullptr)
			err = "unknown error";
		throw Exception("Could not decode image with stb_image (%s).", err);
	}

	return img;
}

FormatHandler::EncodedImage STBHandler::encode(const DecodedImage &img, EncodedFormat encodedFormat)
{
	if (!canEncode(img.format, encodedFormat))
		throw Exception("Invalid format.");

	// We don't actually use stb_image for encoding, but this code is small
	// enough that it might as well stay here.

	EncodedImage encimg;

	const size_t headerlen = 18;
	const size_t bpp = 4;

	encimg.size = (img.width * img.height * bpp) + headerlen;

	// We need to use malloc because we use stb_image_free (which uses free())
	// as our custom free() function, which is called by the ImageData after
	// encode() is complete.
	// stb_image's source code is compiled with this source, so calling malloc()
	// directly is fine.
	encimg.data = (unsigned char *) malloc(encimg.size);

	if (encimg.data == nullptr)
		throw Exception("Out of memory.");

	// here's the header for the Targa file format.
	encimg.data[0]  = 0; // ID field size
	encimg.data[1]  = 0; // colormap type
	encimg.data[2]  = 2; // image type
	encimg.data[3]  = encimg.data[4] = 0; // colormap start
	encimg.data[5]  = encimg.data[6] = 0; // colormap length
	encimg.data[7]  = 32; // colormap bits
	encimg.data[8]  = encimg.data[9] = 0; // x origin
	encimg.data[10] = encimg.data[11] = 0; // y origin
	// Targa is little endian, so:
	encimg.data[12] = img.width & 255; // least significant byte of width
	encimg.data[13] = img.width >> 8; // most significant byte of width
	encimg.data[14] = img.height & 255; // least significant byte of height
	encimg.data[15] = img.height >> 8; // most significant byte of height
	encimg.data[16] = bpp * 8; // bits per pixel
	encimg.data[17] = 0x20; // descriptor bits (flip bits: 0x10 horizontal, 0x20 vertical)

	// header done. write the pixel data to TGA:
	memcpy(encimg.data + headerlen, img.data, img.width * img.height * bpp);

	// convert the pixels from RGBA to BGRA.
	Color32 *encodedpixels = (Color32 *) (encimg.data + headerlen);
	for (int y = 0; y < img.height; y++)
	{
		for (int x = 0; x < img.width; x++)
		{
			unsigned char r = encodedpixels[y * img.width + x].r;
			unsigned char b = encodedpixels[y * img.width + x].b;
			encodedpixels[y * img.width + x].r = b;
			encodedpixels[y * img.width + x].b = r;
		}
	}

	return encimg;
}

void STBHandler::freeRawPixels(unsigned char *mem)
{
	// The STB decoder gave memory allocated directly by stb_image to the
	// ImageData, so we use stb_image_free to delete it.
	stbi_image_free(mem);
}

}
