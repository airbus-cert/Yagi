#ifndef __YAGI_LOADER__
#define __YAGI_LOADER__

#include <libdecomp.hh>

namespace yagi 
{
	class Loader : public LoadImage
	{
	public:
		Loader();

		std::string getArchType(void) const override;
		void loadFill(uint1* ptr, int4 size, const Address& addr) override;
		void adjustVma(long adjust);
	};
}

#endif