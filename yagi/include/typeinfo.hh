#ifndef __YAGI_TYPEFACTORY__
#define __YAGI_TYPEFACTORY__

#include <string>
#include <optional>
#include <memory>
#include <vector>

namespace yagi 
{

	// forward declaration
	class TypeInfo;

	struct TypeStructField
	{
		uint64_t offset;
		std::string name;
		std::unique_ptr<TypeInfo> type;
	};

	class TypeInfo
	{
	public:
		virtual std::vector<std::unique_ptr<TypeInfo>> getFuncPrototype() const = 0;
		virtual std::vector<std::string> getFuncParamName() const = 0;

		virtual size_t getSize() const = 0;

		virtual std::string getName() const = 0;

		virtual bool isInt() const = 0;
		virtual bool isPtr() const = 0;
		virtual bool isBool() const = 0;
		virtual bool isFloat() const = 0;
		virtual bool isStruct() const = 0;
		virtual bool isDotDotDot() const = 0;
		virtual bool isVoid() const = 0;
		virtual bool isFunc() const = 0;
		virtual std::vector<TypeStructField> getFields() const = 0;
		virtual std::unique_ptr<TypeInfo> getPointedObject() const = 0;
	};

	class TypeInfoFactory
	{
	public:
		virtual ~TypeInfoFactory() {}

		virtual std::optional<std::unique_ptr<TypeInfo>> build(const std::string& name) = 0;
		virtual std::optional<std::unique_ptr<TypeInfo>> build(uint64_t ea) = 0;
	};
}

#endif