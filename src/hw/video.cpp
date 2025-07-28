/*
 * $Author: pstrand $
 * $Date: 2002/03/11 10:50:48 $
 * $Log: video.cpp,v $
 * Revision 1.10  2002/03/11 10:50:48  pstrand
 * hw/time.h -> reltime.h&abstime.h, and some more..
 *
 * Revision 1.9  2002/02/26 22:35:45  pstrand
 * mackefix
 *
 * Revision 1.8  2002/01/10 23:09:11  macke
 * massa bök
 *
 * Revision 1.7  2001/10/10 00:46:25  peter
 * works but audio not synced, will rewrite to use a generic videodecoder framework...
 *
 * Revision 1.6  2001/07/27 15:47:36  peter
 * massive reorg...
 *
 * Revision 1.5  2001/05/12 22:05:46  peter
 * *** empty log message ***
 *
 * Revision 1.4  2001/05/10 22:08:29  peter
 * video...&worker
 *
 * Revision 1.3  2001/05/04 09:24:36  peter
 * *** empty log message ***
 *
 * Revision 1.2  2001/04/24 12:12:52  peter
 * *** empty log message ***
 *
 * Revision 1.1  2001/04/21 13:57:28  peter
 * *** empty log message ***
 *
 */

#include "hw/compat.h"

#include <string>
#include <iostream>

#include "hw/video.h"
#include "hw/video_helper.h"
#include "hw/gl.h"

namespace reaper {
namespace hw {
namespace video {

Player::Player(gfx::Gfx& g, const std::string& name)
 : gx(g)
{
	plugin = new MpgPlugin();
	out = new MemOutputStream();
	in= new StreamReader(name);
	audio = new APlayer;
	out->set_audio_player(audio);
	
	plugin->setOutputPlugin(out);
	plugin->setInputPlugin(in);


}

void Player::initgfx()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90,1,1,20);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0,0,-3, 0,0,0, 0,1,0);

	glClearColor(0,0,0,0);

	memset(texture[0][0], 0, 1024*512*3);

	GLuint texname;

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_2D,texname);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, texture[0][0]);
}

void Player::draw(VideoFrame& frame)
{
	static char buf[1024*512*4];
	if (frame.data == 0)
		return;
//	printf("new frame: %d\n", frame.data[0]);
	memcpy(buf, frame.data, frame.width*frame.height*4);
	glClear(GL_COLOR_BUFFER_BIT);
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			(1024 - frame.width) / 2, (512 - frame.height) / 2,
			frame.width, frame.height, GL_RGBA, GL_UNSIGNED_BYTE,
			buf);


	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex2f(3,1.5);
	glTexCoord2f(0,1);
	glVertex2f(3,-1.5);
	glTexCoord2f(1,1);
	glVertex2f(-3,-1.5);
	glTexCoord2f(1,0);
	glVertex2f(-3,1.5);
	glEnd();
}



void Player::play()
{
	printf("play\n");
	plugin->play();
	printf("running\n");
	VideoFrame frame;
	int i = 0;
	while (plugin->getStreamState() != _STREAM_STATE_EOF) {
//		printf("new main frame\n");
		out->get_videoframe(frame);
//		printf("draw\n");
		if ((i++ + 2) % 3)
			draw(frame);
		gx.update_screen();
//		std::cin.get();
//		hw::time::msleep(5);
	}
	plugin->close();


}

Player::~Player()
{


	delete plugin;
	delete in;
	delete out;
}


}
}
}
