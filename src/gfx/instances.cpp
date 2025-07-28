/*
 * $Author: macke $
 * $Date: 2002/02/08 11:27:51 $
 * $Log: instances.cpp,v $
 * Revision 1.7  2002/02/08 11:27:51  macke
 * rendering reorg and silhouette shadows!
 *
 * Revision 1.6  2001/12/11 23:16:46  macke
 * engine tails now disappear properly opon death...
 *
 * Revision 1.5  2001/10/01 17:36:31  macke
 * Hmm.. grafikmotorn äger objekt som håller en ref-ptr till grafikmotorn.. fel!
 *
 * Revision 1.4  2001/09/24 02:33:24  macke
 * Meckat lite med fulbuggen i grafikmotorn.. verkar funka att deallokera nu.. ?
 *
 */

#include "hw/compat.h"
#include "main/types.h"
#include "gfx/abstracts.h"
#include "gfx/instances.h"
#include "gfx/renderer.h"
#include "hw/debug.h"

namespace reaper {
namespace {
debug::DebugOutput dout("gfx::instance");
}

namespace gfx {

void EffectPtr::draw() const 
{	
	ref().draw(get()); 
}

void SimEffectPtr::insert() const   
{
	ref().insert(get()); 
}
void SimEffectPtr::insert_release()
{ 
	ref().insert_orphan(release()); owned = false; 
}
void SimEffectPtr::transfer()       
{
	ref().transfer(release()); owned = false; 
}
void SimEffectPtr::remove() const   
{
	ref().remove(get()); owned = false; 
}

SimEffectPtr::~SimEffectPtr() {
	if(owned && get() != 0) {
		transfer();
	}
}


}
}
