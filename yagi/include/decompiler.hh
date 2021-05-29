#ifndef __YAGI_DECOMPILER__
#define __YAGI_DECOMPILER__

#include <memory>
#include <optional>

#include "idecompile.hh"
#include "typeinfo.hh"
#include "symbolinfo.hh"
#include "logger.hh"

namespace yagi 
{
	class YagiArchitecture;

	/*!
	 *	\brief	Implement the IDecompile interface for Ghidra
	 */
	class GhidraDecompiler : public IDecompiler
	{
	private:
		/*!
		 *	\brief	pointer to the main Ghidra architecture 
		 */
		std::unique_ptr<YagiArchitecture> m_architecture;

		/*!
		 * \brief	compute the Sleigh id from the computer type
		 */
		static std::string compute_sleigh_id(const Compiler& compilerType) noexcept;

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
		 *	\brief	main function for decompiler
		 *	\param	funcAddress	address of function to decompile
		 */
		std::string decompile(uint64_t funcAddress) override;

		/*!
		 *	\brief	factory
		 *			Use to build a ghidra decompiler interface
		 *	\param	compilerType	decompiler id to load
		 *  \param	logger	logger use to inform state of the decompilation
		 *	\param	symbolDatabase	symboles database use to increase the decompilation output
		 *  \param	typeDatabase	type declared use to increase the decompilation output
		 */
		static std::optional<std::unique_ptr<IDecompiler>> build(
			const Compiler& compilerType,
			std::unique_ptr<Logger> logger, 
			std::unique_ptr<SymbolInfoFactory> symbolDatabase, 
			std::unique_ptr<TypeInfoFactory> typeDatabase
		) noexcept;
	};
}

#endif