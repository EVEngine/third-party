#pragma once

#ifdef MEDIALOADER_ENABLE_GME

#include "medialoader/sound/Decoder.h"

#ifdef LOVE_APPLE_USE_FRAMEWORKS
#include <Game_Music_Emu/gme.h>
#else
#include <gme.h>
#endif

namespace medialoader {

class GmeDecoder : public Decoder
{
public:

	GmeDecoder(Data *data, int bufferSize);
	virtual ~GmeDecoder();

	static bool accepts(const std::string &ext);

	love::sound::Decoder *clone();
	int decode();
	bool seek(double s);
	bool rewind();
	bool isSeekable();
	int getChannelCount() const;
	int getBitDepth() const;
	double getDuration();

private:
	Music_Emu *emu;
	int num_tracks;
	int cur_track;
}; // Decoder

} // medialoader
#endif // MEDIALOADER_ENABLE_GME

