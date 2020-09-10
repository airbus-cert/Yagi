#ifndef __YAGI_IDATYPEFACTORY__
#define __YAGI_IDATYPEFACTORY__

#include "typeinfo.hh"

namespace yagi 
{
	class IdaTypeInfoFactory : public TypeInfoFactory
	{
	public:
		explicit IdaTypeInfoFactory();
		virtual ~IdaTypeInfoFactory();

		std::optional<std::unique_ptr<TypeInfo>> build(const std::string& name) override;
		std::optional<std::unique_ptr<TypeInfo>> build(uint64_t ea) override;

	};
}

#endif