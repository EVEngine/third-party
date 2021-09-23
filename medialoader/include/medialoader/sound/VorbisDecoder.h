#pragma once

#include "medialoader/sound/Decoder.h"

// vorbis
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace medialoader {

// Struct for handling data
struct SOggFile
{
	const char *dataPtr;	// Pointer to the data in memory
	int64_t dataSize;	// Size of the data
	int64_t dataRead;	// How much we've read so far
};

class VorbisDecoder : public Decoder
{
public:

	VorbisDecoder(const char* data, size_t size, int bufferSize);
	virtual ~VorbisDecoder();

	static bool accepts(const std::string &ext);

	Decoder* clone();
	int decode();
	bool seek(double s);
	bool rewind();
	bool isSeekable();
	int getChannelCount() const;
	int getBitDepth() const;
	int getSampleRate() const;
	double getDuration();

private:
	SOggFile oggFile;				// (see struct)
	ov_callbacks vorbisCallbacks;	// Callbacks used to read the file from mem
	OggVorbis_File handle;			// Handle to the file
	vorbis_info *vorbisInfo;		// Info
	vorbis_comment *vorbisComment;	// Comments
	int endian;						// Endianness
	double duration;
}; // VorbisDecoder

} // medialoader
