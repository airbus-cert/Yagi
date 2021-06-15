#include "ghidradecompiler.hh"
#include "architecture.hh"
#include "scope.hh"
#include "typeinfo.hh"
#include "symbolinfo.hh"
#include "exception.hh"
#include "idaloader.hh"
#include "print.hh"
#include "base.hh"

namespace yagi 
{
	/**********************************************************************/
	GhidraDecompiler::GhidraDecompiler(std::unique_ptr<YagiArchitecture> architecture)
		: m_architecture(std::move(architecture))
	{

	}

	/**********************************************************************/
	std::optional<Decompiler::Result> GhidraDecompiler::decompile(uint64_t funcAddress)
	{
		try
		{
			auto funcSym = m_architecture->getSymbolDatabase().find_function(funcAddress);

			if (!funcSym.has_value())
			{
				m_architecture->getLogger().info("Unable to find a function at ", to_hex(funcAddress));
				return nullopt;
			}

			auto scope = m_architecture->symboltab->getGlobalScope();
			// clear scope to update all symbols
			scope->clear();

			auto func = scope->findFunction(
				Address(
					m_architecture->getDefaultCodeSpace(), 
					funcSym.value()->getAddress()
				)
			);

			m_architecture->clearAnalysis(func);
			m_architecture->allacts.getCurrent()->reset(*func);

			auto res = m_architecture->allacts.getCurrent()->perform(*func); 
			m_architecture->setPrintLanguage("yagi-c-language");

			stringstream ss;
			m_architecture->print->setIndentIncrement(3);
			m_architecture->print->setOutputStream(&ss);

			//print as C
			m_architecture->print->docFunction(func);

			// get back context information
			auto idaPrint = static_cast<IdaPrint*>(m_architecture->print);
			return Decompiler::Result(funcSym.value()->getName(), ss.str(), idaPrint->getEmitter().getSymbolAddr());
		}
		
		catch (LowlevelError& e)
		{
			m_architecture->getLogger().error(e.explain);
			return nullopt;
		}
		catch (Error& e)
		{
			m_architecture->getLogger().error(e.what());
			return nullopt;
		}
		catch(std::exception& e)
		{
			m_architecture->getLogger().error(e.what());
			return nullopt;
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
		case Compiler::Language::X86:
			language = "x86";
			languageMeta = "default";
			break;
		case Compiler::Language::X86_GCC:
			language = "x86";
			languageMeta = "default:gcc";
			break;
		case Compiler::Language::X86_WINDOWS:
			language = "x86";
			languageMeta = "default:windows";
			break;
		case Compiler::Language::PPC:
			language = "PowerPC";
			languageMeta = "default";
			break;
		case Compiler::Language::MIPS:
			language = "MIPS";
			languageMeta = "default";
			break;
		case Compiler::Language::ARM:
			{
				if (compilerType.mode == Compiler::Mode::M64)
				{
					language = "AARCH64";
					languageMeta = "v8A:default";
				}
				else 
				{
					language = "ARM";
					languageMeta = "v7";
				}
			}
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
	std::string GhidraDecompiler::compute_default_cc(const Compiler& compilerType)
	{
		switch (compilerType.language)
		{
		case Compiler::Language::X86:
			return "__stdcall";
		case Compiler::Language::X86_GCC:
			return "__stdcall";
		case Compiler::Language::X86_WINDOWS:
			return "__fastcall";
		case Compiler::Language::ARM:
			if (compilerType.mode == Compiler::Mode::M32)
				return "__stdcall";
			else
				return "__cdecl";
		case Compiler::Language::PPC:
			return "__stdcall";
		case Compiler::Language::MIPS:
			return "__stdcall";
		default:
			break;
		}

		throw NoDefaultCallingConvention();
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<Decompiler>> GhidraDecompiler::build(
		const Compiler& compilerType,
		std::unique_ptr<Logger> logger, 
		std::unique_ptr<SymbolInfoFactory> symbolDatabase, 
		std::unique_ptr<TypeInfoFactory> typeDatabase
	) noexcept
	{
		auto sleighId = compute_sleigh_id(compilerType);
		logger->info("load compiler with sleigh id : " + sleighId);

		auto architecture = std::make_unique<YagiArchitecture>(
			"", 
			sleighId,
			std::make_unique<IdaLoaderFactory>(),
			std::move(logger), 
			std::move(symbolDatabase), 
			std::move(typeDatabase),
			compute_default_cc(compilerType)
		);

		// compute injection
		switch (compilerType.language)
		{
		case Compiler::Language::X86:
		case Compiler::Language::X86_GCC:
		case Compiler::Language::X86_WINDOWS:
			architecture->addInjection("alloca_probe", "alloca_probe");
			architecture->addInjection("guard_dispatch_icall_fptr", "guard_dispatch_icall");
			break;
		}

		try
		{
			DocumentStorage store;
			architecture->init(store);
			return std::make_unique<GhidraDecompiler>(std::move(architecture));
		}
		catch (LowlevelError& e)
		{
			architecture->getLogger().error(e.explain);
			return std::nullopt;
		}
	}
} // end of namespace ghidra