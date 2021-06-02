#include "scope.hh"
#include "symbolinfo.hh"
#include "base.hh"
#include "exception.hh"
#include "typemanager.hh"

#define UNIMPLEMENTED throw UnImplementedFunction(__func__)

namespace yagi 
{
	/**********************************************************************/
	YagiScope::YagiScope(uint8_t id, YagiArchitecture* architecture)
		: Scope(id, "", architecture, this), m_proxy(0, "", architecture, this)
	{}

	/**********************************************************************/
	ScopeInternal* YagiScope::getProxy()
	{
		return &m_proxy;
	}

	/**********************************************************************/
	Funcdata* YagiScope::findFunction(const Address& addr) const
	{
		// don't check cache to force symbol database update
		auto proxy = static_cast<YagiScope*>(glb->symboltab->getGlobalScope())->getProxy();

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

	/**********************************************************************/
	SymbolEntry* YagiScope::findAddr(const Address& addr, const Address& usepoint) const
	{
		return nullptr;
	}

	/**********************************************************************/
	SymbolEntry* YagiScope::findContainer(const Address& addr, int4 size, const Address& usepoint) const
	{
		auto proxy = static_cast<YagiScope*>(glb->symboltab->getGlobalScope())->getProxy();
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

	/**********************************************************************/
	SymbolEntry* YagiScope::findClosestFit(const Address& addr, int4 size, const Address& usepoint) const
	{
		return m_proxy.findClosestFit(addr, size, usepoint);
	}

	/**********************************************************************/
	ExternRefSymbol* YagiScope::findExternalRef(const Address& addr) const
	{
		auto proxy = static_cast<YagiScope*>(glb->symboltab->getGlobalScope())->getProxy();
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

	/**********************************************************************/
	LabSymbol* YagiScope::findCodeLabel(const Address& addr) const
	{
		auto proxy = static_cast<YagiScope*>(glb->symboltab->getGlobalScope())->getProxy();
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

	/**********************************************************************/
	bool YagiScope::isNameUsed(const std::string& name, const Scope* scope) const
	{
		return m_proxy.isNameUsed(name, scope);
	}

	/**********************************************************************/
	Funcdata* YagiScope::resolveExternalRefFunction(ExternRefSymbol* sym) const
	{
		auto proxy = static_cast<YagiScope*>(glb->symboltab->getGlobalScope())->getProxy();
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

	/**********************************************************************/
	/*!
	 *	\brief	function called most of time when a global variable is found
	 *			Symbol generated will be added by the decompiler
	 *			and retrive by the findContainer function
	 *			This is typically the case for import function in windows world
	 */
	string YagiScope::buildVariableName(const Address& addr, const Address& pc, Datatype* ct, int4& index, uint4 flags) const
	{
		return "unk_" + to_hex(addr.getOffset());
	}

	/**********************************************************************/
	SymbolEntry* YagiScope::addSymbol(const string& name, Datatype* ct, const Address& addr, const Address& usepoint)
	{
		return m_proxy.addSymbol(name, ct, addr, usepoint);
	}

	/**********************************************************************/
	void YagiScope::addSymbolInternal(Symbol* sym) 
	{ 
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	SymbolEntry* YagiScope::addMapInternal(Symbol* sym, uint4 exfl, const Address& addr, int4 off, int4 sz, const RangeList& uselim)
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	Scope* YagiScope::buildSubScope(uint8 id, const string& nm)
	{ 
		return new ScopeInternal(id, nm, this->glb); 
	}

	/**********************************************************************/
	void YagiScope::removeRange(AddrSpace* spc, uintb first, uintb last)
	{ 
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	SymbolEntry* YagiScope::addDynamicMapInternal(Symbol* sym, uint4 exfl, uint8 hash, int4 off, int4 sz, const RangeList& uselim)
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::clear(void)
	{ 
		m_proxy.clear(); 
	}

	/**********************************************************************/
	void YagiScope::adjustCaches(void)
	{ 
		m_proxy.adjustCaches(); 
	}

	/**********************************************************************/
	string YagiScope::buildUndefinedName(void) const
	{ 
		return m_proxy.buildUndefinedName(); 
	}

	/**********************************************************************/
	void YagiScope::setAttribute(Symbol* sym, uint4 attr)
	{ 
		m_proxy.setAttribute(sym, attr); 
	}

	/**********************************************************************/
	void YagiScope::clearAttribute(Symbol* sym, uint4 attr)
	{ 
		m_proxy.clearAttribute(sym, attr); 
	}

	/**********************************************************************/
	void YagiScope::setDisplayFormat(Symbol* sym, uint4 attr)
	{ 
		m_proxy.setDisplayFormat(sym, attr); 
	}

	/**********************************************************************/
	SymbolEntry* YagiScope::findOverlap(const Address& addr, int4 size) const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	SymbolEntry* YagiScope::findBefore(const Address& addr) const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	SymbolEntry* YagiScope::findAfter(const Address& addr) const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::findByName(const string& name, vector<Symbol*>& res) const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	MapIterator YagiScope::begin() const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	MapIterator YagiScope::end() const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	list<SymbolEntry>::const_iterator YagiScope::beginDynamic() const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	list<SymbolEntry>::const_iterator YagiScope::endDynamic() const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	list<SymbolEntry>::iterator YagiScope::beginDynamic()
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	list<SymbolEntry>::iterator YagiScope::endDynamic()
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::clearCategory(int4 cat)
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::clearUnlockedCategory(int4 cat)
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::clearUnlocked()
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::restrictScope(Funcdata* f)
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::removeSymbolMappings(Symbol* symbol)
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::removeSymbol(Symbol* symbol)
	{
		m_proxy.removeSymbol(symbol);
	}

	/**********************************************************************/
	void YagiScope::renameSymbol(Symbol* sym, const string& newname)
	{
		m_proxy.renameSymbol(sym, newname);
	}

	/**********************************************************************/
	void YagiScope::retypeSymbol(Symbol* sym, Datatype* ct)
	{
		m_proxy.retypeSymbol(sym, ct);
	}

	/**********************************************************************/
	string YagiScope::makeNameUnique(const string& nm) const
	{
		return m_proxy.makeNameUnique(nm);
	}

	/**********************************************************************/
	void YagiScope::saveXml(ostream& s) const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::restoreXml(const Element* el)
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::printEntries(ostream& s) const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	int4 YagiScope::getCategorySize(int4 cat) const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	Symbol* YagiScope::getCategorySymbol(int4 cat, int4 ind) const
	{
		UNIMPLEMENTED;
	}

	/**********************************************************************/
	void YagiScope::setCategory(Symbol* sym, int4 cat, int4 ind)
	{
		UNIMPLEMENTED;
	}

} // end of namespace yagi