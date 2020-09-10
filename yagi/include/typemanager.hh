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

		YagiArchitecture* m_archi;

		Datatype* findById(const string& n, uint8 id) override;

	public:
		explicit TypeManager(YagiArchitecture* architecture);
		virtual ~TypeManager();

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

		TypeCode* parseFunc(const TypeInfo& typeInfo);
		Datatype* parseTypeInfo(const TypeInfo& typeInfo);

		Datatype* findByTypeInfo(const TypeInfo& typeInfo);

		void update(Funcdata& func);
	};
}

#endif