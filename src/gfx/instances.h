#ifndef REAPER_GFX_INSTANCES_H
#define REAPER_GFX_INSTANCES_H

#include <memory>
#include "gfx/gfx.h"
#include "gfx/gfx_types.h"
#include "gfx/abstracts.h"

namespace reaper {
namespace gfx {

class impl_accessor {
protected:
	Renderer &r;
	impl_accessor() : r(Renderer::get_unsafe_ref()) {}

	lowlevel::Renderer& ref() const { return *r.i.get(); }
};

/// Helper class for pointers to graphics entities
template<class T>
class smart_ptr : protected impl_accessor {
protected:
	std::unique_ptr<T> ptr;
	explicit smart_ptr(T* p) : impl_accessor(), ptr(p) {}
	smart_ptr(smart_ptr&& other) noexcept
		: impl_accessor(), ptr(std::move(other.ptr))
	{
	}
	smart_ptr& operator=(smart_ptr&& other) noexcept
	{
		ptr = std::move(other.ptr);
		return *this;
	}

	smart_ptr(const smart_ptr&) = delete;
	smart_ptr& operator=(const smart_ptr&) = delete;
public:
	T* get() const { return ptr.get(); }
	T* release() { return ptr.release(); }
};

class EffectPtr : public smart_ptr<Effect> {
public:
	explicit EffectPtr(Effect *p) : smart_ptr<Effect>(p) {}
	EffectPtr(EffectPtr&&) noexcept = default;
	EffectPtr& operator=(EffectPtr&&) noexcept = default;
	void draw() const;
};

class SimEffectPtr : public smart_ptr<SimEffect> {
	mutable bool owned;
public:
	explicit SimEffectPtr(SimEffect *p) : smart_ptr<SimEffect>(p), owned(true) {}
	SimEffectPtr(SimEffectPtr&& other) noexcept
		: smart_ptr<SimEffect>(std::move(other)), owned(other.owned)
	{
		other.owned = false;
	}
	SimEffectPtr& operator=(SimEffectPtr&& other) noexcept
	{
		if (this == &other)
			return *this;
		if (owned && get())
			transfer();
		smart_ptr<SimEffect>::operator=(std::move(other));
		owned = other.owned;
		other.owned = false;
		return *this;
	}
	~SimEffectPtr();
	void insert() const;
	void insert_release();
	void transfer();
	void remove() const;
};

EffectPtr shot(const Matrix &m, const Color &c); 
EffectPtr sphere(const Matrix &m, float r, const Color &c);

SimEffectPtr shieldsphere(const Matrix &m, float r, const Color &c, const Vector &light);

SimEffectPtr enginetrail(const Matrix &mtx, float size, const Point &offset);
SimEffectPtr smoketrail(const Matrix &mtx, float size, const Point &offset);
SimEffectPtr static_light(const Point &pos, const Color &color, float radius, float fade_speed);
SimEffectPtr dynamic_light(const Matrix &m, const Point &o, const Color &color, 
			       float radius, float fade_speed, bool gravitation = true);

SimEffectPtr sparks(const Point &p, float i);
SimEffectPtr smokepuff(const Point &p, float i);
SimEffectPtr ground_explosion(const Point &p, float i);
SimEffectPtr air_explosion(const Point &p, const Vector &v, float i);
SimEffectPtr grey_smoke(const Point &p, float i);
SimEffectPtr white_smoke(const Point &p, float i);
SimEffectPtr blood_n_guts(const Point &p, float i);

}
}
#endif
