#include "idaloader.hh"
#include "exception.hh"
#include <libdecomp.hh>

// due to some include incompatibility
// we only forward the interesting function
#ifdef __linux__
#define __stdcall
#endif

extern "C" int64_t __stdcall get_bytes(void* buf, int64_t size, uint64_t ea, int gmb_flags = 0, void* mask = NULL);

#define IDA_LOADER	"ida"

namespace yagi 
{
	/**********************************************************************/
	IdaLoader::IdaLoader()
		: LoadImage(IDA_LOADER)
	{}

	/**********************************************************************/
	std::string IdaLoader::getArchType(void) const
	{
		return IDA_LOADER;
	}

	/**********************************************************************/
	void IdaLoader::loadFill(uint1* ptr, int4 size, const Address& addr)
	{
		::get_bytes(ptr, size, addr.getOffset());
	}

	/**********************************************************************/
	void IdaLoader::adjustVma(long adjust)
	{
		throw LowlevelError("Cannot adjust YAGI virtual memory");
	}
} // end of namespace ghidra
