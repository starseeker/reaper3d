#include "hw/compat.h"

#include "hw/worker.h"
#include "hw/abstime.h"
#include "hw/gl.h"
#include "hw/debug.h"

#include "gfx/gfx.h"
#include "gfx/misc.h"

namespace reaper {
namespace gfx {
namespace {
debug::DebugOutput dout("gfx", 0);
}


class PngWriter
 : public hw::worker::Job
{
	std::string fn;
	int w, h;
	char* data;
public:
	PngWriter(std::string f, int width, int height, char* d)
	 : fn(f), w(width), h(height), data(d)
	{}
	bool operator()() {
		char* data2 = (char*) malloc(w*h*4);
		for(int i=0; i<h; i++) {
			for(int j=0; j<w*3; j++) {
				data2[(i*w*3)+j] = data[((h-i-1)*w*3)+j];
			}
		}
		free(data);
		misc::save_png(fn, data2, w, h);
		free(data2);
		return false;
	}
};


void Renderer::screenshot()
{	
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	// FIXME (need to allocat more memory than is used, otherwise crash.. hmm..)
	char* data  = (char*)malloc(vp[2] * vp[3] * 4);

	glReadPixels(vp[0], vp[1], vp[2], vp[3], GL_RGB, GL_UNSIGNED_BYTE, data);

	std::string fn = hw::time::strtime("shot_%Y-%m-%d_%H.%M.%S");
	hw::worker::worker()->add_job(new PngWriter(fn, vp[2], vp[3], data));

	dout << "Screenshot saved to: " << fn << ".png\n";
}

}
}
