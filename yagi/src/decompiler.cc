#include "decompiler.hh"
#include "architecture.hh"
#include "scope.hh"
#include "typeinfo.hh"
#include "symbolinfo.hh"
#include "error.hh"

namespace yagi 
{
	GhidraDecompiler::GhidraDecompiler(std::unique_ptr<YagiArchitecture> architecture)
		: m_architecture(std::move(architecture))
	{

	}

	std::string GhidraDecompiler::decompile(uint64_t funcAddress)
	{
		auto funcSym = SymbolInfo::find(funcAddress, m_architecture->getSymbolDatabase());

		if (!funcSym.has_value())
		{
			throw UnableToFindFunction(funcAddress);
		}

		auto scope = m_architecture->symboltab->getGlobalScope();
		auto func = scope->findFunction(Address(m_architecture->getDefaultCodeSpace(), funcSym.value().getAddress()));
		
		try
		{
			m_architecture->clearAnalysis(func);
			m_architecture->allacts.getCurrent()->reset(*func);

			auto res = m_architecture->allacts.getCurrent()->perform(*func);

			m_architecture->setPrintLanguage("gaip-c-language");

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

	std::optional<std::unique_ptr<IDecompiler>> GhidraDecompiler::build(
		std::unique_ptr<ILogger> logger, 
		std::shared_ptr<SymbolFactory> symbolDatabase, 
		std::unique_ptr<TypeInfoFactory> typeDatabase
	) noexcept
	{
		try
		{
			auto architecture = std::make_unique<YagiArchitecture>("foo.c", std::move(logger), symbolDatabase, std::move(typeDatabase));
			DocumentStorage store;
			architecture->init(store);
			return std::make_unique<GhidraDecompiler>(std::move(architecture));
		}
		catch (LowlevelError& e)
		{
			logger->error(e.explain);
			return std::nullopt;
		}
	}
} // end of namespace ghidra