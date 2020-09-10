#include "scope.hh"
#include "symbolinfo.hh"
#include "base.hh"
#include "error.hh"
#include "typemanager.hh"

namespace yagi 
{
	IdaScope::IdaScope(YagiArchitecture* architecture)
		: Scope("", architecture, this), m_proxy("", architecture, this)
	{}

	ScopeInternal* IdaScope::getProxy()
	{
		return &m_proxy;
	}

	Funcdata* IdaScope::findFunction(const Address& addr) const
	{
		auto proxy = static_cast<IdaScope*>(glb->symboltab->getGlobalScope())->getProxy();
		auto result = proxy->findFunction(addr);
		if (result != nullptr)
		{
			return result;
		}

		auto data = SymbolInfo::find(addr.getOffset(), static_cast<YagiArchitecture*>(glb)->getSymbolDatabase());

		if (!data.has_value())
		{
			return nullptr;
		}

		// found a function
		auto sym = proxy->addFunction(addr, data.value().getName());

		auto funcData = sym->getFunction();

		// Try to set model type
		static_cast<TypeManager*>(glb->types)->update(*funcData);

		return funcData;
	}

	SymbolEntry* IdaScope::findAddr(const Address& addr, const Address& usepoint) const
	{
		//auto proxy = static_cast<IdaScope*>(glb->symboltab->getGlobalScope())->getProxy();
		//return proxy->findAddr(addr, usepoint);
		return nullptr;
	}

	SymbolEntry* IdaScope::findContainer(const Address& addr, int4 size, const Address& usepoint) const
	{
		auto proxy = static_cast<IdaScope*>(glb->symboltab->getGlobalScope())->getProxy();
		auto result = proxy->findContainer(addr, size, usepoint);
		if (result != nullptr)
		{
			return result;
		}

		auto data = SymbolInfo::load(addr.getOffset(), static_cast<YagiArchitecture*>(glb)->getSymbolDatabase());
		if (data.has_value())
		{
			auto scope = glb->symboltab->getGlobalScope();

			switch (data.value().getType())
			{
			case SymbolInfo::Type::Function:
				return proxy->addMapPoint(proxy->addFunction(addr, data.value().getName()), addr, usepoint);
			case SymbolInfo::Type::Import:
				return proxy->addMapPoint(proxy->addExternalRef(addr, addr, data.value().getName()), addr, usepoint);
			case SymbolInfo::Type::Label:
				return proxy->addMapPoint(proxy->addCodeLabel(addr, data.value().getName()), addr, usepoint);
			case SymbolInfo::Type::Other:
			{
				auto type = static_cast<YagiArchitecture*>(glb)->getTypeInfoFactory().build(addr.getOffset());
				return proxy->addMapPoint(proxy->addSymbol(data.value().getName(), static_cast<TypeManager*>(glb->types)->findByTypeInfo(*(type.value()))), addr, usepoint);
			}	
			default:
				break;
			}
		}

		return nullptr;
	}

	SymbolEntry* IdaScope::findClosestFit(const Address& addr, int4 size, const Address& usepoint) const
	{
		return m_proxy.findClosestFit(addr, size, usepoint);
	}

	ExternRefSymbol* IdaScope::findExternalRef(const Address& addr) const
	{
		auto proxy = static_cast<IdaScope*>(glb->symboltab->getGlobalScope())->getProxy();
		auto result = proxy->findExternalRef(addr);
		if (result != nullptr)
		{
			return result;
		}

		auto data = SymbolInfo::load(addr.getOffset(), static_cast<YagiArchitecture*>(glb)->getSymbolDatabase());
		if (!data.has_value() || !data.value().isImport())
		{
			return nullptr;
		}

		return proxy->addExternalRef(addr, addr, data.value().getName());
	}

	LabSymbol* IdaScope::findCodeLabel(const Address& addr) const
	{
		auto proxy = static_cast<IdaScope*>(glb->symboltab->getGlobalScope())->getProxy();
		auto result = proxy->findCodeLabel(addr);
		if (result != nullptr)
		{
			return result;
		}

		auto data = SymbolInfo::load(addr.getOffset(), static_cast<YagiArchitecture*>(glb)->getSymbolDatabase());
		if (!data.has_value() || !data.value().isLabel())
		{
			return nullptr;
			
		}
		return proxy->addCodeLabel(addr, data.value().getName());
	}

	bool IdaScope::isNameUsed(const std::string& name, const Scope* scope) const
	{
		return m_proxy.isNameUsed(name, scope);
	}

	Funcdata* IdaScope::resolveExternalRefFunction(ExternRefSymbol* sym) const
	{
		auto proxy = static_cast<IdaScope*>(glb->symboltab->getGlobalScope())->getProxy();
		auto data = SymbolInfo::load(sym->getRefAddr().getOffset(), static_cast<YagiArchitecture*>(glb)->getSymbolDatabase());
		if (data.has_value())
		{
			auto funcData = proxy->addFunction(sym->getRefAddr(), data.value().getName())->getFunction();

			// Try to set model type
			static_cast<TypeManager*>(glb->types)->update(*funcData);
			
			return funcData;
		}
		return nullptr;
	}

	/*!
	 *	\brief	function called most of time when a global variable is found
	 *			Symbol generated will be added by the decompiler
	 *			and retrive by the findContainer function
	 *			This is typically the case for import function in windows world
	 */
	string IdaScope::buildVariableName(const Address& addr, const Address& pc, Datatype* ct, int4& index, uint4 flags) const
	{
		return "unk_" + to_hex(addr.getOffset());
	}

	SymbolEntry* IdaScope::addSymbol(const string& name, Datatype* ct, const Address& addr, const Address& usepoint)
	{
		return m_proxy.addSymbol(name, ct, addr, usepoint);
	}
} // end of namespace gaip