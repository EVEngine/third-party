#pragma once
#include "medialoader/sound/Decoder.h"

#include <libmpg123/mpg123.h>


namespace medialoader {

struct DecoderFile
{
	unsigned char *data;
	size_t size;
	size_t offset;

	DecoderFile(const char* data, size_t size)
		: data((unsigned char *) data)
		, size(size)
		, offset(0)
	{}
};

class Mpg123Decoder : public Decoder
{
public:

	Mpg123Decoder(const char* data, size_t size, int bufferSize);
	virtual ~Mpg123Decoder();

	static bool accepts(const std::string &ext);
	static void quit();

	Decoder *clone();
	int decode();
	bool seek(double s);
	bool rewind();
	bool isSeekable();
	int getChannelCount() const;
	int getBitDepth() const;
	double getDuration();

private:

	DecoderFile decoder_file;

	mpg123_handle *handle;
	static bool inited;

	int channels;

	double duration;

}; // Decoder

} // medialoader
