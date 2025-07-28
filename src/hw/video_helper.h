#ifndef REAPER_HW_VIDEO_HELPER_H
#define REAPER_HW_VIDEO_HELPER_H

#include "hw/snd_simple.h"

#include "ext/mpegvideo/mpgplayer/mpgPlugin.h"
#include "ext/mpegvideo/util/render/dither/dither32Bit.h"
#include "ext/mpegvideo/output/avSyncer.h"

#undef min
#undef max

#include "res/res.h"

#include "misc/iostream_helper.h"
#include "hw/concurrent.h"

#include "hw/debug.h"
#include "hw/snd.h"
#include "hw/interfaces.h"
#include "hw/snd_subsystem.h"

#include "misc/plugin.h"

namespace reaper {
namespace hw {
namespace video {

typedef misc::PluginCreator<snd::Subsystem, misc::SharedObj> SndProxy;

class Performance;
class YUVDumper;

class AudioPlayer {
public:
	virtual void init(int bits, bool stereo, int frequency) = 0;
	virtual void play(unsigned char* buf, long length) = 0;
};

class VideoFrame {
public:
	int width;
	int height;
	unsigned char* data;
	VideoFrame()
	 : width(0), height(0), data(0) { }
};


class Main
 : public ifs::Debug
{
	debug::DebugOutput dr;
public:
	Main() : dr("snd_plugin")
	{ }
	debug::DebugOutput& derr() { return dr; }
};

class Dec
 : public snd::AudioDecoder
{
	snd::SoundData* buf;
public:
	Dec(snd::SoundData* b)
	 : buf(b)
	{
	}
	bool init(misc::SmartPtr<res::res_stream>)
	{
		return true;
	}
	std::auto_ptr<snd::AudioSource> get()
	{
		return std::auto_ptr<snd::AudioSource>(0);
	}
	snd::SoundInfo info()
	{
		return buf->info;
	}
	bool read(snd::Samples& sd)
	{
		sd = buf->data;
		return true;
	}
	~Dec() { }

};

class APlayer : public AudioPlayer
{
	SndProxy snd_proxy;
	snd::Subsystem* player;
	snd::SoundPtr audio;
	misc::SmartPtr<Dec> decoder;
	hw::snd::SoundInfo si;
	snd::SoundData sd;
public:
	APlayer()
	{
		Main* main = new Main;
		player = snd_proxy.create("snd_simple", main);
	}
	virtual void init(int bits, bool stereo, int frequency)
	{
		si.bits_per_sample = bits;
		si.channels = stereo ? 2 : 1;
		si.samplerate = frequency;
		decoder = misc::SmartPtr<Dec>(new Dec(&sd));
//		audio = player->prepare(decoder);
		audio->play();
	}

	virtual void play(unsigned char* buf, long length)
	{
		sd.data.resize(length);
		std::copy(buf, buf+length, &sd.data[0]);
//		audio->do_stuff();
	}
	virtual ~APlayer() { }
};

class StreamReader : public InputStream
{
	res::res_stream input;
	bool is_open;
public:
	StreamReader(const std::string& name)
	 : input(res::Video, name), is_open(true)
	{ }
	int open(const char*) { return 0; }
	int isOpen() { return is_open; }
	int eof() { return input.eof(); }
	
	int read(char* ptr, int size) {
//		printf("read req: %d -> %d\n", size, input.good());
		input.read(ptr, size);
		return size;
	}

	int seek(long pos) {
		misc::set_pos(input, pos);
		return 1;
	}
	long getByteLength() {
		return misc::get_size(input);
	}
	long getBytePosition() {
		return misc::get_cur_pos(input);
	}
};



class MemOutputStream : public OutputStream {
	AudioPlayer* player;
	AVSyncer* avSyncer;
	PictureArray* picArray; 

	int lBufferSet;
	int lVideoInit;
	int lavSync;
	int lneedInit;
	int lPerformance;
	int lYUVDump;

	int width, height;

	YUVPicture* pic;
	hw::concurrent::Semaphore pic_write, pic_read;
	hw::concurrent::Mutex access;


	unsigned char* membuffer;

	AudioTime* audioTime;
	YUVDumper* yuvDumper;

public:
	MemOutputStream();
	~MemOutputStream();

	// Audio Output

	int audioSetup(int frequency,int stereo,int sign,int big,int sixteen);
	void audioClose();
	void audioOpen();
	int audioPlay(TimeStamp* startStamp,
		TimeStamp* endStamp,char *buffer, int size);


	int getPreferredDeliverSize();


	// Video Output

	int openWindow(int width, int height,const char *title);
	void closeWindow();
	void flushWindow();

	PictureArray* lockPictureArray();
	void unlockPictureArray(PictureArray* pictureArray);

	int getFrameusec();

	int getDepth();
	int getOutputInit();
	void setOutputInit(int lInit);

	void config(const char* key,const char* value,void* user_data);

	void get_videoframe(VideoFrame&);
	void set_audio_player(AudioPlayer* pl) { player = pl; }

	// methods which do not belong to the outputStream intferface;
	AVSyncer* getAVSyncer();

	void writeInfo(class PluginInfo* pluginInfo) { }

};

}
}
}
#endif

/*
 * $Author: peter $
 * $Date: 2002/01/17 04:58:47 $
 * $Log: video_helper.h,v $
 * Revision 1.15  2002/01/17 04:58:47  peter
 * ljud-plugin-mm fix..
 *
 * Revision 1.14  2002/01/12 02:47:09  peter
 * *** empty log message ***
 *
 * Revision 1.13  2002/01/11 21:26:39  peter
 * audiosource, mm
 *
 * Revision 1.12  2002/01/07 16:38:26  peter
 * Singalong: "Vi hatar, vi hatar vc--, vi hatar de som vc skapat har..."
 *
 * Revision 1.11  2002/01/07 14:00:30  peter
 * resurs och ljudmeck
 *
 * Revision 1.10  2001/10/16 21:10:48  peter
 * video & win32 fixar
 *
 * Revision 1.9  2001/10/10 01:00:33  peter
 * no message
 *
 * Revision 1.8  2001/10/10 00:46:26  peter
 * works but audio not synced, will rewrite to use a generic videodecoder framework...
 *
 * Revision 1.7  2001/08/06 12:16:25  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.6.4.1  2001/08/03 13:44:01  peter
 * pragma once obsolete...
 *
 * Revision 1.6  2001/07/06 01:47:23  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 * Revision 1.5  2001/05/12 22:05:47  peter
 * *** empty log message ***
 *
 * Revision 1.4  2001/05/10 22:08:29  peter
 * video...&worker
 *
 * Revision 1.3  2001/05/04 09:24:36  peter
 * *** empty log message ***
 *
 * Revision 1.2  2001/04/24 12:12:53  peter
 * *** empty log message ***
 *
 * Revision 1.1  2001/04/21 13:57:28  peter
 * *** empty log message ***
 *
 */

