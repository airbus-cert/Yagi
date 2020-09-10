#include "idatypefactory.hh"
#include "idatypeinfo.hh"
#include <idp.hpp>
#include <typeinf.hpp>

namespace yagi 
{
	IdaTypeInfoFactory::IdaTypeInfoFactory()
	{

	}

	IdaTypeInfoFactory::~IdaTypeInfoFactory()
	{}

	std::optional<std::unique_ptr<TypeInfo>> IdaTypeInfoFactory::build(const std::string& name)
	{
		// Rewrite raw type
		tinfo_t idaTypeInfo;

		if (!idaTypeInfo.get_named_type(get_idati(), name.c_str()))
		{
			return std::nullopt;
		}

		return std::make_unique<IdaTypeInfo>(idaTypeInfo);
	}

	std::optional<std::unique_ptr<TypeInfo>> IdaTypeInfoFactory::build(uint64_t ea)
	{
		tinfo_t idaTypeInfo;

		if (!get_tinfo(&idaTypeInfo, ea) && !guess_tinfo(&idaTypeInfo, ea))
		{
			return std::nullopt;
		}

		return std::make_unique<IdaTypeInfo>(idaTypeInfo);
	}

} // end of namespace ghidra