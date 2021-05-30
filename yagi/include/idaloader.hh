#ifndef __YAGI_IDALOADER__
#define __YAGI_IDALOADER__

#include "loader.hh"
#include <libdecomp.hh>

namespace yagi 
{
	class IdaLoader : public LoadImage
	{
	public:
		explicit IdaLoader();

		std::string getArchType(void) const override;
		void loadFill(uint1* ptr, int4 size, const Address& addr) override;
		void adjustVma(long adjust);
	};

	using IdaLoaderFactory = LoaderFactoryDefault<IdaLoader>;
}

#endif