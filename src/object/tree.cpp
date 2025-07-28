
#include "hw/compat.h"

#include "object/base.h"
#include "object/factory.h"
#include "object/hull.h"
#include "object/object_impl.h"
#include "res/config.h"
#include "main/types_io.h"
#include "misc/utility.h"

namespace reaper {
namespace object {




class Tree2 : public SillyImpl
{
	gfx::RenderInfo leaves;
public:
	Tree2(MkInfo mk) :
	   SillyImpl(MkInfo("tree1", mk.mtx, mk.cid, mk.id, mk.info, mk.cdata)), 
	   leaves("tree1_leaves", get_mtx(), true ) 
	{ 
		   ri.link(leaves);
		   //FIXME: Trees are rotated randomly around y, remove once leveleditor gets fixed
		   data.set_mtx(get_mtx() * Matrix(misc::frand() * 360, Vector(0,1,0)));
	}

	const gfx::RenderInfo* render(bool effects) const { return SillyImpl::render(effects); }
};
	

SillyBase* tree1(MkInfo mk)
{
	return new Tree2(mk);
}

class Bush : public SillyImpl
{
public:
	Bush(MkInfo mk) :
	SillyImpl(mk)
	{
		ri.blend = true;
	}
};

SillyBase* bush1(MkInfo mk)
{
	return new Bush(mk);
}


namespace {
struct Foo {
	Foo() {
		factory::inst().register_object("tree1", tree1);
		factory::inst().register_object("bush1", bush1);
	}
} bar;
}

void tree() { }

}
}

