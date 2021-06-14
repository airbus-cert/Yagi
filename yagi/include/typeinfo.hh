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

	/*!
	 * \brief	Structure informations
	 */
	class StructInfo
	{
	public:
		virtual ~StructInfo() = default;

		/*!
		 * \brief	get fields member of the structure
		 * \return	all type members with meta information of type declaration
		 */
		virtual std::vector<TypeStructField> getFields() const = 0;
	};

	/*!
	 * \brief	Type is pointer to another type
	 */
	class PtrInfo
	{
	public:
		virtual ~PtrInfo() = default;

		/*!
		 * \brief	compute the target type information
		 * \return	the pointed type information
		 */
		virtual std::unique_ptr<TypeInfo> getPointedObject() const = 0;
	};

	/*!
	 * \brief	Type information is an Array
	 */
	class ArrayInfo
	{
	public:
		virtual ~ArrayInfo() = default;

		/*!
		 * \brief	array is not very different from pointer type
		 *			But have a number of elements
		 * \return	type of one element of the array
		 */
		virtual std::unique_ptr<TypeInfo> getPointedObject() const = 0;

		/*!
		 * \brief	return the number of elements of the array
		 */
		virtual uint64_t getSize() const = 0;
	};

	/*!
	 * \brief	Function Type information interface 
	 */
	class FuncInfo
	{
	public:
		virtual ~FuncInfo() = default;

		/*!
		 * \brief	Is it a varargs function
		 * \return	True if it's a varargs function
		 */
		virtual bool isDotDotDot() const = 0;

		/*!
		 *	\brief	Build a list of type
		 *			List[ReturnType, Param1Type, Param2Type, ...]
		 *			This is compatible with the proto model interface of ghidra
		 *	\return	List of type compatible with ghidra interface
		 */
		virtual std::vector<std::unique_ptr<TypeInfo>> getFuncPrototype() const = 0;

		/*!
		 *	\brief	Build a list of name
		 *			List[ReturnName, Param1Name, Param2Name, ...]
		 *			This is compatible with the proto model interface of ghidra
		 *	\return	List of type compatible with ghidra interface
		 */
		virtual std::vector<std::string> getFuncParamName() const = 0;

		/*!
		 * \brief	Get the calling convention of the function
		 */
		virtual std::string getCallingConv() const = 0;

		/*!
		 * \brief	Return the name of the function
		 */
		virtual std::string getName() const = 0;
	};

	/*!
	 * \brief	Type information interface 
	 */
	class TypeInfo
	{
	public:
		/*!
		 * \brief	get the size of the the type in bytes 
		 */
		virtual size_t getSize() const = 0;

		/*!
		 * \brief	return the name of the type 
		 */
		virtual std::string getName() const = 0;

		/*!
		 * \brief	Is the type is linked to an integer
		 * \warning	Any interger type, uint16 uin8 etc..., are integer
		 * \return	True if type is an integer
		 */
		virtual bool isInt() const = 0;

		/*!
		 * \brief	In c++ boolean exist and are typed
		 * \return	True if associated type is a boolean
		 */
		virtual bool isBool() const = 0;

		/*!
		 * \brief	Is A floating point type
		 * \return	True if assocate is a single precision floating point
		 */
		virtual bool isFloat() const = 0;

		/*!
		 * \brief	Is it the void type
		 * \warning	A void type can have a zero size
		 *			If it's the case use the default addresse size
		 * \return	True if the associated type is a void
		 */
		virtual bool isVoid() const = 0;

		/*!
		 * \brief	Is the type have a const representation
		 * \return	True if the type is const
		 */
		virtual bool isConst() const = 0;

		/*!
		 * \brief	Is it a char type
		 * \warning	It's also an integer
		 * \return	True if it's a char type
		 */
		virtual bool isChar() const = 0;

		/*!
		 * \brief	Is it an unicode type
		 * \return	true if it's an unicode
		 */
		virtual bool isUnicode() const = 0;

		/*!
		 * \brief	Convert the current type, if it's possible
		 *			into a function type info
		 * \return	If it's a function type give the associated object
		 *			If it's not, the nullopt
		 */
		virtual std::optional<std::unique_ptr<FuncInfo>> toFunc() const = 0;

		/*!
		 * \brief	Convert the current type, if it's possible
		 *			into a Struct type info
		 * \return	If it's a struct type give the associated object
		 *			If it's not, the nullopt
		 */
		virtual std::optional<std::unique_ptr<StructInfo>> toStruct() const = 0;

		/*!
		 * \brief	Convert the current type, if it's possible
		 *			into a Pointer type info
		 * \return	If it's a pointer type give the associated object
		 *			If it's not, the nullopt
		 */
		virtual std::optional<std::unique_ptr<PtrInfo>> toPtr() const = 0;

		/*!
		 * \brief	Convert the current type, if it's possible
		 *			into an Array type info
		 * \return	If it's an Array type give the associated object
		 *			If it's not, the nullopt
		 */
		virtual std::optional<std::unique_ptr<ArrayInfo>> toArray() const = 0;
	
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