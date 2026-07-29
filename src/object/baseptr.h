
#ifndef REAPER_OBJECT_OBJECTREF_H
#define REAPER_OBJECT_OBJECTREF_H

#include "misc/smartptr.h"

namespace reaper {
namespace object {
	class SillyBase;
	class StaticBase;
	class DynamicBase;
	class PlayerBase;
	class ShotBase;

	using PlayerPtr = misc::SmartPtr<object::PlayerBase>;
	using PlayerWPtr = misc::WeakPtr<object::PlayerBase>;
	using SillyPtr = misc::SmartPtr<object::SillyBase>;
	using SillyWPtr = misc::WeakPtr<object::SillyBase>;
	using StaticPtr = misc::SmartPtr<object::StaticBase>;
	using StaticWPtr = misc::WeakPtr<object::StaticBase>;
	using DynamicPtr = misc::SmartPtr<object::DynamicBase>;
	using DynamicWPtr = misc::WeakPtr<object::DynamicBase>;
	using ShotPtr = misc::SmartPtr<object::ShotBase>;
	using ShotWPtr = misc::WeakPtr<object::ShotBase>;

	using ID = int;
}

}


#endif
