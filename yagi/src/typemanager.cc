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
	TypeCode* TypeManager::parseFunc(const FuncInfo& typeInfo)
	{
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
		std::string cc = "__fastcall";
		try
		{
			cc = typeInfo.getCallingConv();
		}
		catch (UnknownCallingConvention&)
		{}
			
		if (!glb->hasModel(cc))
		{
			// by default we put __fastcall as calling convention
			cc = "__fastcall";
		}

		return getTypeCode(glb->getModel(cc), retType, paramType, typeInfo.isDotDotDot());
	}

	/**********************************************************************/
	Datatype* TypeManager::parseTypeInfo(const TypeInfo& typeInfo)
	{
		auto name = typeInfo.getName();

		auto ptrType = typeInfo.toPtr();
		if (ptrType.has_value())
		{
			auto ct = new TypePointer(
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
			auto ct = new TypeStruct(name);
			setName(ct, name);
			auto fields = structType.value()->getFields();
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

		auto funcType = typeInfo.toFunc();
		if (funcType.has_value())
		{
			return parseFunc(*funcType.value());
		}

		auto arrayType = typeInfo.toArray();
		if (arrayType.has_value())
		{
			if (typeInfo.getSize() > 0)
			{
				auto ct = new TypeArray(
					typeInfo.getSize(), 
					findByTypeInfo(*arrayType.value()->getPointedObject())
				);
				setName(ct, name);
				return ct;
			}
			// if an array of size 0 convert to pointer
			else {
				auto ct = new TypePointer(
					glb->getDefaultCodeSpace()->getAddrSize(),
					findByTypeInfo(*arrayType.value()->getPointedObject()),
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

	/**********************************************************************/
	void TypeManager::update(Funcdata& func)
	{
		auto typeInfo = m_archi->getTypeInfoFactory()->build(func.getAddress().getOffset());
		if (!typeInfo.has_value())
		{
			return;
		}

		auto funcType = typeInfo.value()->toFunc();

		// it's not a function
		if (!funcType.has_value())
		{
			throw SymbolIsNotAFunction(typeInfo.value()->getName());
		}

		auto type = parseFunc(*(funcType.value()));
		auto proto = type->getPrototype();

		PrototypePieces pieces;
		proto->getPieces(pieces);

		// Update with param name if possible
		pieces.innames = funcType.value()->getFuncParamName();
		func.getFuncProto().setPieces(pieces);

		if (func.getName() == "__alloca_probe")
		{
			// inject interface is only available through
			// XML API...
			std::stringstream ss; 
			func.getFuncProto().saveXml(ss);

			auto document = xml_tree(ss);
			Element inject(document->getRoot());
			inject.setName("inject");
			inject.addContent("alloca_probe", 0, 12);

			document->getRoot()->addChild(&inject);
			func.getFuncProto().restoreXml(document->getRoot(), m_archi);
		}
	}

} // end of namespace yagi