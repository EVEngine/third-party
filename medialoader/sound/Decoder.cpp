
#include "medialoader/sound/Decoder.h"


namespace medialoader {

Decoder::Decoder(const char* data, size_t size, int bufferSize)
	: data(data), size(size)
	, bufferSize(bufferSize)
	, sampleRate(DEFAULT_SAMPLE_RATE)
	, buffer(0)
	, eof(false)
{
	buffer = new char[bufferSize];
}

Decoder::~Decoder()
{
	if (buffer != 0)
		delete [](char *) buffer;
}

void* Decoder::getBuffer() const
{
	return buffer;
}

int Decoder::getSize() const
{
	return bufferSize;
}

int Decoder::getSampleRate() const
{
	return sampleRate;
}

bool Decoder::isFinished()
{
	return eof;
}

} // medialoader
