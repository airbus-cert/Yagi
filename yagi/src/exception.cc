#include "exception.hh"
#include "base.hh"
#include <sstream>

namespace yagi 
{
	/**********************************************************************/
	Error::Error(std::string reason)
		: m_reason(reason)
	{

	}

	/**********************************************************************/
	char const* Error::what() const noexcept
	{
		return m_reason.c_str();
	}

	/**********************************************************************/
	UnknownTypeError::UnknownTypeError(const std::string& typeName)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unknown type : " << typeName;
		m_reason = ss.str();
	}

	/**********************************************************************/
	SymbolIsNotAFunction::SymbolIsNotAFunction(const std::string& functionName)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Symbol " << functionName << " is not a function";
		m_reason = ss.str();
	}

	/**********************************************************************/
	TypeIsNotAFunction::TypeIsNotAFunction(const std::string& typeName)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Type " << typeName << " is not a function";
		m_reason = ss.str();
	}

	/**********************************************************************/
	UnableToFindFunction::UnableToFindFunction(uint64_t ea)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unable to find a function at  " << to_hex(ea);
		m_reason = ss.str();
	}

	/**********************************************************************/
	UnableToFindType::UnableToFindType(uint32_t typeId)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unable to find a type " << typeId << " in cache";
		m_reason = ss.str();
	}

	/**********************************************************************/
	UnknownCallingConvention::UnknownCallingConvention(std::string funcName)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unknown calling convention of method " << funcName;
		m_reason = ss.str();
	}

	/**********************************************************************/
	UnknownCompiler::UnknownCompiler(uint32_t compilerId)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unknown the compiler ID " << compilerId;
		m_reason = ss.str();
	}

	/**********************************************************************/
	UnImplementedFunction::UnImplementedFunction(const std::string& funcName)
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Unimplemented function " << funcName;
		m_reason = ss.str();
	}

	/**********************************************************************/
	NoDefaultCallingConvention::NoDefaultCallingConvention()
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "The current arch have no default calling convention ";
		m_reason = ss.str();
	}

	/**********************************************************************/
	NoMoreData::NoMoreData()
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "No more data in the loader, decompilation is canceled";
		m_reason = ss.str();
	}

	/**********************************************************************/
	UnableToFoundGhidraFolder::UnableToFoundGhidraFolder()
		: Error("")
	{
		std::stringstream ss(m_reason);
		ss << "Ghidra folder missing. Yagi was not correctly installed.";
		m_reason = ss.str();
	}
} // end of namespace yagi
