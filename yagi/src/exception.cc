#include "exception.hh"
#include "base.hh"
#include <sstream>

namespace yagi 
{
	Error::Error(std::string reason)
		: m_reason(reason)
	{

	}

	char const* Error::what() const
	{
		return m_reason.c_str();
	}

	UnknownTypeError::UnknownTypeError(const std::string& typeName)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unknown type : " << typeName;
		m_reason = ss.str();
	}

	SymbolIsNotAFunction::SymbolIsNotAFunction(const std::string& functionName)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Symbol " << functionName << " is not a function";
		m_reason = ss.str();
	}

	TypeIsNotAFunction::TypeIsNotAFunction(const std::string& typeName)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Type " << typeName << " is not a function";
		m_reason = ss.str();
	}

	UnableToFindFunction::UnableToFindFunction(uint64_t ea)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unable to find a function at  " << to_hex(ea);
		m_reason = ss.str();
	}

	UnableToFindType::UnableToFindType(uint32_t typeId)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unable to find a type " << typeId << " in cache";
		m_reason = ss.str();
	}

	InvalidTypeId::InvalidTypeId(uint32_t typeId)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unknown type id " << typeId;
		m_reason = ss.str();
	}

	InvalidType::InvalidType(std::string name)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Invalid type id " << name;
		m_reason = ss.str();
	}

	UnknownCompiler::UnknownCompiler()
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Invalid Compiler Type";
		m_reason = ss.str();
	}
} // end of namespace yagi