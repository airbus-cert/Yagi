#include "idalogger.hh"
#include <idp.hpp>
#include <kernwin.hpp>

namespace yagi 
{
	/**********************************************************************/
	void IdaLogger::print(const std::string& message)
	{
		msg(message.c_str());
	}
} // end of namespace yagi