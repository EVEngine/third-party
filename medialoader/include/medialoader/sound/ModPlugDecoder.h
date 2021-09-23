#pragma once

#include "medialoader/sound/Decoder.h"
#include <libmodplug/modplug.h>

namespace medialoader {

class ModPlugDecoder : public Decoder
{
public:

	ModPlugDecoder(const char* data, size_t size, int bufferSize);
	virtual ~ModPlugDecoder();

	static bool accepts(const std::string &ext);

	Decoder *clone();
	int decode();
	bool seek(double s);
	bool rewind();
	bool isSeekable();
	int getChannelCount() const;
	int getBitDepth() const;
	double getDuration();

private:

	ModPlugFile *plug;
	ModPlug_Settings settings;

	double duration;

}; // Decoder

} // medialoader

