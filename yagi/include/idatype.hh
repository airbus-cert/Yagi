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
	/*!
	 * \brief	Implementation of the TypeInfo Interface for IDA
	 */
	class IdaTypeInfo : public TypeInfo
	{
		friend class IdaFuncInfo;
		friend class IdaStructInfo;
		friend class IdaPtrInfo;
		friend class IdaArrayInfo;

	protected:
		/*!
		 * \brief	IDA type info 
		 */
		tinfo_t m_type;

	public:

		/*!
		 * \brief	ctor 
		 */
		explicit IdaTypeInfo(tinfo_t idaType);

		/*!
		 *	\brief	destructor
		 */
		virtual ~IdaTypeInfo() = default;

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		IdaTypeInfo(const IdaTypeInfo&) = default;
		IdaTypeInfo& operator=(const IdaTypeInfo&) = default;

		/*!
		 *	\brief	moving is allowed
		 */
		IdaTypeInfo(IdaTypeInfo&&) noexcept = default;
		IdaTypeInfo& operator=(IdaTypeInfo&&) noexcept = default;

		/*!
		 * \brief	Size in bytes of the type
		 * \return	Size in bytes of the type
		 */
		size_t getSize() const override;

		/*!
		 * \brief	The name of the type
		 * \param	return the name of the type
		 */
		std::string getName() const override;


		/*!
		 * \brief	Is the type is linked to an integer
		 * \warning	Any interger type, uint16 uin8 etc..., are integer
		 * \return	True if type is an integer
		 */
		bool isInt() const override;

		/*!
		 * \brief	In c++ boolean exist and are typed
		 * \return	True if associated type is a boolean
		 */
		bool isBool() const override;

		/*!
		 * \brief	Is A floating point type
		 * \return	True if assocate is a single precision floating point
		 */
		bool isFloat() const override;

		/*!
		 * \brief	Is it the void type
		 * \warning	A void type can have a zero size
		 *			If it's the case use the default addresse size
		 * \return	True if the associated type is a void
		 */
		bool isVoid() const override;

		/*!
		 * \brief	Is the type have a const representation
		 * \return	True if the type is const
		 */
		bool isConst() const override;

		/*!
		 * \brief	Is it a char type
		 * \warning	It's also an integer
		 * \return	True if it's a char type
		 */
		bool isChar() const override;

		/*!
		 * \brief	Is it an unicode type
		 * \return	true if it's an unicode
		 */
		bool isUnicode() const;

		/*!
		 * \brief	Convert the current type, if it's possible
		 *			into a function type info
		 * \return	If it's a function type give the associated object
		 *			If it's not, the nullopt
		 */
		std::optional<std::unique_ptr<FuncInfo>> toFunc() const override;

		/*!
		 * \brief	Convert the current type, if it's possible
		 *			into a Struct type info
		 * \return	If it's a struct type give the associated object
		 *			If it's not, the nullopt
		 */
		std::optional<std::unique_ptr<StructInfo>> toStruct() const override;

		/*!
		 * \brief	Convert the current type, if it's possible
		 *			into a Pointer type info
		 * \return	If it's a pointer type give the associated object
		 *			If it's not, the nullopt
		 */
		std::optional<std::unique_ptr<PtrInfo>> toPtr() const override;

		/*!
		 * \brief	Convert the current type, if it's possible
		 *			into an Array type info
		 * \return	If it's an Array type give the associated object
		 *			If it's not, the nullopt
		 */
		std::optional<std::unique_ptr<ArrayInfo>> toArray() const override;
	};

	/*!
	 * \brief	The implementation of the FuncInfo for IDA 
	 */
	class IdaFuncInfo : public FuncInfo
	{
	protected:

		/*!
		 * \brief	The inner IDA type informations 
		 */
		IdaTypeInfo m_info;

	public:
		explicit IdaFuncInfo(IdaTypeInfo info);
		~IdaFuncInfo() = default;

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		IdaFuncInfo(const IdaFuncInfo&) = default;
		IdaFuncInfo& operator=(const IdaFuncInfo&) = default;

		/*!
		 *	\brief	moving is allowed
		 */
		IdaFuncInfo(IdaFuncInfo&&) noexcept = default;
		IdaFuncInfo& operator=(IdaFuncInfo&&) noexcept = default;

		/*!
		 * \brief	Is it a varargs function
		 * \return	True if it's a varargs function
		 */
		bool isDotDotDot() const override;

		/*!
		 * \brief	return the inner type
		 * \return	the inner type
		 */
		const TypeInfo& getType() const;

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

		/*!
		 * \brief	Get the calling convention of the function 
		 */
		std::string getCallingConv() const override;

		/*!
		 * \brief	Return name of the function
		 */
		std::string getName() const override;
	};

	/*!
	 * \brief	Implementation of the StrucInfo for IDA 
	 */
	class IdaStructInfo : public StructInfo
	{
	protected:
		/*!
		 * \brief	The inner IDA type 
		 */
		IdaTypeInfo m_info;
	public:
		explicit IdaStructInfo(IdaTypeInfo info);
		~IdaStructInfo() = default;

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		IdaStructInfo(const IdaStructInfo&) = default;
		IdaStructInfo& operator=(const IdaStructInfo&) = default;

		/*!
		 *	\brief	moving is allowed
		 */
		IdaStructInfo(IdaStructInfo&&) noexcept = default;
		IdaStructInfo& operator=(IdaStructInfo&&) noexcept = default;

		/*!
		 * \brief	return the inner IDA type
		 * \return	the inner IDA type
		 */
		const TypeInfo& getType() const;

		/*!
		 * \brief	Return the list of structure fields 
		 */
		std::vector<TypeStructField> getFields() const override;
	};

	/*!
	 * \brief	Implementation of the Pointer Information type interface 
	 */
	class IdaPtrInfo : public PtrInfo
	{
	protected:
		/*!
		 * \brief	inner IDA type information
		 */
		IdaTypeInfo m_info;

	public:

		explicit IdaPtrInfo(IdaTypeInfo info);
		~IdaPtrInfo() = default;

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		IdaPtrInfo(const IdaPtrInfo&) = default;
		IdaPtrInfo& operator=(const IdaPtrInfo&) = default;

		/*!
		 *	\brief	moving is allowed
		 */
		IdaPtrInfo(IdaPtrInfo&&) noexcept = default;
		IdaPtrInfo& operator=(IdaPtrInfo&&) noexcept = default;

		/*!
		 * \brief	Return the inner type
		 * \return	The inner IDA type
		 */
		const TypeInfo& getType() const;

		/*!
		 * \brief	Get the type information of the pointed object
		 * \return	The type information of the pointed object
		 */
		std::unique_ptr<TypeInfo> getPointedObject() const override;
		
	};

	/*!
	 * \brief	Implementation of the Array information interface 
	 */
	class IdaArrayInfo : public ArrayInfo
	{
	protected:
		/*!
		 * \brief	inner IDA type information
		 */
		IdaTypeInfo m_info;

	public:
		explicit IdaArrayInfo(IdaTypeInfo info);
		~IdaArrayInfo() = default;

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		IdaArrayInfo(const IdaArrayInfo&) = default;
		IdaArrayInfo& operator=(const IdaArrayInfo&) = default;

		/*!
		 *	\brief	moving is allowed
		 */
		IdaArrayInfo(IdaArrayInfo&&) noexcept = default;
		IdaArrayInfo& operator=(IdaArrayInfo&&) noexcept = default;

		/*!
		 * \brief	Get the inner type informations
		 */
		const TypeInfo& getType() const;

		/*!
		 * \brief	Get the type information of the Array elements
		 * \return	The type information of the Array elements
		 */
		std::unique_ptr<TypeInfo> getPointedObject() const override;

		/*!
		 * \brief	Get the number of element in the array
		 * \return	The number of element in the array
		 */
		uint64_t getSize() const override;
	};

	/*!
	 * \brief	Implementation of the Type factory interface for IDA
	 */
	class IdaTypeInfoFactory : public TypeInfoFactory
	{
	public:
		explicit IdaTypeInfoFactory() = default;
		virtual ~IdaTypeInfoFactory() = default;

		/*!
		 * \brief	Build a type information object from the type name
		 * \param	name	name of the type
		 * \return	if the type is found the TypeInfo object
		 *			if not return nullopt
		 */
		std::optional<std::unique_ptr<TypeInfo>> build(const std::string& name) override;

		/*!
		 * \brief	Try to find a type information at a specific address
		 * \param	ea	address of the type
		 * \return	if the type is found the TypeInfo object
		 *			if not return nullopt
		 */
		std::optional<std::unique_ptr<TypeInfo>> build(uint64_t ea) override;

		std::optional<std::unique_ptr<TypeInfo>> build(tinfo_t info);
		std::optional<std::unique_ptr<TypeInfo>> build_decl(const std::string& name);
	};
}

#endif