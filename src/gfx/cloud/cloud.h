
#ifndef REAPER_GFX_CLOUD_CLOUD_H
#define REAPER_GFX_CLOUD_CLOUD_H

#include <deque>
#include <memory>

namespace reaper {
namespace gfx {

class Camera;

namespace cloud {

class CloudEffect;

using Clouds = std::deque<std::unique_ptr<CloudEffect>>;


class CloudSystem
{
	Clouds clouds;
public:
	CloudSystem();
	~CloudSystem();
	void init();
	void shutdown();
	void simul();
	void update(const Camera& cam, const Matrix& m);
};


}
}
}

#endif
