/*
 * $Author: peter $
 * $Date: 2001/08/12 13:32:28 $
 * $Revision: 1.8 $
 */


#ifndef REAPER_HW_VIDEO_H
#define REAPER_HW_VIDEO_H

#include "hw/gfx.h"
#include "hw/decoder.h"

class MpgPlugin;
class InputStream;

namespace reaper {
namespace hw {
namespace video {

class VideoFrame;
class MemOutputStream;
class AudioPlayer;

class Player 
{
	unsigned char texture[1024][512][3];
	void draw(VideoFrame&);
	MpgPlugin* plugin;
	MemOutputStream* out;
	InputStream* in;
	AudioPlayer* audio;
	gfx::Gfx& gx;
public:
	Player(gfx::Gfx& gx, const std::string& name);
	~Player();
	void initgfx();
	void play();
};


// typedef decoder::Decoder<FrameInfo, FrameData> VideoDecoder;

}
}
}

#endif

