#ifndef __YAGI_BASE__
#define __YAGI_BASE__

#include <vector>
#include <string>

namespace yagi 
{
	std::string to_hex(uint64_t addr);

	std::vector<std::string> split(const std::string& s, char delimiter);
}

#endif