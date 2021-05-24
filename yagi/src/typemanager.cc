#include "typemanager.hh"
#include "base.hh"
#include "exception.hh"

#include <regex>

namespace yagi 
{
	/**********************************************************************/
	TypeManager::TypeManager(YagiArchitecture* architecture)
		: ::TypeFactory(architecture), m_archi(architecture)
	{

	}

	/**********************************************************************/
	TypeManager::~TypeManager()
	{

	}

	/**********************************************************************/
	Datatype* TypeManager::findById(const string& n, uint8 id)
	{
		// find in cache
		auto result = TypeFactory::findById(n, id);
		if (result != nullptr)
		{
			return result;
		}

		auto type = m_archi->getTypeInfoFactory()->build(n);
		
		if (!type.has_value())
		{
			throw UnknownTypeError(n);
		}

		return parseTypeInfo(*(type.value()));
	}

	/**********************************************************************/
	TypeCode* TypeManager::parseFunc(const TypeInfo& typeInfo)
	{
		if (!typeInfo.isFunc())
		{
			throw InvalidType(typeInfo.getName());
		}

		auto prototype = typeInfo.getFuncPrototype();
		auto retType = findByTypeInfo(*(prototype.front()));
		std::vector<Datatype*> paramType;
		auto paramIter = prototype.begin();
		paramIter++;
		std::transform(paramIter, prototype.end(), std::back_inserter(paramType),
			[this](const std::unique_ptr<TypeInfo>& type)
			{
				return findByTypeInfo(*type);
			}
		);

		// handle calling convention conversion
		auto cc = typeInfo.getCallingConv();
		if (!glb->hasModel(cc))
		{
			// convert cdecl convention into thiscall ghidra
			if (cc == "__cdecl")
			{
				cc = "__thiscall";
			}
			else 
			{
				throw UnknownCallingConvention(typeInfo.getName());
			}
		}
		return getTypeCode(glb->getModel(cc), retType, paramType, typeInfo.isDotDotDot());
	}

	/**********************************************************************/
	Datatype* TypeManager::parseTypeInfo(const TypeInfo& typeInfo)
	{
		auto name = typeInfo.getName();

		if (typeInfo.isPtr())
		{
			auto ct = new TypePointer(
				glb->getDefaultCodeSpace()->getAddrSize(),
				findByTypeInfo(*typeInfo.getPointedObject()),
				glb->getDefaultCodeSpace()->getWordSize()
			);
			setName(ct, name);
			return ct;
		}

		if (typeInfo.isBool())
		{
			auto ct = new TypeBase(typeInfo.getSize(), TYPE_BOOL, name);
			setName(ct, name);
			return ct;
		}

		if (typeInfo.isChar())
		{
			auto ct = new TypeChar(name);
			setName(ct, name);
			return ct;
		}

		if (typeInfo.isInt())
		{
			auto ct = new TypeBase(typeInfo.getSize(), TYPE_INT, name);
			setName(ct, name);
			return ct;
		}

		if (typeInfo.isFloat())
		{
			auto ct = new TypeBase(typeInfo.getSize(), TYPE_FLOAT, name);
			setName(ct, name);
			return ct;
		}

		if (typeInfo.isStruct())
		{
			auto ct = new TypeStruct(name);
			setName(ct, name);
			auto fields = typeInfo.getFields();
			std::vector<TypeField> result;
			std::transform(fields.begin(), fields.end(), std::back_inserter(result),
				[this](const TypeStructField& info)
				{
					return TypeField { static_cast<int4>(info.offset), info.name, findByTypeInfo(*(info.type)) };
				}
			);
			setFields(result, ct, 0, 0);
			return ct;
		}

		if (typeInfo.isVoid())
		{
			auto ct = new TypeBase(typeInfo.getSize(), TYPE_VOID, name);
			setName(ct, name);
			return ct;
		}

		if (typeInfo.isFunc())
		{
			return parseFunc(typeInfo);
		}

		if (typeInfo.isArray())
		{
			if (typeInfo.getSize() > 0)
			{
				auto ct = new TypeArray(
					typeInfo.getSize(), 
					findByTypeInfo(*typeInfo.getPointedObject())
				);
				setName(ct, name);
				return ct;
			}
			// if an array of size 0 convert to pointer
			else {
				auto ct = new TypePointer(
					glb->getDefaultCodeSpace()->getAddrSize(),
					findByTypeInfo(*typeInfo.getPointedObject()),
					glb->getDefaultCodeSpace()->getWordSize()
				);
				setName(ct, name);
				return ct;
			}
		}

		auto ct = new TypeBase(glb->getDefaultCodeSpace()->getAddrSize(), TYPE_UNKNOWN, name);
		setName(ct, name);
		return ct;
	}

	/**********************************************************************/
	Datatype* TypeManager::findByTypeInfo(const TypeInfo& typeInfo)
	{
		try
		{
			return findByName(typeInfo.getName());
		}
		catch (UnknownTypeError&) {}

		return parseTypeInfo(typeInfo);
	}

	void TypeManager::update(Funcdata& func)
	{
		auto typeInfo = m_archi->getTypeInfoFactory()->build(func.getAddress().getOffset());
		if (!typeInfo.has_value())
		{
			return;
		}

		auto type = parseFunc(*(typeInfo.value()));

		PrototypePieces pieces;
		type->getPrototype()->getPieces(pieces);
		func.getFuncProto().setPieces(pieces);

		// maybe IDA didn't finish to analyze it
		// wait for ghidra type inference
		if (pieces.intypes.size() == 0) {
			func.getFuncProto().setInputLock(false);
		}
	}

} // end of namespace yagi