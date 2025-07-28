/*
 * $Author: pstrand $
 * $Date: 2002/04/16 19:58:42 $
 * $Revision: 1.8 $
 */

#ifndef REAPER_HW_SND_TYPES_H
#define REAPER_HW_SND_TYPES_H

#include <vector>
#include <iosfwd>
#include "hw/exceptions.h"
#include "hw/decoder.h"

namespace reaper
{
namespace hw
{
namespace snd
{

struct SoundInfo
{
	int channels;
	int samplerate;
	int bits_per_sample;

	SoundInfo()
	 : channels(0), samplerate(0), bits_per_sample(0)
	{ }
	SoundInfo(int ch, int sr, int bits)
	 : channels(ch), samplerate(sr), bits_per_sample(bits)
	{ }
};

typedef std::vector<char> Samples;


class AudioSource
{
public:
	virtual SoundInfo info() = 0;
	virtual bool read(Samples& smp) = 0;
	virtual ~AudioSource() { }
};

//typedef std::auto_ptr<AudioSource> AudioSourcePtr;
typedef AudioSource* AudioSourcePtr;

typedef Decoder<AudioSource> AudioDecoder;

typedef AudioDecoder* AudioDecoderPtr;

struct SoundData
{
	SoundInfo info;
	Samples data;
};



}
}
}

#endif

