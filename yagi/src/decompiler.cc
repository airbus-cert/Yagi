#include "decompiler.hh"
#include "architecture.hh"
#include "scope.hh"
#include "typeinfo.hh"
#include "symbolinfo.hh"
#include "exception.hh"

namespace yagi 
{
	/**********************************************************************/
	GhidraDecompiler::GhidraDecompiler(std::unique_ptr<YagiArchitecture> architecture)
		: m_architecture(std::move(architecture))
	{

	}

	/**********************************************************************/
	std::string GhidraDecompiler::decompile(uint64_t funcAddress)
	{
		auto funcSym = m_architecture->getSymbolDatabase()->find_function(funcAddress);

		if (!funcSym.has_value())
		{
			throw UnableToFindFunction(funcAddress);
		}

		auto scope = m_architecture->symboltab->getGlobalScope();
		auto func = scope->findFunction(Address(m_architecture->getDefaultCodeSpace(), funcSym.value()->getAddress()));
		
		try
		{
			m_architecture->clearAnalysis(func);
			m_architecture->allacts.getCurrent()->reset(*func);

			auto res = m_architecture->allacts.getCurrent()->perform(*func);

			m_architecture->setPrintLanguage("yagi-c-language");

			stringstream ss;
			m_architecture->print->setIndentIncrement(3);

			m_architecture->print->setOutputStream(&ss);

			//print as C
			m_architecture->print->docFunction(func);
			return ss.str();
		}
		
		catch (LowlevelError& e)
		{
			return e.explain;
		}
	}

	/**********************************************************************/
	std::string GhidraDecompiler::compute_sleigh_id(const Compiler& compilerType) noexcept {

		std::string language = "";
		std::string languageMeta = "";

		std::string endianess = "";
		std::string mode = "";

		switch (compilerType.language)
		{
		case Compiler::Language::X86_WINDOWS:
			language = "x86";
			languageMeta = "default:windows";
			break;
		}
		
		switch (compilerType.endianess)
		{
		case Compiler::Endianess::BE:
			endianess = "BE";
			break;
		case Compiler::Endianess::LE:
			endianess = "LE";
			break;
		default:
			break;
		}

		switch (compilerType.mode)
		{
		case Compiler::Mode::M32:
			mode = "32";
			break;
		case Compiler::Mode::M64:
			mode = "64";
			break;
		default:
			break;
		}

		return language + ":" + endianess + ":" + mode + ":" + languageMeta;
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<IDecompiler>> GhidraDecompiler::build(
		const Compiler& compilerType,
		std::unique_ptr<ILogger> logger, 
		std::unique_ptr<SymbolInfoFactory> symbolDatabase, 
		std::unique_ptr<TypeInfoFactory> typeDatabase
	) noexcept
	{
		auto sleighId = compute_sleigh_id(compilerType);
		logger->info("load compiler with sleigh id : " + sleighId);

		auto architecture = std::make_unique<YagiArchitecture>(
			"", 
			sleighId,
			std::move(logger), 
			std::move(symbolDatabase), 
			std::move(typeDatabase)
		);

		try
		{
			DocumentStorage store;
			architecture->init(store);
			return std::make_unique<GhidraDecompiler>(std::move(architecture));
		}
		catch (LowlevelError& e)
		{
			architecture->getLogger()->error(e.explain);
			return std::nullopt;
		}
	}
} // end of namespace ghidra