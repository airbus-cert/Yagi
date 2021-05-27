#ifndef __YAGI_SCOPE__
#define __YAGI_SCOPE__

#include <memory>
#include <map>
#include <libdecomp.hh>
#include "yagiarchitecture.hh"

namespace yagi 
{
	class IdaScope : public Scope
	{
	protected:


		ScopeInternal m_proxy;

		void addSymbolInternal(Symbol* sym) override { throw LowlevelError("addSymbolInternal should not be performed on ida scope"); }
		SymbolEntry* addMapInternal(Symbol* sym, uint4 exfl, const Address& addr, int4 off, int4 sz, const RangeList& uselim) override { throw LowlevelError("addMapInternal should not be performed on ida scope"); }


		Scope* buildSubScope(uint8 id, const string& nm) override { return new ScopeInternal(id, nm, glb); }
		void removeRange(AddrSpace* spc, uintb first, uintb last) override { throw LowlevelError("removeRange should not be performed on ida scope"); }
		SymbolEntry* addDynamicMapInternal(Symbol* sym, uint4 exfl, uint8 hash, int4 off, int4 sz, const RangeList& uselim) override { throw LowlevelError("addMap unimplemented"); }

	public:
		explicit IdaScope(uint8_t id, YagiArchitecture* architecture);

		virtual Funcdata* findFunction(const Address& addr) const override;
		string buildVariableName(const Address& addr, const Address& pc, Datatype* ct, int4& index, uint4 flags) const override;

		SymbolEntry* addSymbol(const string& name, Datatype* ct,
			const Address& addr, const Address& usepoint);

		ScopeInternal* getProxy();
		
		SymbolEntry* Scope::findAddr(const Address&, const Address&) const override;
		SymbolEntry* Scope::findContainer(const Address&, int4, const Address&) const override;
		SymbolEntry* Scope::findClosestFit(const Address&, int4, const Address&) const override;
		ExternRefSymbol* Scope::findExternalRef(const Address& addr) const override;
		LabSymbol* Scope::findCodeLabel(const Address&) const override;
		bool Scope::isNameUsed(const std::string&, const Scope*) const override;
		Funcdata* Scope::resolveExternalRefFunction(ExternRefSymbol* sym) const override;

		void clear(void) override { m_proxy.clear(); }
		void adjustCaches(void) override { m_proxy.adjustCaches(); }
		
		string buildUndefinedName(void) const override { return m_proxy.buildUndefinedName(); }
		void setAttribute(Symbol* sym, uint4 attr) override { m_proxy.setAttribute(sym, attr); }
		void clearAttribute(Symbol* sym, uint4 attr) override { m_proxy.clearAttribute(sym, attr); }
		void setDisplayFormat(Symbol* sym, uint4 attr) override { m_proxy.setDisplayFormat(sym, attr); }

		SymbolEntry* findOverlap(const Address& addr, int4 size) const { throw LowlevelError("findOverlap unimplemented"); }
		SymbolEntry* findBefore(const Address& addr) const { throw LowlevelError("findBefore unimplemented"); }
		SymbolEntry* findAfter(const Address& addr) const { throw LowlevelError("findAfter unimplemented"); }
		void findByName(const string& name, vector<Symbol*>& res) const { throw LowlevelError("findByName unimplemented"); }
		MapIterator begin() const override { throw LowlevelError("begin unimplemented"); }
		MapIterator end() const override { throw LowlevelError("end unimplemented"); }
		list<SymbolEntry>::const_iterator beginDynamic() const override { throw LowlevelError("beginDynamic unimplemented"); }
		list<SymbolEntry>::const_iterator endDynamic() const override { throw LowlevelError("endDynamic unimplemented"); }
		list<SymbolEntry>::iterator beginDynamic() override { throw LowlevelError("beginDynamic unimplemented"); }
		list<SymbolEntry>::iterator endDynamic() override { throw LowlevelError("endDynamic unimplemented"); }
		void clearCategory(int4 cat) override { throw LowlevelError("clearCategory unimplemented"); }
		void clearUnlockedCategory(int4 cat) override { throw LowlevelError("clearUnlockedCategory unimplemented"); }
		void clearUnlocked() override { throw LowlevelError("clearUnlocked unimplemented"); }
		void restrictScope(Funcdata* f) override { throw LowlevelError("restrictScope unimplemented"); }
		void removeSymbolMappings(Symbol* symbol) override { throw LowlevelError("removeSymbolMappings unimplemented"); }
		void removeSymbol(Symbol* symbol) override { throw LowlevelError("removeSymbol unimplemented"); }
		void renameSymbol(Symbol* sym, const string& newname) override { throw LowlevelError("renameSymbol unimplemented"); }
		void retypeSymbol(Symbol* sym, Datatype* ct) override { throw LowlevelError("retypeSymbol unimplemented"); }
		string makeNameUnique(const string& nm) const override { throw LowlevelError("makeNameUnique unimplemented"); }
		void saveXml(ostream& s) const override { throw LowlevelError("saveXml unimplemented"); }
		void restoreXml(const Element* el) override { throw LowlevelError("restoreXml unimplemented"); }
		void printEntries(ostream& s) const override { throw LowlevelError("printEntries unimplemented"); }
		int4 getCategorySize(int4 cat) const override { throw LowlevelError("getCategorySize unimplemented"); }
		Symbol* getCategorySymbol(int4 cat, int4 ind) const override { throw LowlevelError("getCategorySymbol unimplemented"); }
		void setCategory(Symbol* sym, int4 cat, int4 ind) override { throw LowlevelError("setCategory unimplemented"); }
	};
}

#endif