#pragma once
#include "medialoader/sound/Decoder.h"

#include "dr_flac/dr_flac.h"
#include <string.h>

namespace medialoader {

class FLACDecoder : public Decoder
{
public:
	FLACDecoder(const char* data, size_t size, int bufferSize);
	~FLACDecoder();

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
	drflac *flac;
}; // Decoder

} // medialoader
