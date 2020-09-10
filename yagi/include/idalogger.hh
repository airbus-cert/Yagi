#ifndef __YAGI_IDALOG__
#define __YAGI_IDALOG__

#include "ilogger.hh"

namespace yagi 
{
	class IdaLogger : public ILogger
	{
	public:
		void print(const std::string& message) override;
	};
}

#endif