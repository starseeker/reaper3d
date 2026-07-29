#include <string>
#include <vector>
#include <map>
#include <typeinfo>

#include "main/exceptions.h"
#include "hw/abstime.h"
#include "hw/debug.h"


#include <cstdlib>
namespace reaper {
namespace {
        typedef std::map<std::string, std::string> Args;
        typedef std::vector<std::string> Argv;

        debug::DebugOutput derr("test_main");
	bool is_testing;
	hw::time::TimeSpan test_start;
	Args args;
	Argv argv;
}
}

int test_main();

bool exit_now()
{
	return reaper::is_testing
	    && (reaper::hw::time::get_time() - reaper::test_start).approx() > 10e6;
}

int main(int argc, char *av[])
{
	int i = 0;
	while (i < argc) {
		reaper::args[av[i]] = (i < argc - 1) ? av[i+1] : "";
		reaper::argv.push_back(av[i]);
		i++;
	}
	if (reaper::args.count("-g"))
		reaper::debug::debug_priority(20);
	try {
		char* p = std::getenv("TESTING");
		reaper::is_testing = p && (*p != '0');
		reaper::test_start = reaper::hw::time::get_time();
		return test_main();
	}
        catch (const reaper::fatal_error_base& e) {
                reaper::derr << "Fatal: " << typeid(e).name() << ' ' << e.what() << '\n';
                return -2;
        } 
        catch (const reaper::error_base& e) {
                reaper::derr << "Error: " << typeid(e).name() << ' ' << e.what() << '\n';
                return -1;
        }
        catch (const std::exception& e) {
                reaper::derr << "Std exception: " << typeid(e).name() << ' ' << e.what() << '\n';
                return -3;
        }
	catch (...) {
		reaper::derr << "Unknown exception\n";
		return -4;
	}
}

/*
 * $Author: fizzgig $
 * $Date: 2002/09/23 12:08:43 $
 * $Log: test_main.h,v $
 * Revision 1.24  2002/09/23 12:08:43  fizzgig
 * Moved all anonymous namespaces into the reaper-namespace
 *
 * Revision 1.23  2002/03/11 10:50:49  pstrand
 * hw/hwtime.h -> reltime.h&abstime.h, and some more..
 *
 * Revision 1.22  2002/02/15 16:22:02  peter
 * no message
 *
 * Revision 1.21  2002/02/06 11:56:36  peter
 * *** empty log message ***
 *
 * Revision 1.20  2002/01/31 05:36:22  peter
 * *** empty log message ***
 *
 * Revision 1.19  2002/01/26 11:24:16  peter
 * resourceimprovements, and minor fixing..
 *
 * Revision 1.18  2002/01/17 05:04:03  peter
 * mp3 i meny (inte igång men funkar), pluginfix..
 *
 * Revision 1.17  2002/01/07 14:00:31  peter
 * resurs och ljudmeck
 *
 * Revision 1.16  2001/11/27 04:09:27  peter
 * *** empty log message ***
 *
 * Revision 1.15  2001/11/26 02:20:14  peter
 * ljudfixar (smartptr), andra mindre fixar..
 *
 * Revision 1.14  2001/07/23 23:48:10  macke
 * Slimmad grafikhantering samt lite namnbyten
 *
 * Revision 1.13  2001/07/19 00:50:06  peter
 * småfixar..
 *
 * Revision 1.12  2001/07/15 21:43:31  peter
 * småfixar
 *
 * Revision 1.11  2001/07/11 10:54:22  peter
 * time.h uppdateringar...
 *
 * Revision 1.10  2001/07/06 01:47:27  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 */
