#ifndef __YAGI_GHIDRA__
#define __YAGI_GHIDRA__

#include <string>

namespace yagi
{
	namespace ghidra
	{
		/*!
		 * \brief	Init the ghidra library with file backends
		 * \param	ghidraPath	Path to the root folder that Start with Ghidra/Processors
		 */
		void init(const std::string& ghidraPath);
	}
}

#endif