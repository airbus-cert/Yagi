#include "symbolinfo.hh"
#include "base.hh"
#include "error.hh"
#include <idp.hpp>
#include <name.hpp>
#include <typeinf.hpp>

namespace yagi 
{
	/**********************************************************************/
	SymbolInfo::SymbolInfo(uint64_t ea, std::string name, std::shared_ptr<SymbolFactory> database)
		: m_ea(ea), m_name(name), m_database(database)
	{

	}

	/**********************************************************************/
	std::optional<SymbolInfo> SymbolInfo::load(uint64_t ea, std::shared_ptr<SymbolFactory> database) noexcept
	{
		auto name = database->getSymbol(ea);
		if (!name.has_value())
		{
			return std::nullopt;
		}

		return SymbolInfo(ea, name.value(), database);
	}

	/**********************************************************************/
	std::optional<SymbolInfo> SymbolInfo::find(uint64_t ea, std::shared_ptr<SymbolFactory> database) noexcept
	{
		auto function = database->getFunction(ea);
		if (!function.has_value())
		{
			return std::nullopt;
		}

		auto [name, startEA, endEA] = function.value();
		return SymbolInfo(startEA, name, database);
	}

	/**********************************************************************/
	uint64_t SymbolInfo::getAddress() const noexcept
	{
		return m_ea;
	}

	/**********************************************************************/
	uint64_t SymbolInfo::getFunctionSize() const
	{
		auto function = m_database->getFunction(m_ea);
		if (!function.has_value())
		{
			throw SymbolIsNotAFunction(m_name);
		}

		auto [name, startEA, endEA] = function.value();
		if (startEA != m_ea)
		{
			throw SymbolIsNotAFunction(m_name);
		}

		return endEA - startEA;
	}

	/**********************************************************************/
	std::string SymbolInfo::getName() const noexcept
	{
		return m_name;
	}

	/**********************************************************************/
	bool SymbolInfo::isFunction() const noexcept
	{
		return m_database->isFunction(m_ea);
	}

	/**********************************************************************/
	bool SymbolInfo::isLabel() const noexcept
	{
		return m_database->isLabel(m_ea);
	}

	/**********************************************************************/
	bool SymbolInfo::isImport() const noexcept
	{
		return m_database->isImport(m_name);
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
} // end of namespace gaip