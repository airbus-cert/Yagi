#ifndef __YAGI_IDECOMPILE__
#define __YAGI_IDECOMPILE__

#include <string>

namespace yagi 
{
	class IDecompiler
	{
	public:
		virtual std::string decompile(uint64_t funcAddress) = 0;
		virtual ~IDecompiler() = default;
	};
}

#endif