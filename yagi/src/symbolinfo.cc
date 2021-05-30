#include "symbolinfo.hh"
#include "base.hh"

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
	std::string SymbolInfo::getName() const
	{
		return m_name;
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