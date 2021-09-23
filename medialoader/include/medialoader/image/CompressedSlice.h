
#pragma once

// LOVE
#include "medialoader/image/pixelformat.h"
#include <cstdint>
#include <cstddef>

namespace medialoader
{

class CompressedMemory
{
public:
	CompressedMemory(size_t size);
	virtual ~CompressedMemory();

	uint8_t *data;
	size_t size;

}; 

// Compressed image data can have multiple mipmap levels, each represented by a
// sub-image.
class CompressedSlice
{
public:
	CompressedSlice(PixelFormat format, int width, int height, CompressedMemory *memory, size_t offset, size_t size);
	CompressedSlice(const CompressedSlice &slice);
	virtual ~CompressedSlice();

	CompressedSlice *clone() const;
	void *getData() const { return memory->data + offset; }
	size_t getSize() const { return dataSize; }
	bool isSRGB() const { return sRGB; }
	size_t getOffset() const { return offset; }

private:

	CompressedMemory* memory;
	PixelFormat format;
	int width, height;
	size_t offset;
	size_t dataSize;
	bool sRGB;

}; // CompressedSlice

} // medialoader
