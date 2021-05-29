#include "scope.hh"
#include "symbolinfo.hh"
#include "base.hh"
#include "exception.hh"
#include "typemanager.hh"

namespace yagi 
{
	IdaScope::IdaScope(uint8_t id, YagiArchitecture* architecture)
		: Scope(id, "", architecture, this), m_proxy(0, "", architecture, this)
	{}

	ScopeInternal* IdaScope::getProxy()
	{
		return &m_proxy;
	}

	Funcdata* IdaScope::findFunction(const Address& addr) const
	{
		// don't check cache to force symbol database update
		auto proxy = static_cast<IdaScope*>(glb->symboltab->getGlobalScope())->getProxy();

		auto result = proxy->findFunction(addr);
		if (result != nullptr)
		{
			return result;
		}

		auto data = static_cast<YagiArchitecture*>(glb)->getSymbolDatabase()->find(addr.getOffset());

		if (!data.has_value())
		{
			return nullptr;
		}

		// found a function
		auto sym = proxy->addFunction(addr, data.value()->getName());

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

		auto data = static_cast<YagiArchitecture*>(glb)->getSymbolDatabase()->find(addr.getOffset());
		if (data.has_value())
		{
			auto scope = glb->symboltab->getGlobalScope();
			auto name = data.value()->getName();
			Symbol* symbol = nullptr;

			switch (data.value()->getType())
			{
			case SymbolInfo::Type::Function:
				symbol = proxy->addFunction(addr, name);
				break;
			case SymbolInfo::Type::Import:
				symbol = proxy->addExternalRef(addr, addr, name);
				break;
			case SymbolInfo::Type::Label:
				symbol = proxy->addCodeLabel(addr, name);
				break;
			case SymbolInfo::Type::Other:
			{
				auto type = static_cast<YagiArchitecture*>(glb)->getTypeInfoFactory()->build(addr.getOffset());
				if (type.has_value())
				{
					symbol = proxy->addSymbol(name, static_cast<TypeManager*>(glb->types)->findByTypeInfo(*(type.value())));
				}
				else 
				{
					symbol = proxy->addSymbol(name, static_cast<TypeManager*>(glb->types)->getBase(size, TYPE_UNKNOWN));
				}
				break;
			}	
			default:
				return nullptr;
			}

			if (data.value()->isReadOnly())
			{
				proxy->setAttribute(symbol, Varnode::readonly);
			}

			return proxy->addMapPoint(symbol, addr, usepoint);
		}

		if (result != nullptr) {
			
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

		auto data = static_cast<YagiArchitecture*>(glb)->getSymbolDatabase()->find(addr.getOffset());
		if (!data.has_value() || !data.value()->isImport())
		{
			return nullptr;
		}

		return proxy->addExternalRef(addr, addr, data.value()->getName());
	}

	LabSymbol* IdaScope::findCodeLabel(const Address& addr) const
	{
		auto proxy = static_cast<IdaScope*>(glb->symboltab->getGlobalScope())->getProxy();
		auto result = proxy->findCodeLabel(addr);
		if (result != nullptr)
		{
			return result;
		}

		auto data = static_cast<YagiArchitecture*>(glb)->getSymbolDatabase()->find(addr.getOffset());
		if (!data.has_value() || !data.value()->isLabel())
		{
			return nullptr;
			
		}
		return proxy->addCodeLabel(addr, data.value()->getName());
	}

	bool IdaScope::isNameUsed(const std::string& name, const Scope* scope) const
	{
		return m_proxy.isNameUsed(name, scope);
	}

	Funcdata* IdaScope::resolveExternalRefFunction(ExternRefSymbol* sym) const
	{
		auto proxy = static_cast<IdaScope*>(glb->symboltab->getGlobalScope())->getProxy();
		auto data = static_cast<YagiArchitecture*>(glb)->getSymbolDatabase()->find(sym->getRefAddr().getOffset());
		if (data.has_value())
		{
			auto funcData = proxy->addFunction(sym->getRefAddr(), data.value()->getName())->getFunction();

			// Try to set model type
			try
			{
				static_cast<TypeManager*>(glb->types)->update(*funcData);
			}
			catch (Error& e)
			{
				static_cast<YagiArchitecture*>(glb)->getLogger().error(e.what());
			}
			
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
} // end of namespace yagi