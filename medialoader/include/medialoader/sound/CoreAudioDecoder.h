#pragma once
// #include "config.h"
#ifdef MEDIALOADER_ENABLE_COREAUDIO

#include "medialoader/sound/Decoder.h"

// Core Audio
#include <AudioToolbox/AudioFormat.h>
#include <AudioToolbox/ExtendedAudioFile.h>


/**
 * Decoder which supports all formats handled by Apple's Core Audio framework.
 **/
class CoreAudioDecoder : public Decoder
{
public:

	CoreAudioDecoder(Data *data, int bufferSize);
	virtual ~CoreAudioDecoder();

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

	void closeAudioFile();

	AudioFileID audioFile;
	ExtAudioFileRef extAudioFile;

	AudioStreamBasicDescription inputInfo;
	AudioStreamBasicDescription outputInfo;

	double duration;

}; // CoreAudioDecoder


#endif // LOVE_SUPPORT_COREAUDIO

