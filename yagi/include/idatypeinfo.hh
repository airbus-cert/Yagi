#ifndef __YAGI_TYPEINFO__
#define __YAGI_TYPEINFO__

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <functional>

#include <typeinf.hpp>
#include <idp.hpp>

#include "typeinfo.hh"
#include "exception.hh"


namespace yagi 
{
	class IdaTypeInfo : public TypeInfo
	{
	protected:
		tinfo_t m_type;

	public:

		explicit IdaTypeInfo(tinfo_t idaType);
		/*!
		 *	\brief	destructor
		 */
		virtual ~IdaTypeInfo();

		/*!
		 *	\brief	Build a list of type
		 *			List[ReturnType, Param1Type, Param2Type, ...]
		 *			This is compatible with the proto model interface of ghidra
		 *	\return	List of type compatible with ghidra interface
		 */
		std::vector<std::unique_ptr<TypeInfo>> getFuncPrototype() const;

		/*!
		 *	\brief	Build a list of name
		 *			List[ReturnName, Param1Name, Param2Name, ...]
		 *			This is compatible with the proto model interface of ghidra
		 *	\return	List of type compatible with ghidra interface
		 */
		std::vector<std::string> getFuncParamName() const;

		size_t getSize() const;
		std::string getName() const;
		bool isInt() const;
		bool isPtr() const;
		bool isBool() const;
		bool isFloat() const;
		bool isStruct() const;
		bool isDotDotDot() const;
		bool isVoid() const;
		bool isFunc() const;
		bool isConst() const;
		bool isChar() const;
		bool isArray() const;

		std::unique_ptr<TypeInfo> getPointedObject() const;
		std::vector<TypeStructField> getFields() const;
		uint64_t getArraySize() const;
		std::string getCallingConv() const;
	};

	class IdaTypeInfoFactory : public TypeInfoFactory
	{
	public:
		explicit IdaTypeInfoFactory();
		virtual ~IdaTypeInfoFactory();

		std::optional<std::unique_ptr<TypeInfo>> build(const std::string& name) override;
		std::optional<std::unique_ptr<TypeInfo>> build(uint64_t ea) override;

	};
}

#endif