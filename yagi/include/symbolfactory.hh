#ifndef __YAGI_SYMBOLFACTORY__
#define __YAGI_SYMBOLFACTORY__

#include <string>
#include <optional>

namespace yagi 
{
	/*!
	 *	\brief	backend database to query symbol and type
	 *			Even if IDA will be the only backend, this is usefull for testing without IDA
	 */
	class SymbolFactory
	{
	public:
		virtual ~SymbolFactory() {}
		/*!
		 *	\brief	Query a symbol by address
		 *	\param	ea	address of the symbol
		 *	\return	name of the symbol
		 */
		virtual std::optional<std::string> getSymbol(uint64_t ea) = 0;

		/*!
		 *	\brief	Load the function symbol that include the address
		 *	\param	ea	any address of the function
		 *	\return	function name and start and end address if found
		 */
		virtual std::optional<std::tuple<std::string, uint64_t, uint64_t>> getFunction(uint64_t ea) = 0;

		virtual bool isFunction(uint64_t ea) = 0;
		virtual bool isImport(const std::string& name) = 0;
		virtual bool isLabel(uint64_t ea) = 0;
	};
}

#endif