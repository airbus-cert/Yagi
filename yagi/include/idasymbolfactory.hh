#ifndef __YAGI_IDASYMBOLFACTORY__
#define __YAGI_IDASYMBOLFACTORY__

#include "symbolinfo.hh"

namespace yagi 
{

	class IdaSymbolInfo : public SymbolInfo 
	{
	public:
		/*!
		 *	\brief	ctor
		 */
		explicit IdaSymbolInfo(uint64_t ea, std::string name);

		/*!
		 *	\brief	if symbol refer to a function compute the size of the symbol
		 *	\return	the size of the symbol
		 *	\raise	SymbolIsNotAFunction
		 */
		uint64_t getFunctionSize() const override;


		/*!
		 *	\brief	state of symbol
		 *	\return	true if the symbol is a function
		 */
		bool isFunction() const noexcept override;

		/*!
		 *	\brief	state of the function
		 *	\return	true if symbol is associated to a symbol
		 */
		bool isLabel() const noexcept override;

		/*!
		 *	\brief	state of the symbol
		 *	\return	true the symbol is associate to an import
		 */
		bool isImport() const noexcept override;

		bool isReadOnly() const noexcept override;
	};

	class IdaSymbolInfoFactory : public SymbolInfoFactory
	{
	public:
		IdaSymbolInfoFactory();

		std::optional<std::unique_ptr<SymbolInfo>> find(uint64_t ea);
		std::optional<std::unique_ptr<SymbolInfo>> find_function(uint64_t ea);
	};
}

#endif