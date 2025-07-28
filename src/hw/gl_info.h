
#ifndef REAPER_HW_GL_EXT_H
#define REAPER_HW_GL_EXT_H

#include <string>

namespace reaper {
namespace hw {
namespace gfx {

struct OpenGLinfo_data;

class OpenGLinfo
{
	OpenGLinfo_data* data;

	friend OpenGLinfo& opengl_info_inst();
	friend void reinit_opengl();

	OpenGLinfo();
	OpenGLinfo(const OpenGLinfo&);
	void init();
public:
	~OpenGLinfo();

	const std::string& vendor() const;
	const std::string& renderer() const;
	const std::string& version() const;

	bool is_supported(const std::string& s) const;

	friend std::ostream& operator<<(std::ostream &s, const OpenGLinfo &i);
};

std::ostream& operator<<(std::ostream&, const OpenGLinfo&);

// This needs to be called when a new opengl context has been created.
// It calls reinit_glstates as well.
void reinit_opengl();


const OpenGLinfo& opengl_info();


}
}
}

#endif

