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
		friend class IdaFuncInfo;
		friend class IdaStructInfo;
		friend class IdaPtrInfo;
		friend class IdaArrayInfo;

	protected:
		tinfo_t m_type;

	public:

		explicit IdaTypeInfo(tinfo_t idaType);

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		IdaTypeInfo(const IdaTypeInfo&) = default;
		IdaTypeInfo& operator=(const IdaTypeInfo&) = default;

		/*!
		 *	\brief	destructor
		 */
		virtual ~IdaTypeInfo() = default;

		size_t getSize() const override;
		std::string getName() const override;
		bool isInt() const override;
		bool isBool() const override;
		bool isFloat() const override;
		bool isVoid() const override;
		bool isConst() const override;
		bool isChar() const override;

		std::optional<std::unique_ptr<FuncInfo>> toFunc() const override;
		std::optional<std::unique_ptr<StructInfo>> toStruct() const override;
		std::optional<std::unique_ptr<PtrInfo>> toPtr() const override;
		std::optional<std::unique_ptr<ArrayInfo>> toArray() const override;
	};

	class IdaFuncInfo : public FuncInfo
	{
	protected:
		IdaTypeInfo m_info;

	public:
		explicit IdaFuncInfo(IdaTypeInfo info);

		bool isDotDotDot() const override;
		const TypeInfo& getType() const override;
		/*!
		 *	\brief	Build a list of type
		 *			List[ReturnType, Param1Type, Param2Type, ...]
		 *			This is compatible with the proto model interface of ghidra
		 *	\return	List of type compatible with ghidra interface
		 */
		std::vector<std::unique_ptr<TypeInfo>> getFuncPrototype() const override;

		/*!
		 *	\brief	Build a list of name
		 *			List[ReturnName, Param1Name, Param2Name, ...]
		 *			This is compatible with the proto model interface of ghidra
		 *	\return	List of type compatible with ghidra interface
		 */
		std::vector<std::string> getFuncParamName() const override;
		std::string getCallingConv() const override;
	};

	class IdaStructInfo : public StructInfo
	{
	protected:
		IdaTypeInfo m_info;
	public:
		explicit IdaStructInfo(IdaTypeInfo info);
		~IdaStructInfo() = default;
		const TypeInfo& getType() const override;
		std::vector<TypeStructField> getFields() const override;
	};

	class IdaPtrInfo : public PtrInfo
	{
	protected:
		IdaTypeInfo m_info;
	public:
		explicit IdaPtrInfo(IdaTypeInfo info);
		~IdaPtrInfo() = default;
		const TypeInfo& getType() const override;
		std::unique_ptr<TypeInfo> getPointedObject() const override;
		
	};

	class IdaArrayInfo : public ArrayInfo
	{
	protected:
		IdaTypeInfo m_info;
	public:
		explicit IdaArrayInfo(IdaTypeInfo info);
		~IdaArrayInfo() = default;

		const TypeInfo& getType() const override;
		std::unique_ptr<TypeInfo> getPointedObject() const override;
		uint64_t getSize() const override;
	};

	class IdaTypeInfoFactory : public TypeInfoFactory
	{
	public:
		explicit IdaTypeInfoFactory() = default;
		virtual ~IdaTypeInfoFactory() = default;

		std::optional<std::unique_ptr<TypeInfo>> build(const std::string& name) override;
		std::optional<std::unique_ptr<TypeInfo>> build(uint64_t ea) override;

	};
}

#endif