
#include "hw/compat.h"

#include "hw/gfx.h"



namespace reaper
{
namespace hw
{
namespace gfx
{

VideoMode::VideoMode(int w, int h, int d, bool f, int fq, int i)
 : width(w), height(h), fullscreen(f), depth(d), frequency(fq), id(i)
{ }

VideoMode::VideoMode(int w, int h, bool f, int i)
 : width(w), height(h), fullscreen(f), depth(0), frequency(0), id(i)
{ }

VideoMode::VideoMode()
 : width(0), height(0), fullscreen(false), depth(0), frequency(0), id(-1)
{ }

bool operator<(const VideoMode& m1, const VideoMode& m2)
{
	return m1.width < m2.width || m1.height < m2.height
	    || m1.depth < m2.depth || m1.frequency < m2.frequency;
}

bool operator==(const VideoMode& m1, const VideoMode& m2)
{
	return m1.width == m2.width && m1.height == m2.height &&
	       (m1.depth == 0 || m2.depth == 0 || m1.depth == m2.depth) &&
	       m1.fullscreen == m2.fullscreen;
}


}
}
}
