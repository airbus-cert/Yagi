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
	Datatype* TypeManager::findById(const string& n, uint8 id, int4 sz)
	{
		auto cached = findByIdLocal(n, id);

		if (cached != nullptr)
		{
			return cached;
		}

		auto type = m_archi->getTypeInfoFactory().build(n);
		
		if (!type.has_value())
		{
			throw UnknownTypeError(n);
		}

		return parseTypeInfo(*(type.value()));
	}

	/**********************************************************************/
	TypeCode* TypeManager::parseFunc(const FuncInfo& typeInfo)
	{
		auto prototype = typeInfo.getFuncPrototype();
		Datatype* retType = getTypeVoid();
		std::vector<Datatype*> paramType;

		if (prototype.size() > 0)
		{
			retType = findByTypeInfo(*(prototype.front()));

			auto paramIter = prototype.begin();
			paramIter++;
			std::transform(paramIter, prototype.end(), std::back_inserter(paramType),
				[this](const std::unique_ptr<TypeInfo>& type)
				{
					return findByTypeInfo(*type);
				}
			);
		}

		// handle calling convention conversion
		std::string cc = "__stdcall";
		try
		{
			cc = typeInfo.getCallingConv();
		}
		catch (UnknownCallingConvention&)
		{}
			
		if (!glb->hasModel(cc))
		{
			// by default we check configuration as calling convention
			cc = m_archi->getDefaultCC();
			if (!glb->hasModel(cc))
			{
				cc = (*m_archi->protoModels.begin()).first;
			}
			m_archi->getLogger().info("use ", cc, std::string("as default calling convention for "), typeInfo.getName());
		}

		auto newType = getTypeCode(glb->getModel(cc), retType, paramType, typeInfo.isDotDotDot());
		setName(newType, typeInfo.getName());
		return newType;
	}

	/**********************************************************************/
	Datatype* TypeManager::parseTypeInfo(const TypeInfo& typeInfo)
	{
		auto name = typeInfo.getName();

		auto ptrType = typeInfo.toPtr();
		if (ptrType.has_value())
		{
			auto ct = getTypePointer(
				glb->getDefaultCodeSpace()->getAddrSize(),
				findByTypeInfo(*ptrType.value()->getPointedObject()),
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

		if (typeInfo.isUnicode())
		{
			auto ct = new TypeUnicode(name, typeInfo.getSize(), TYPE_INT);
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

		auto structType = typeInfo.toStruct();
		if (structType.has_value())
		{
			auto ct = getTypeStruct(name);
			auto fields = structType.value()->getFields();
			std::vector<TypeField> result;
			std::transform(fields.begin(), fields.end(), std::back_inserter(result),
				[this](const TypeStructField& info)
				{
					return TypeField{ static_cast<int4>(info.offset), info.name, findByTypeInfo(*(info.type)) };
				}
			);
			setFields(result, ct, 0, 0);
			return ct;
		}

		if (typeInfo.isVoid())
		{
			size_t size = typeInfo.getSize();
			if (size == 0)
			{
				size = glb->getDefaultCodeSpace()->getAddrSize();
			}
			auto ct = new TypeBase(size, TYPE_VOID, name);
			setName(ct, name);
			return ct;
		}

		auto funcType = typeInfo.toFunc();
		if (funcType.has_value())
		{
			return parseFunc(*funcType.value());
		}

		auto arrayType = typeInfo.toArray();
		if (arrayType.has_value())
		{
			// if an array of size 0 doesn't handle by ghidra
			if (typeInfo.getSize() == 0)
			{
				return findByTypeInfo(*arrayType.value()->getPointedObject());
			}
			
			auto ct = getTypeArray(
				typeInfo.getSize(), 
				findByTypeInfo(*arrayType.value()->getPointedObject())
			);
			setName(ct, name);
			return ct;
		}

		auto unknownSize = typeInfo.getSize();
		if (unknownSize == (size_t)(-1))
		{
			unknownSize = glb->getDefaultCodeSpace()->getAddrSize();
		}
		auto ct = new TypeBase(unknownSize, TYPE_UNKNOWN, name);
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

	/**********************************************************************/
	void TypeManager::update(Funcdata& func)
	{
		auto typeInfo = m_archi->getTypeInfoFactory().build(func.getAddress().getOffset());
		if (!typeInfo.has_value())
		{
			return;
		}

		// handle the pointer to function type
		auto ptrType = typeInfo.value()->toPtr();
		if (ptrType.has_value())
		{
			typeInfo = ptrType.value()->getPointedObject();
		}

		auto funcType = typeInfo.value()->toFunc();

		// it's not a function
		if (!funcType.has_value())
		{
			m_archi->getLogger().error("Unable to update function ", func.getName(), std::string(" : symbol is not a function"));
			return;
		}

		auto type = parseFunc(*(funcType.value()));
		auto proto = type->getPrototype();

		PrototypePieces pieces;
		proto->getPieces(pieces);

		// Update with param name if possible
		auto newParamNames = funcType.value()->getFuncParamName();
		if (newParamNames.size() == pieces.innames.size())
		{
			pieces.innames = funcType.value()->getFuncParamName();
			func.getFuncProto().setPieces(pieces);
		}
	}
} // end of namespace yagi