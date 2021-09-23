#pragma once

#include "medialoader/sound/Decoder.h"

#include "Wuff/wuff.h"

namespace medialoader {

// Struct for handling data
struct WaveFile
{
	char *data;
	size_t size;
	size_t offset;
};

class WaveDecoder : public Decoder
{
public:

	WaveDecoder(const char* data, size_t size, int bufferSize);
	virtual ~WaveDecoder();

	static bool accepts(const std::string &ext);

	Decoder *clone();
	int decode();
	bool seek(double s);
	bool rewind();
	bool isSeekable();
	int getChannelCount() const;
	int getBitDepth() const;
	int getSampleRate() const;
	double getDuration();

private:

	WaveFile dataFile;
	wuff_handle *handle;
	wuff_info info;

}; // WaveDecoder

} // medialoader
