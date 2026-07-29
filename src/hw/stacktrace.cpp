#include "hw/stacktrace.h"

#include <csignal>
#include <cstdlib>

#if defined(__linux__)
#include <execinfo.h>
#include <unistd.h>
#endif

namespace reaper::hw
{

#if defined(__linux__)
namespace {

void print_fatal_stack(int signal_number)
{
	constexpr char heading[] = "Fatal signal; stack trace:\n";
	::write(STDERR_FILENO, heading, sizeof(heading) - 1);

	void* frames[64];
	const int count = ::backtrace(frames, 64);
	::backtrace_symbols_fd(frames, count, STDERR_FILENO);

	std::signal(signal_number, SIG_DFL);
	std::raise(signal_number);
}

}
#endif

std::vector<std::string> get_stack_frames()
{
#if defined(__linux__)
	void* frames[64];
	const int count = ::backtrace(frames, 64);
	char** symbols = ::backtrace_symbols(frames, count);
	if (symbols == nullptr)
		return {};

	std::vector<std::string> result;
	result.reserve(count);
	for (int index = 0; index < count; ++index)
		result.emplace_back(symbols[index]);
	std::free(symbols);
	return result;
#else
	return {};
#endif
}

void load_symbols_prog(const std::string&)
{
}

void load_symbols_file(const std::string&)
{
}

void catch_segfault()
{
#if defined(__linux__)
	std::signal(SIGSEGV, print_fatal_stack);
	std::signal(SIGABRT, print_fatal_stack);
#endif
}

}
