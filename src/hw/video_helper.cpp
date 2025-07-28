/*
 * $Author: pstrand $
 * $Date: 2002/03/11 10:50:48 $
 * $Log: video_helper.cpp,v $
 * Revision 1.8  2002/03/11 10:50:48  pstrand
 * hw/time.h -> reltime.h&abstime.h, and some more..
 *
 * Revision 1.7  2001/11/26 02:20:11  peter
 * ljudfixar (smartptr), andra mindre fixar..
 *
 * Revision 1.6  2001/10/16 21:10:47  peter
 * video & win32 fixar
 *
 * Revision 1.5  2001/10/10 00:46:26  peter
 * works but audio not synced, will rewrite to use a generic videodecoder framework...
 *
 * Revision 1.4  2001/05/12 22:05:46  peter
 * *** empty log message ***
 *
 * Revision 1.3  2001/05/10 22:08:29  peter
 * video...&worker
 *
 * Revision 1.2  2001/04/24 12:12:53  peter
 * *** empty log message ***
 *
 * Revision 1.1  2001/04/21 13:57:28  peter
 * *** empty log message ***
 *
 */

#include "hw/compat.h"

#include <string>

#include "hw/video_helper.h"



namespace reaper {
namespace hw {
namespace video {



MemOutputStream::MemOutputStream() {
	avSyncer=new AVSyncer(10000);

	audioTime=new AudioTime();

	lneedInit=false;
	lVideoInit=false;
	lBufferSet=false;
	membuffer = 0;
	pic = 0;
	pic_write.signal();
}


MemOutputStream::~MemOutputStream() {
	delete audioTime;
	delete avSyncer;
}



int MemOutputStream::audioSetup(int frequency,int stereo,
				   int sign,int big,int sixteen) {
//	printf("audiosetup\n");
	access.lock();
	audioTime->setFormat(stereo,sixteen,frequency);
	avSyncer->audioSetup(frequency,stereo,sign,big,sixteen);
	if (player)
		player->init(sixteen, stereo == 1, frequency);
	access.unlock();
	return true;
}


void MemOutputStream::audioClose(void) {
	access.lock();
	avSyncer->audioClose();
	access.unlock();
}


void MemOutputStream::audioOpen() {
//	printf("audioopen\n");
}




int MemOutputStream::audioPlay(TimeStamp* startStamp,
			       TimeStamp* endStamp, char *buffer, int size) {
//	printf("audioplay\n");
	access.lock();
	if (lneedInit && player) {
		player->init(audioTime->getSampleSize(), audioTime->getStereo() == 1, audioTime->getSampleSize());
		lneedInit=false;
	}
	if (player)
		player->play(reinterpret_cast<unsigned char*>(buffer), size);
	avSyncer->audioPlay(startStamp,endStamp,buffer,size);
	access.unlock();
	return size;
}


int MemOutputStream::getPreferredDeliverSize() {
	access.lock();
	int i = avSyncer->getPreferredDeliverSize();
	access.unlock();
	return i;
}


int MemOutputStream::openWindow(int w, int h,const char *title) {
	printf("openwindow\n");
	access.lock();
	width = w;
	height = h;
	picArray = new PictureArray(width, height);
	setOutputInit(true);
	access.unlock();
	return 1;
}


void MemOutputStream::closeWindow() {
	printf("closeWindow\n");
}

void MemOutputStream::flushWindow() {
	printf("flushWindow\n");
}


PictureArray* MemOutputStream::lockPictureArray() {
	return picArray;
}


void MemOutputStream::unlockPictureArray(PictureArray* pictureArray)
{
//	printf("waiting for write\n");
	pic_write.wait();

	access.lock();
	pic = pictureArray->getYUVPictureCallback();
//	printf("writing pic\n");

	if (pic == 0)
		return;
	avSyncer->syncPicture(pic);
	access.unlock();
	pic_read.signal();
}

int MemOutputStream::getFrameusec() {
	printf("getFrameusec\n");
	return avSyncer->getFrameusec();
}

  

int MemOutputStream::getOutputInit() {
	printf("getOutputInit\n");
	return lVideoInit;
}


void MemOutputStream::setOutputInit(int lInit) {
	printf("setOutputInit");
	this->lVideoInit=lInit;
}
 
 

void MemOutputStream::config(const char* key, const char* value,void* user_data) {
	printf("config\n");
}



AVSyncer* MemOutputStream::getAVSyncer() {
	printf("avsyncer\n");
	return avSyncer;
}


void MemOutputStream::get_videoframe(VideoFrame& frame)
{
	pic_read.wait();
	access.lock();
	static Dither32Bit dither(0x000000ff, 0x0000ff00, 0x00ff0000);
	if (frame.data == 0) {
		frame.width = width;
		frame.height = height;
		frame.data = new unsigned char[width * height * 4];
		printf("creating frame: %dx%d\n", width, height);
	}
	if (pic) {
//		printf("dithering...\n");
		dither.ditherImageColor32(pic->getLuminancePtr(),
					  pic->getCrPtr(), pic->getCbPtr(),
					  frame.data, frame.height, frame.width, 0);
	}
	access.unlock();
	pic_write.signal();
}

}
}
}
