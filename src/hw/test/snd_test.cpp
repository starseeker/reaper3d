/*
 * $Author: peter $
 * $Date: 2001/01/06 05:46:42 $
 * $Log: snd_test.cpp,v $
 * Revision 1.1  2001/01/06 05:46:42  peter
 * no message
 *
 */

#include "hw/compat.h"

#include "hw/gfx.h"
#include "hw/snd.h"
#include "hw/debug.h"

#include <iostream>

using reaper::hw::gfx::Gfx;
using reaper::hw::snd::SoundSystem;

int main()
{
	Gfx gfx;
	SoundSystem snd(gfx);

	return 0;
}