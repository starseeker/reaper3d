
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

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
	std::vector<char> data;
public:
	PngWriter(std::string f, int width, int height, std::vector<char> pixels)
	 : fn(std::move(f)), w(width), h(height), data(std::move(pixels))
	{}
	bool operator()() {
		const std::size_t row_bytes = static_cast<std::size_t>(w) * 3;
		std::vector<char> flipped(data.size());
		for(int row = 0; row < h; ++row) {
			const auto source =
				data.begin() + (h - row - 1) * row_bytes;
			std::copy_n(
				source,
				row_bytes,
				flipped.begin() + row * row_bytes);
		}
		misc::save_png(fn, flipped.data(), w, h);
		return false;
	}
};


void Renderer::screenshot()
{	
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	GLint previous_pack_alignment = 0;
	glGetIntegerv(GL_PACK_ALIGNMENT, &previous_pack_alignment);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	std::vector<char> data(
		static_cast<std::size_t>(vp[2]) * vp[3] * 3);
	glReadPixels(
		vp[0],
		vp[1],
		vp[2],
		vp[3],
		GL_RGB,
		GL_UNSIGNED_BYTE,
		data.data());
	glPixelStorei(GL_PACK_ALIGNMENT, previous_pack_alignment);

	std::string fn = hw::time::strtime("shot_%Y-%m-%d_%H.%M.%S");
	hw::worker::worker()->add_job(
		std::make_shared<PngWriter>(
			fn,
			vp[2],
			vp[3],
			std::move(data)));

	dout << "Screenshot saved to: " << fn << ".png\n";
}

}
}
