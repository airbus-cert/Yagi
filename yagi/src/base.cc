#include "base.hh"
#include <sstream>

namespace yagi 
{
	std::string to_hex(uint64_t addr)
	{
		std::stringstream stream;
		stream << "0x" << std::hex << addr;
		return stream.str();
	}

	std::vector<std::string> split(const std::string& s, char delimiter)
	{
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(s);
		while (std::getline(tokenStream, token, delimiter))
		{
			tokens.push_back(token);
		}
		return tokens;
	}
} // end of namespace yagi