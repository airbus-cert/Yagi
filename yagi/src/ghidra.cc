#include "ghidra.hh"
#include "exception.hh"

#include <libdecomp.hh>

namespace yagi 
{
	void ghidra::init()
	{
		startDecompilerLibrary("C:\\work\\dev\\build\\");
	}
} // end of namespace yagi