#ifndef REAPER_HW_STACKTRACE_H
#define REAPER_HW_STACKTRACE_H

#include <string>
#include <vector>

namespace reaper::hw
{

std::vector<std::string> get_stack_frames();

void load_symbols_prog(const std::string& program);
void load_symbols_file(const std::string& file);

void catch_segfault();

}

#endif
