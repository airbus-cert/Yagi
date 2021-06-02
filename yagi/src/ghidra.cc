#include "ghidra.hh"
#include "exception.hh"

#include <libdecomp.hh>

namespace yagi 
{
	/**********************************************************************/
	void ghidra::init(const std::string& ghidraPath)
	{
		startDecompilerLibrary(ghidraPath.c_str());
	}
} // end of namespace yagi