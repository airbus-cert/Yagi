#ifndef __YAGI_DECOMPILER__
#define __YAGI_DECOMPILER__

#include <memory>
#include <optional>

#include "decompiler.hh"
#include "typeinfo.hh"
#include "symbolinfo.hh"
#include "logger.hh"
#include "loader.hh"

class Funcdata;

namespace yagi 
{

	class YagiArchitecture;

	/*!
	 *	\brief	Implement the IDecompile interface for Ghidra
	 */
	class GhidraDecompiler : public Decompiler
	{
	private:
		/*!
		 *	\brief	pointer to the main Ghidra architecture 
		 */
		std::unique_ptr<YagiArchitecture> m_architecture;

	protected:
		/*!
		 * \brief	Find high level variable and defined address
		 * \param	data	the source function
		 * \param	symbols	the output list of symbols
		 */
		void findVarSymbols(const Funcdata& data, std::map<std::string, MemoryLocation>& symbols) const;

		/*!
		 * \brief	Find calling function and populate the sylbol map
		 * \param	data	the source function
		 * \param	symbols	the output map populate by algo
		 */
		void findFunctionSymbols(const Funcdata& data, std::map<std::string, MemoryLocation>& symbols) const;

		/*!
		 * \param	Trying to find Constant symbols
		 * \param	data	the source function
		 * \param	symbols	the output symbol maps
		 */
		void findConstantSymbols(const Funcdata& data, std::map<std::string, MemoryLocation>& symbols) const;

	public:
		/*!
		 *	\brief	ctor
		 *	\param	architecture	Ghidra architecture
		 */
		explicit GhidraDecompiler(std::unique_ptr<YagiArchitecture> architecture);

		/*!
		 *	\brief	default deletor 
		 */
		virtual ~GhidraDecompiler() = default;

		/*!
		 *	\brief	Copy is forbidden 
		 */
		GhidraDecompiler(const GhidraDecompiler&) = delete;
		GhidraDecompiler& operator=(const GhidraDecompiler&) = delete;

		/*!
		 *	\brief	Move is authorized 
		 */
		GhidraDecompiler(GhidraDecompiler&&) noexcept = default;
		GhidraDecompiler& operator=(GhidraDecompiler&&) noexcept = default;

		/*!
		 * \brief	compute the Sleigh id from the computer type
		 */
		static std::string compute_sleigh_id(const Compiler& compilerType) noexcept;

		/*!
		 * \brief	compute default calling convention
		 * \param	compilerType	type of arch
		 */
		static std::string compute_default_cc(const Compiler& compilerType);

		/*!
		 *	\brief	main function for decompiler
		 *	\param	funcAddress	address of function to decompile
		 */
		std::optional<Decompiler::Result> decompile(uint64_t funcAddress) override;

		/*!
		 *	\brief	factory
		 *			Use to build a ghidra decompiler interface
		 *	\param	compilerType	decompiler id to load
		 *  \param	loaderFactory	Loader factory, use to interact with file or IDA
		 *  \param	logger	logger use to inform state of the decompilation
		 *	\param	symbolDatabase	symboles database use to increase the decompilation output
		 *  \param	typeDatabase	type declared use to increase the decompilation output
		 */
		static std::optional<std::unique_ptr<Decompiler>> build(
			const Compiler& compilerType,
			std::unique_ptr<Logger> logger, 
			std::unique_ptr<SymbolInfoFactory> symbolDatabase, 
			std::unique_ptr<TypeInfoFactory> typeDatabase
		) noexcept;
	};
}

#endif