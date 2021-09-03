#include "mock_type_test.h"

MockFuncInfo::MockFuncInfo(bool isDotDotDot, std::string callingConvention, std::vector<MockTypeInfo> prototype, std::vector<std::string> paramName)
	: m_isDotDotDot{ isDotDotDot }, m_callingConvention{ callingConvention }, m_prototype{ prototype }, m_paramName{ paramName }
{}

bool MockFuncInfo::isDotDotDot() const
{
	return m_isDotDotDot;
}

std::vector<std::unique_ptr<yagi::TypeInfo>> MockFuncInfo::getFuncPrototype() const
{
	std::vector<std::unique_ptr<yagi::TypeInfo>> result;
	for (auto& r : m_prototype)
	{
		result.push_back(std::make_unique<MockTypeInfo>(r));
	}
	return result;
}

std::vector<std::string> MockFuncInfo::getFuncParamName() const
{
	return m_paramName;
}

std::string MockFuncInfo::getCallingConv() const
{
	return m_callingConvention;
}

std::string MockFuncInfo::getName() const
{
	return "";
}

MockTypeInfo::MockTypeInfo(size_t size, std::string name, bool isInt, bool isBool, bool isFloat, bool isVoid, bool isConst, bool isChar, bool isUnicode)
	: m_size{ size }, m_name{ name }, m_isInt{ isInt }, m_isBool{ isBool }, m_isFloat{ isFloat }, m_isVoid{ isVoid }, m_isConst{ isConst }, m_isChar{ isChar }, m_isUnicode{ isUnicode }, m_funcInfo{ std::nullopt }
{}

MockTypeInfo::MockTypeInfo(size_t size, std::string name, MockFuncInfo funcInfo)
	: m_size {size}, m_name {name}, m_funcInfo{std::make_optional<MockFuncInfo>(funcInfo)}, m_isInt{ false }, m_isBool{ false }, m_isFloat{ false }, m_isVoid{ false }, m_isConst{ false }, m_isChar{ false }, m_isUnicode{ false }
{}

size_t MockTypeInfo::getSize() const
{
	return m_size;
}

std::string MockTypeInfo::getName() const
{
	return m_name;
}

std::string MockTypeInfo::getRawName() const
{
	return m_name;
}

bool MockTypeInfo::isInt() const
{
	return m_isInt;
}

bool MockTypeInfo::isBool() const
{
	return m_isBool;
}

bool MockTypeInfo::isFloat() const
{
	return m_isFloat;
}

bool MockTypeInfo::isVoid() const
{
	return m_isVoid;
}

bool MockTypeInfo::isConst() const
{
	return m_isConst;
}

bool MockTypeInfo::isChar() const
{
	return m_isChar;
}

bool MockTypeInfo::isUnicode() const
{
	return m_isUnicode;
}

std::optional<std::unique_ptr<yagi::FuncInfo>>MockTypeInfo::toFunc() const
{
	if (!m_funcInfo.has_value())
	{
		return std::nullopt;
	}

	return std::make_unique<MockFuncInfo>(m_funcInfo.value());
}

std::optional<std::unique_ptr<yagi::StructInfo>> MockTypeInfo::toStruct() const
{
	return std::nullopt;
}

std::optional<std::unique_ptr<yagi::PtrInfo>> MockTypeInfo::toPtr() const
{
	return std::nullopt;
}

std::optional<std::unique_ptr<yagi::ArrayInfo>> MockTypeInfo::toArray() const
{
	return std::nullopt;
}
