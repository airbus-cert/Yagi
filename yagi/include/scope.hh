#ifndef __YAGI_SCOPE__
#define __YAGI_SCOPE__

#include <memory>
#include <map>
#include <libdecomp.hh>
#include "yagiarchitecture.hh"

namespace yagi 
{
	class YagiScope : public Scope
	{
	protected:
		/*!
		 * \brief	use proxy design pattern
		 *			Aggregate is better then inheritance
		 */
		ScopeInternal m_proxy;

		/*!
		 * \brief	Unimplemented 
		 */
		void addSymbolInternal(Symbol* sym) override;

		/*!
		 * \brief	Unimplemented
		 */
		SymbolEntry* addMapInternal(Symbol* sym, uint4 exfl, const Address& addr, int4 off, int4 sz, const RangeList& uselim) override;

		/*!
		 * \brief	build subscope for sub function
		 * \param	id	id of he scope
		 * \param	nm	name of the scope
		 * \return	ScopeInternal implementation
		 */
		Scope* buildSubScope(uint8 id, const string& nm) override;

		/*!
		 * \brief	Unimplemented
		 */
		void removeRange(AddrSpace* spc, uintb first, uintb last) override;

		/*!
		 * \brief	Unimplemented
		 */
		SymbolEntry* addDynamicMapInternal(Symbol* sym, uint4 exfl, uint8 hash, int4 off, int4 sz, const RangeList& uselim);

		void setInjectAttribute(Funcdata& fd, std::string inject_name);
	public:
		/*!
		 * \brief	ctor
		 * \param	id	scope id
		 * \param	architecture	current architecture
		 */
		explicit YagiScope(uint8_t id, YagiArchitecture* architecture);

		/*!
		 * \brief	find a funciton symbol
		 * \param	addr	addresse of the symbol
		 */
		virtual Funcdata* findFunction(const Address& addr) const override;

		/*!
		 * \brief	build a new param name
		 * \param	address of the variable
		 * \param	pc
		 * \param	ct	assocuiated data type
		 * \param	index
		 * \param	flags
		 */
		string buildVariableName(const Address& addr, const Address& pc, Datatype* ct, int4& index, uint4 flags) const override;

		/*!
		 *	\brief	add a new symbol in cache 
		 */
		SymbolEntry* addSymbol(const string& name, Datatype* ct, const Address& addr, const Address& usepoint) override;

		/*!
		 * \brief	return the proxy object for proxy pattern
		 */
		ScopeInternal* getProxy();
		
		/*!
		 * \param	addr	address of the symbol
		 * \return	entry at the addr
		 */
		SymbolEntry* findAddr(const Address& addr, const Address& usepoint) const override;

		/*!
		 * \brief	try to find any symbol at a particular address
		 * \param	addr	address of the symbol
		 * \param	size	size of the symbol
		 * \param	usepoint
		 * \return	symbol at the specific address
		 */
		SymbolEntry* findContainer(const Address& addr, int4 size, const Address& usepoint) const override;

		/*!
		 * \brief	Find the closest symbol
		 *			Use proxy for implementation
		 */
		SymbolEntry* findClosestFit(const Address&, int4, const Address&) const override;

		/*!
		 * \brief	find externaml references
		 *			All imported data
		 * \param	addr	address of the external symbol
		 * \return	the symbol associated with external ref
		 */
		ExternRefSymbol* findExternalRef(const Address& addr) const override;

		/*!
		 * \brief	find a label
		 * \param	addr address of the label
		 * \return	code label
		 */
		LabSymbol* findCodeLabel(const Address& addr) const override;

		/*!
		 *	\brief	chcek for name existence 
		 */
		bool isNameUsed(const std::string&, const Scope*) const override;

		/*!
		 * \brief	Load func data (prototype, calling convention etc...) for an import (or global variables) 
		 * \param	sym	symbol of the import
		 * \return	funciton data
		 */
		Funcdata* resolveExternalRefFunction(ExternRefSymbol* sym) const override;

		/*!
		 * \brief	clear the cache
		 *			Actually we used a global scope for only one function
		 *			because we serve as proxy for IDA
		 */
		void clear(void) override;

		/*!
		 * \brief	adjust cache is the new interface
		 *			Use proxy
		 */
		void adjustCaches(void) override;
		
		/*!
		 * \brief	even not found symol can have a name
		 * \return	name of the symbol
		 */
		string buildUndefinedName(void) const override;

		/*!
		 * \brief	common function to deal with private members
		 * \param	symbol
		 * \param	attr	like read only attribute
		 */
		void setAttribute(Symbol* sym, uint4 attr) override;

		/*!
		 * \brief	rest all attributes
		 */
		void clearAttribute(Symbol* sym, uint4 attr) override;

		/*!
		 * \brief	format display
		 */
		void setDisplayFormat(Symbol* sym, uint4 attr) override;

		/*!
		 * \brief	Unimplemented
		 */
		SymbolEntry* findOverlap(const Address& addr, int4 size) const;

		/*!
		 * \brief	Unimplemented
		 */
		SymbolEntry* findBefore(const Address& addr) const;

		/*!
		 * \brief	Unimplemented
		 */
		SymbolEntry* findAfter(const Address& addr) const;

		/*!
		 * \brief	Unimplemented
		 */
		void findByName(const string& name, vector<Symbol*>& res) const;

		/*!
		 * \brief	Unimplemented
		 */
		MapIterator begin() const override;

		/*!
		 * \brief	Unimplemented
		 */
		MapIterator end() const override;

		/*!
		 * \brief	Unimplemented
		 */
		list<SymbolEntry>::const_iterator beginDynamic() const override;

		/*!
		 * \brief	Unimplemented
		 */
		list<SymbolEntry>::const_iterator endDynamic() const override;

		/*!
		 * \brief	Unimplemented
		 */
		list<SymbolEntry>::iterator beginDynamic() override;

		/*!
		 * \brief	Unimplemented
		 */
		list<SymbolEntry>::iterator endDynamic() override;

		/*!
		 * \brief	Unimplemented
		 */
		void clearCategory(int4 cat) override;

		/*!
		 * \brief	Unimplemented
		 */
		void clearUnlockedCategory(int4 cat) override;

		/*!
		 * \brief	Unimplemented
		 */
		void clearUnlocked() override;

		/*!
		 * \brief	Unimplemented
		 */
		void restrictScope(Funcdata* f) override;

		/*!
		 * \brief	Unimplemented
		 */
		void removeSymbolMappings(Symbol* symbol) override;

		/*!
		 * \brief	Unimplemented
		 */
		void removeSymbol(Symbol* symbol) override;

		/*!
		 * \brief	Rename a symbol
		 */
		void renameSymbol(Symbol* sym, const string& newname) override;

		/*!
		 * \brief	Unimplemented
		 */
		void retypeSymbol(Symbol* sym, Datatype* ct) override;

		/*!
		 * \brief	Unimplemented
		 */
		string makeNameUnique(const string& nm) const override;

		/*!
		 * \brief	Unimplemented
		 */
		void saveXml(ostream& s) const override;

		/*!
		 * \brief	Unimplemented
		 */
		void restoreXml(const Element* el) override;

		/*!
		 * \brief	Unimplemented
		 */
		void printEntries(ostream& s) const override;

		/*!
		 * \brief	Unimplemented
		 */
		int4 getCategorySize(int4 cat) const override;

		/*!
		 * \brief	Unimplemented
		 */
		Symbol* getCategorySymbol(int4 cat, int4 ind) const override;

		/*!
		 * \brief	Unimplemented
		 */
		void setCategory(Symbol* sym, int4 cat, int4 ind) override;
	};
}

#endif