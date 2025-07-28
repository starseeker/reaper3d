
#include "hw/compat.h"

#include <math.h>

#include "hw/debug.h"
#include "hw/dynload.h"
#include "misc/test_main.h"

#include <iostream>

using namespace reaper;
using hw::dynload::DynLoader;

int test_main()
{
#ifdef WIN32
	DynLoader dyn("msvcrt");
#else
	DynLoader dyn("m");
#endif
	double (*sine)(double);
	dyn.load("sin", sine);
	derr << "loaded: " << sine(1.2) << " int: " << sin(1.2) << '\n';
	return 0;
}


