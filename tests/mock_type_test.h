#ifndef __YAGI_MOCK_TYPE_TEST__
#define __YAGI_MOCK_TYPE_TEST__

#include "typeinfo.hh"
#include <functional>
#include <optional>
#include <memory>
#include <string>

using MockTypeInfoFactoryFindAddress = std::function<std::optional<std::unique_ptr<yagi::TypeInfo>>(uint64_t)>;
using MockTypeInfoFactoryFindName = std::function<std::optional<std::unique_ptr<yagi::TypeInfo>>(const std::string&)>;
class MockTypeInfoFactory : public yagi::TypeInfoFactory
{
protected:
	MockTypeInfoFactoryFindAddress m_findAddressCallback;
	MockTypeInfoFactoryFindName m_findNameCallback;

public:
	explicit MockTypeInfoFactory(MockTypeInfoFactoryFindAddress findAddressCallback, MockTypeInfoFactoryFindName findNameCallback)
		: m_findAddressCallback{ findAddressCallback }, m_findNameCallback{ findNameCallback }
	{}

	std::optional<std::unique_ptr<yagi::TypeInfo>> build(uint64_t ea) override
	{
		return m_findAddressCallback(ea);
	}

	std::optional<std::unique_ptr<yagi::TypeInfo>> build(const std::string& name) override
	{
		return m_findNameCallback(name);
	}
};

class MockTypeInfo;

class MockFuncInfo : public yagi::FuncInfo
{
protected:
	bool m_isDotDotDot;
	std::string m_callingConvention;
	std::vector<MockTypeInfo> m_prototype;
	std::vector<std::string> m_paramName;

public:
	MockFuncInfo(bool isDotDotDot, std::string callingConvention, std::vector<MockTypeInfo> prototype, std::vector<std::string> paramName);

	bool isDotDotDot() const;
	std::vector<std::unique_ptr<yagi::TypeInfo>> getFuncPrototype() const;

	std::vector<std::string> getFuncParamName() const;
	std::string getCallingConv() const;
	std::string getName() const;
};

class MockTypeInfo : public yagi::TypeInfo
{
protected:
	size_t m_size;
	std::string m_name;
	bool m_isInt;
	bool m_isBool;
	bool m_isFloat;
	bool m_isVoid;
	bool m_isConst;
	bool m_isChar;
	bool m_isUnicode;

	std::optional<MockFuncInfo> m_funcInfo;

public:

	MockTypeInfo(size_t size, std::string name, bool isInt, bool isBool, bool isFloat, bool isVoid, bool isConst, bool isChar, bool isUnicode);
	MockTypeInfo(size_t size, std::string name, MockFuncInfo funcInfo);

	MockTypeInfo(const MockTypeInfo&) = default;
	MockTypeInfo& operator=(const MockTypeInfo&) = default;

	size_t getSize() const;
	std::string getName() const;
	std::string getRawName() const;
	bool isInt() const;
	bool isBool() const;
	bool isFloat() const;
	bool isVoid() const;
	bool isConst() const;
	bool isChar() const;
	bool isUnicode() const;

	std::optional<std::unique_ptr<yagi::FuncInfo>> toFunc() const;
	std::optional<std::unique_ptr<yagi::StructInfo>> toStruct() const;
	std::optional<std::unique_ptr<yagi::PtrInfo>> toPtr() const;
	std::optional<std::unique_ptr<yagi::ArrayInfo>> toArray() const;
};

#endif
