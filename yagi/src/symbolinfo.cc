#include "symbolinfo.hh"
#include "base.hh"
#include <idp.hpp>
#include <name.hpp>
#include <typeinf.hpp>

namespace yagi 
{
	/**********************************************************************/
	// define the prefix
	const std::string SymbolInfo::IMPORT_PREFIX = "__imp_";

	/**********************************************************************/
	SymbolInfo::SymbolInfo(uint64_t ea, std::string name)
		: m_ea(ea), m_name(name)
	{

	}

	/**********************************************************************/
	uint64_t SymbolInfo::getAddress() const noexcept
	{
		return m_ea;
	}

	/**********************************************************************/
	std::string SymbolInfo::getName() const noexcept
	{
		qstring pname;
		if (!cleanup_name(&pname, m_ea, m_name.c_str()))
		{
			pname = m_name.c_str();
		}

		qstring idaName = demangle_name(pname.c_str(), 0);
		if (idaName != "")
		{
			auto pp = idaName.find('(', 0);
			size_t sp = pp;
			while (sp > 0)
			{
				if (idaName.c_str()[sp] == ' ')
				{
					break;
				}
				sp--;
			}
			pname = idaName.substr(sp, pp);
		}

		// Mark import symbol with IDA convention
		if (isImport()) {
			return IMPORT_PREFIX + pname.c_str();
		}
		
		return pname.c_str();
	}

	/**********************************************************************/
	SymbolInfo::Type SymbolInfo::getType() const noexcept
	{
		if (isFunction())
		{
			return SymbolInfo::Type::Function;
		}
		if (isLabel())
		{
			return SymbolInfo::Type::Label;
		}
		if (isImport())
		{
			return SymbolInfo::Type::Import;
		}

		return SymbolInfo::Type::Other;
	}
} // end of namespace yagi