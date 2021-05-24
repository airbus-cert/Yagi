#ifndef __YAGI_ERROR__
#define __YAGI_ERROR__

#include <exception>
#include <string>

namespace yagi 
{
	class Error : public std::exception
	{
	protected:
		std::string m_reason;
	public:
		explicit Error(std::string reason);
		virtual char const* what() const override;
	};

	class UnknownTypeError : public Error
	{
	public:
		explicit UnknownTypeError(const std::string& typeName);
	};

	class SymbolIsNotAFunction : public Error
	{
	public:
		explicit SymbolIsNotAFunction(const std::string& functionName);
	};

	class TypeIsNotAFunction : public Error
	{
	public:
		explicit TypeIsNotAFunction(const std::string& typeName);
	};

	class UnableToFindFunction : public Error
	{
	public:
		explicit UnableToFindFunction(uint64_t ea);
	};

	class UnableToFindType : public Error
	{
	public:
		explicit UnableToFindType(uint32_t typeName);
	};

	class UnableToFindPrototype : public Error
	{
	public:
		explicit UnableToFindPrototype(std::string funName);
	};

	class InvalidTypeId : public Error
	{
	public:
		explicit InvalidTypeId(uint32_t typeId);
	};

	class InvalidType : public Error
	{
	public:
		explicit InvalidType(std::string);
	};

	class UnknownCompiler : public Error
	{
	public:
		explicit UnknownCompiler();
	};

	class UnknownCallingConvention : public Error
	{
	public:
		explicit UnknownCallingConvention(std::string funcName);
	};
}

#endif