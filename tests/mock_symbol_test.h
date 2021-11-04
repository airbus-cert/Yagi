#ifndef __YAGI_MOCK_SYMBOL_TEST__
#define __YAGI_MOCK_SYMBOL_TEST__

#include "symbolinfo.hh"
#include "mock_type_test.h"
#include "base.hh"
#include <functional>
#include <optional>
#include <memory>

using MockSymbolInfoFactoryFind = std::function<std::optional<std::unique_ptr<yagi::SymbolInfo>>(uint64_t)>;
using MockSymbolInfoFactoryFindFunction = std::function<std::optional<std::unique_ptr<yagi::FunctionSymbolInfo>>(uint64_t)>;
class MockSymbolInfoFactory : public yagi::SymbolInfoFactory
{
protected:
	MockSymbolInfoFactoryFind m_findCallback;
	MockSymbolInfoFactoryFindFunction m_findFunctionCallback;

public:
	explicit MockSymbolInfoFactory(MockSymbolInfoFactoryFind findCallback, MockSymbolInfoFactoryFindFunction findFunctionCallback)
		: m_findCallback { findCallback }, m_findFunctionCallback { findFunctionCallback }
	{}

	std::optional<std::unique_ptr<yagi::SymbolInfo>> find(uint64_t ea) override
	{
		return m_findCallback(ea);
	}

	std::optional<std::unique_ptr<yagi::FunctionSymbolInfo>> find_function(uint64_t ea) override
	{
		return m_findFunctionCallback(ea);
	}
};

class MockSymbolInfo : public yagi::SymbolInfo
{
protected:
	uint64_t m_size;
	bool m_isFunction;
	bool m_isLabel;
	bool m_isImport;
	bool m_isReadOnly;

public:
	MockSymbolInfo(uint64_t ea, std::string name, uint64_t size, 
		bool isFunction, bool isLabel, bool isImport, bool isReadOnly)
		: SymbolInfo(ea, name), m_size{ size }, 
		m_isFunction{isImport}, m_isLabel{isLabel}, 
		m_isImport{isImport}, m_isReadOnly{isReadOnly}
	{}

	uint64_t getFunctionSize() const override
	{
		return m_size;
	}

	bool isFunction() const noexcept
	{
		return m_isFunction;
	}

	bool isLabel() const noexcept
	{
		return m_isLabel;
	}

	bool isImport() const noexcept
	{
		return m_isImport;
	}

	bool isReadOnly() const noexcept
	{
		return m_isReadOnly;
	}
};

class MockFunctionSymbolInfo : public yagi::FunctionSymbolInfo
{
public:
	std::map<std::string, std::tuple<std::string, uint64_t>> m_name;
	std::map<std::string, std::tuple<MockTypeInfo, uint64_t>> m_type;

	explicit MockFunctionSymbolInfo(std::unique_ptr<yagi::SymbolInfo> symbol)
		: yagi::FunctionSymbolInfo{ std::move(symbol) }
	{
	}

	std::optional<std::string> findStackVar(uint64_t offset, uint32_t addrSize) override
	{
		return std::nullopt;
	}

	std::optional<std::string> findName(uint64_t pc, const std::string& space, uint64_t& offset) override
	{
		std::stringstream ss;
		ss << yagi::to_hex(pc) << "." << space;

		auto iter = m_name.find(ss.str());
		if (iter == m_name.end())
		{
			return std::nullopt;
		}

		offset = std::get<1>(iter->second);
		return std::get<0>(iter->second);
	}

	void saveName(const yagi::MemoryLocation& loc, const std::string& value)  override
	{
		std::stringstream ss;
		ss << yagi::to_hex(loc.pc.front()) << "." << loc.spaceName;
		m_name.emplace(ss.str(), std::make_tuple(value, loc.offset));
	}

	void saveType(const yagi::MemoryLocation& loc, const yagi::TypeInfo& newType) override
	{
		std::stringstream ss;
		ss << yagi::to_hex(loc.pc.front()) << "." << loc.spaceName;
		m_type.emplace(ss.str(), std::make_tuple(MockTypeInfo(newType.getSize(), newType.getName(), newType.isInt(), newType.isBool(), newType.isFloat(), newType.isVoid(), newType.isConst(), newType.isChar(), newType.isUnicode()), loc.offset));
	}

	bool clearType(const yagi::MemoryLocation& loc)
	{
		return true;
	}

	std::optional<std::unique_ptr<yagi::TypeInfo>> findType(uint64_t pc, const std::string& from, uint64_t& offset) override
	{
		std::stringstream ss;
		ss << yagi::to_hex(pc) << "." << from;

		auto iter = m_type.find(ss.str());
		if (iter == m_type.end())
		{
			return std::nullopt;
		}
		offset = std::get<1>(iter->second);
		return std::make_unique<MockTypeInfo>(std::get<0>(iter->second));
	}
};

#endif
