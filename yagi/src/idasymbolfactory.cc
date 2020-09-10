#include "idasymbolfactory.hh"
#include "base.hh"
#include "error.hh"
#include "idatool.hh"
#include <idp.hpp>
#include <name.hpp>
#include <sstream>

namespace yagi 
{
	IdaSymbolFactory::IdaSymbolFactory()
	{

	}

	std::optional<std::string> IdaSymbolFactory::getSymbol(uint64_t ea)
	{
		qstring name;
		if (get_name(&name, ea) == 0 || name.size() == 0)
		{
			return std::nullopt;
		}
		return name.c_str();
	}

	std::optional<std::tuple<std::string, uint64_t, uint64_t>> IdaSymbolFactory::getFunction(uint64_t ea)
	{
		auto idaFunc = get_func(ea);
		if (idaFunc == nullptr)
		{
			return std::nullopt;
		}

		qstring idaName;
		get_short_name(&idaName, idaFunc->start_ea);
		auto beginParameter = idaName.find("(");
		auto functionName = split(idaName.substr(0, beginParameter).c_str(), ' ').back();
		return std::make_tuple(functionName, idaFunc->start_ea, idaFunc->end_ea);
	}

	bool IdaSymbolFactory::isFunction(uint64_t ea)
	{
		auto idaFunc = get_func(ea);
		return idaFunc != nullptr && idaFunc->start_ea == ea;
	}

	bool IdaSymbolFactory::isImport(const std::string& name)
	{
		std::string importName = name;
		if (importName.length() > 6 && importName.substr(0, 6) == "__imp_")
		{
			importName = importName.substr(6, importName.length() - 6);
		}

		for (uint i = 0; i < get_import_module_qty(); i++)
		{
			auto found = enum_import_names(i,
				[](ea_t ea, const char* name, uval_t ord, void* param) {
					if (name != nullptr && *static_cast<std::string*>(param) == name)
					{
						return 0;
					}
					return 1;
				}, &importName
			);

			if (found != -1 && found != 1)
			{
				return true;
			}
		}
		return false;
	}
	bool IdaSymbolFactory::isLabel(uint64_t ea)
	{
		xrefblk_t xr;
		for (bool success = xr.first_to((ea_t)ea, XREF_ALL); success; success = xr.next_to()) {
			if (xr.iscode == 0) {
				break;
			}
			if (xr.type != fl_JN) {
				continue;
			}
			return true;
		}

		return false;
	}
} // end of namespace gaip