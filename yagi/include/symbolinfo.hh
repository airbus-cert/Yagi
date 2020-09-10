#ifndef __YAGI_SYMBOLINFO__
#define __YAGI_SYMBOLINFO__

#include "symbolfactory.hh"

#include <optional>
#include <tuple>
#include <string>
#include <memory>

namespace yagi 
{

	class SymbolInfo
	{
	protected:
		/*!
		 *	\brief	the address of symbol
		 */
		uint64_t m_ea;

		/*!
		 *	\brief	the associate string of the symbol
		 */
		std::string m_name;

		/*!
		 *	\brief	the backend database use to query the symbol
		 */
		std::shared_ptr<SymbolFactory> m_database;

		/*!
		 *	\brief	ctor
		 *			prefer using factory load of find
		 */
		explicit SymbolInfo(uint64_t ea, std::string name, std::shared_ptr<SymbolFactory> database);

	public:

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		SymbolInfo(const SymbolInfo&) = default;
		SymbolInfo& operator=(const SymbolInfo&) = default;

		/*!
		 *	\brief	moving is allowed
		 */
		SymbolInfo(SymbolInfo&&) noexcept = default;
		SymbolInfo& operator=(SymbolInfo&&) noexcept = default;

		/*!
		 *	\brief	destructor
		 */
		virtual ~SymbolInfo() = default;

		/*!
		 *	\brief	Try to determine the type (for ghidra) of the name symbol
		 */
		enum class Type
		{
			Function,	// Name is a function
			Label,		// Name is a code label
			Import,		// Name is an import function name
			Other		// Name is other
		};

		/*!
		 *	\brief	Try to load symbol at ea address
		 *	\param	ea	address of the desired symbol
		 *	\param	database	the backend database use
		 *	\return	if found the symbol
		 */
		static std::optional<SymbolInfo> load(uint64_t ea, std::shared_ptr<SymbolFactory> database) noexcept;

		/*!
		 *	\brief	Find the function symbol that encompass the address
		 *	\param	ea	any address of a function
		 *	\param	database	the backend database
		 */
		static std::optional<SymbolInfo> find(uint64_t ea, std::shared_ptr<SymbolFactory> database) noexcept;

		/*!
		 *	\brief	getter of the symbol address
		 *	\return	the address of the symbol
		 */
		uint64_t getAddress() const noexcept;

		/*!
		 *	\brief	if symbol refer to a function compute the size of the symbol
		 *	\return	the size of the symbol
		 *	\raise	SymbolIsNotAFunction
		 */
		uint64_t getFunctionSize() const;

		/*!
		 *	\brief	return the guess type of the function
		 *	\return the guessing type
		 */
		Type getType() const noexcept;

		/*!
		 *	\brief	the associate symbol name
		 *	\return symbol string
		 */
		std::string getName() const noexcept;

		/*!
		 *	\brief	state of symbol
		 *	\return	true if the symbol is a function
		 */
		bool isFunction() const noexcept;

		/*!
		 *	\brief	state of the function
		 *	\return	true if symbol is associated to a symbol
		 */
		bool isLabel() const noexcept;

		/*!
		 *	\brief	state of the symbol
		 *	\return	true the symbol is associate to an import
		 */
		bool isImport() const noexcept;
	};

}

#endif