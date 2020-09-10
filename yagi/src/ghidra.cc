#include "ghidra.hh"
#include "error.hh"

#include <libdecomp.hh>

namespace yagi 
{
	void ghidra::init()
	{
		startDecompilerLibrary("C:\\Program Files\\IDA Pro 7.4\\plugins");
	}
} // end of namespace gaip