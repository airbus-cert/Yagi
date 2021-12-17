#ifndef __YAGI_TYPEMANAGER__
#define __YAGI_TYPEMANAGER__

#include <libdecomp.hh>
#include <vector>
#include <string>
#include <optional>
#include <map>
#include <string>

#include "yagiarchitecture.hh"

namespace yagi 
{
	/*!
	 *	\brief	Factory become a manager because it's based on the factory backend link to IDA
	 */
	class TypeManager : public TypeFactory
	{
	protected:

		/*!
		 * \brief	pointer the global architecture
		 */
		YagiArchitecture* m_archi;

		/*!
		 * \brief	find type by inner id
		 * \param	n	name of the type
		 * \param	id	id of the type
		 * \return	found type
		 */
		Datatype* findById(const string& n, uint8 id, int4 sz) override;

	public:
		/*!
		 * \brief	ctor
		 */
		explicit TypeManager(YagiArchitecture* architecture);

		virtual ~TypeManager() = default;

		/*!
		 *	\brief	Disable copy of IdaTypeFactory prefer moving
		 */
		TypeManager(const TypeManager&) = delete;
		TypeManager& operator=(const TypeManager&) = delete;

		/*!
		 *	\brief	Moving is allowed because unique_ptr allow it
		 *			and we use std map as container
		 */
		TypeManager(TypeManager&&) noexcept = default;
		TypeManager& operator=(TypeManager&&) noexcept = default;

		/*!
		 * \brief	Parse a function information type interface
		 *			Try to create a Ghidra type code
		 * \param	typeInfo	backend type information
		 */
		TypeCode* parseFunc(const FuncInfo& typeInfo);

		/*!
		 * \brief	parse a type information generic interface
		 *			to transform into ghidra type
		 * \param	backend type information interface
		 * \return	ghidra type
		 */
		Datatype* parseTypeInfo(const TypeInfo& typeInfo);

		/*!
		 * \brief	Find a type from typeinformation interface
		 * \param	typeInfo interface to find
		 * \return	ghidra type
		 */
		Datatype* findByTypeInfo(const TypeInfo& typeInfo);

		/*!
		 * \brief	update function information data
		 */
		void update(Funcdata& func);
	};
}

#endif