#include "ghidradecompiler.hh"
#include "architecture.hh"
#include "scope.hh"
#include "typeinfo.hh"
#include "symbolinfo.hh"
#include "exception.hh"
#include "idaloader.hh"
#include "print.hh"
#include "base.hh"
#include "yagiaction.hh"

namespace yagi 
{
	/**********************************************************************/
	GhidraDecompiler::GhidraDecompiler(std::unique_ptr<YagiArchitecture> architecture)
		: m_architecture(std::move(architecture))
	{

	}

	/**********************************************************************/
	void GhidraDecompiler::findVarSymbols(const Funcdata& data, std::map<std::string, MemoryLocation>& symbols) const
	{
		auto iter = data.beginDef();
		while (iter != data.endDef())
		{
			auto varnode = *iter;
			try
			{
				if (
					varnode->getHigh() != nullptr &&
					varnode->getHigh()->getSymbol() != nullptr &&
					varnode->getHigh()->getNameRepresentative() != nullptr 
					)
				{
				
					// if a local variable
					if (varnode->getHigh()->getNameRepresentative()->getDef() != nullptr)
					{
						auto high = varnode->getHigh();
						auto nameRepr = high->getNameRepresentative();
						auto def = nameRepr->getDef();
						auto sym = high->getSymbol();
						symbols.emplace(sym->getName(),
							MemoryLocation(
								nameRepr->getAddr().getSpace()->getName(),
								nameRepr->getAddr().getOffset(),
								nameRepr->getAddr().getSpace()->getAddrSize(),
								def->getAddr().getOffset(),
								sym->getType()->getSize()
							)
						);
					}
					else {
						auto high = varnode->getHigh();
						auto nameRepr = high->getNameRepresentative();
						auto sym = high->getSymbol();
						symbols.emplace(sym->getName(),
							MemoryLocation(
								nameRepr->getAddr().getSpace()->getName(),
								nameRepr->getAddr().getOffset(),
								nameRepr->getAddr().getSpace()->getAddrSize()
							)
						);
					}
				}
			}
			catch (LowlevelError&) {}
			iter++;
		}
	}

	/**********************************************************************/
	void GhidraDecompiler::findFunctionSymbols(const Funcdata& data, std::map<std::string, MemoryLocation>& symbols) const
	{
		// first we add the local function symbol
		symbols.emplace(data.getName(),
			MemoryLocation(
				MemoryLocation::MemoryLocationType::RAM,
				data.getAddress().getOffset(),
				data.getAddress().getSpace()->getAddrSize()
			)
		);

		for (auto i = 0; i < data.numCalls(); i++)
		{
			auto call = data.getCallSpecs(i);
			if(call->getEntryAddress().getSpace() == nullptr)
			{
				continue;
			}

			auto name = call->getName();

			if (name.substr(0, SymbolInfo::IMPORT_PREFIX.length()) == SymbolInfo::IMPORT_PREFIX)
			{
				name = name.substr(SymbolInfo::IMPORT_PREFIX.length(), name.length() - SymbolInfo::IMPORT_PREFIX.length());
			}

			symbols.emplace(name,
				MemoryLocation(
					MemoryLocation::MemoryLocationType::RAM, 
					call->getEntryAddress().getOffset(), 
					call->getEntryAddress().getSpace()->getAddrSize()
				)
			);
		}
	}

	/**********************************************************************/
	void GhidraDecompiler::findLocalSymbols(const Funcdata& data, std::map<std::string, MemoryLocation>& symbols) const
	{
		auto iter = data.getScopeLocal()->begin();
		while (iter != data.getScopeLocal()->end())
		{
			auto symEntry = *iter;
			auto sym = symEntry->getSymbol();
			symbols.emplace(
				sym->getName(),
				MemoryLocation(
					symEntry->getAddr().getSpace()->getName(),
					symEntry->getAddr().getOffset(),
					symEntry->getAddr().getSpace()->getAddrSize(),
					symEntry->getFirstUseAddress().getOffset(),
					sym->getType()->getSize()
				)
			);
			iter++;
		}
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
					funcSym.value()->getSymbol().getAddress()
				)
			);

			m_architecture->clearAnalysis(func);
			m_architecture->performActions(*func);

			// now we compute symbols
			std::map<std::string, MemoryLocation> symbols;
			findVarSymbols(*func, symbols);
			findFunctionSymbols(*func, symbols);
			findLocalSymbols(*func, symbols);
			
			m_architecture->setPrintLanguage("yagi-c-language");

			stringstream ss;
			m_architecture->print->setIndentIncrement(3);
			m_architecture->print->setOutputStream(&ss);

			//print as C
			m_architecture->print->docFunction(func);

			// get back context information
			auto idaPrint = static_cast<IdaPrint*>(m_architecture->print);
			return Decompiler::Result(
				funcSym.value()->getSymbol().getName(), 
				funcSym.value()->getSymbol().getAddress(),
				ss.str(), 
				symbols
			);
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
		case Compiler::Language::SPARC:
			language = "sparc";
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
		case Compiler::Language::ATMEL:
			{
				if (compilerType.mode == Compiler::Mode::M32)
				{
					language = "avr32";
					languageMeta = "default";
				}
				else if (compilerType.mode == Compiler::Mode::M24)
				{
					language = "avr8";
					languageMeta = "xmega";
				}
				else if (compilerType.mode == Compiler::Mode::M16)
				{
					language = "avr8";
					languageMeta = "default";
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
		case Compiler::Mode::M16:
			mode = "16";
			break;
		case Compiler::Mode::M24:
			mode = "24";
			break;
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
		case Compiler::Language::SPARC:
			return "__stdcall";
		case Compiler::Language::ATMEL:
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
} // end of namespace yagi