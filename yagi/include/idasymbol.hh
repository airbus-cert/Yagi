#ifndef __YAGI_IDASYMBOLFACTORY__
#define __YAGI_IDASYMBOLFACTORY__

#include "symbolinfo.hh"

namespace yagi 
{
	/*!
	 * \brief	Symbol database interface from IDA to Yagi 
	 */
	class IdaSymbolInfo : public SymbolInfo 
	{
	public:
		/*!
		 *	\brief	ctor
		 */
		explicit IdaSymbolInfo(uint64_t ea, std::string name);

		/*!
		 * \brief	default ctor 
		 */
		~IdaSymbolInfo() = default;

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		IdaSymbolInfo(const IdaSymbolInfo&) = default;
		IdaSymbolInfo& operator=(const IdaSymbolInfo&) = default;

		/*!
		 *	\brief	moving is allowed
		 */
		IdaSymbolInfo(IdaSymbolInfo&&) noexcept = default;
		IdaSymbolInfo& operator=(IdaSymbolInfo&&) noexcept = default;

		/*!
		 *	\brief	if symbol refer to a function compute the size of the symbol
		 *	\return	the size of the symbol
		 *	\raise	SymbolIsNotAFunction
		 */
		uint64_t getFunctionSize() const override;

		/*!
		 * \brief	override the default name
		 *			with IDA API
		 */
		std::string getName() const override;

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

		/*!
		 *	\brief	Is the symbol is in Read only mode
		 *			Use to espand static data from read only memory space
		 */
		bool isReadOnly() const noexcept override;
	};

	class IdaFunctionSymbolInfo : public FunctionSymbolInfo
	{
	public:
		explicit IdaFunctionSymbolInfo(std::unique_ptr<SymbolInfo> symbol)
			: FunctionSymbolInfo{std::move(symbol)}
		{}

		/*!
		 *	\brief	Copy is forbidden due to unique ptr
		 */
		IdaFunctionSymbolInfo(const IdaFunctionSymbolInfo&) = delete;
		IdaFunctionSymbolInfo& operator=(const IdaFunctionSymbolInfo&) = delete;

		/*!
		 *	\brief	moving is allowed
		 */
		IdaFunctionSymbolInfo(IdaFunctionSymbolInfo&&) noexcept = default;
		IdaFunctionSymbolInfo& operator=(IdaFunctionSymbolInfo&&) noexcept = default;

		/*!
		 * \brief	Try to find a stored var definition at stack offset
		 *			Typically use to search in frame
		 * \param	offset		offset in the stack frame
		 * \param	addrSize	size of address in stack space
		 * \return	if found the name of the stack var
		 */
		std::optional<std::string> findStackVar(uint64_t offset, uint32_t addrSize) override;

		/*!
		 *	\brief	Find a custom stored name use at a particular address for this space
		 *			use to staore registry or stack var name
		 * \param	pc		use address
		 * \param	space	name of memory space
		 * \param	offset	symbol offset
		 * \return	if found the name of var
		 */
		std::optional<std::string> findName(uint64_t pc, const std::string& space, uint64_t& offset) override;

		/*!
		 * \brief	Save a var name use at pc for the space memory
		 *			use to save local stack or registry var name
		 * \param	loc		memory location
		 * \param	space	name of the memory space where var come from
		 */
		void saveName(const MemoryLocation& loc, const std::string& space) override;
		void saveName(uint64_t address, const std::string& space, uint64_t pc, const std::string& value);


		/*!
		 * \brief	Save a type ref use at memory location
		 *			Use to retype local (registry or stack) variables
		 * \param	loc		memory location
		 * \param	newType	new type associated to memory 
		 */
		void saveType(const MemoryLocation& loc, const TypeInfo& newType) override;

		/*!
		 * \brief	Save a type reference at pc
		 * \param	address	symbol address
		 * \param	space	space name
		 * \param	pc		pcode definition address
		 */
		void saveType(uint64_t address, const std::string& space, uint64_t pc, const TypeInfo& newType);

		/*!
		 * \brief	Clear type information link to a memory location
		 * \param	loc		memory location
		 */
		bool clearType(const MemoryLocation& loc) override;

		/*!
		 * \brief	Clear type information link to a definition address
		 * \param	space	space name
		 * \param	pc
		 */
		bool clearType(const std::string& space, uint64_t pc);

		/*!
		 * \brief	retrieve a local type use at pc address from a particular offset
		 *			Use to store local def into dedicated netnode
		 * \param	pc		use address
		 * \param	from	from wich space
		 * \param	offset	offset	in address space [out]
		 * \return	if found the offset into memory space and the linked type
		 */
		std::optional<std::unique_ptr<TypeInfo>> findType(uint64_t pc, const std::string& from, uint64_t& offset) override;
	};

	/*!
	 * \brief	Factory for IDA symbol
	 */
	class IdaSymbolInfoFactory : public SymbolInfoFactory
	{
	public:
		/*!
		 * \brief	ctor 
		 */
		IdaSymbolInfoFactory() = default;

		/*!
		 * \brief	destructor
		 */
		~IdaSymbolInfoFactory() = default;

		/*!
		 * \brief	Find any symbol at a particular address
		 *			This is the implementation for IDA
		 * \param	ea	the address of the symbol
		 * \return	optional symbol if found into database
		 */
		std::optional<std::unique_ptr<SymbolInfo>> find(uint64_t ea) override;

		/*!
		 * \brief	Find a function symbol from an address anywhere in the function
		 *			This is the implementation for IDA
		 * \param	ea	any address that is handle by a function
		 */
		std::optional<std::unique_ptr<FunctionSymbolInfo>> find_function(uint64_t ea) override;
	};
}

#endif