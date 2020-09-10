#ifndef __YAGI_IDASYMBOLFACTORY__
#define __YAGI_IDASYMBOLFACTORY__

#include "symbolfactory.hh"

namespace yagi 
{
	class IdaSymbolFactory : public SymbolFactory
	{
	public:
		IdaSymbolFactory();

		/*!
		 *	\brief	Query a symbol by address
		 *	\param	ea	address of the symbol
		 *	\return	name of the symbol
		 */
		std::optional<std::string> getSymbol(uint64_t ea) override;

		std::optional<std::tuple<std::string, uint64_t, uint64_t>> getFunction(uint64_t ea) override;

		bool isFunction(uint64_t ea) override;
		bool isImport(const std::string& name) override;
		bool isLabel(uint64_t ea) override;
	};
}

#endif