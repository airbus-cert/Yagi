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
#include "yagirule.hh"

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
					auto high = varnode->getHigh();
					auto nameRepr = high->getNameRepresentative();
					auto sym = high->getSymbol();

					if (nameRepr->getDef() != nullptr)
					{
						auto def = nameRepr->getDef();
						MemoryLocation loc(
							nameRepr->getAddr().getSpace()->getName(),
							nameRepr->getAddr().getOffset(),
							nameRepr->getAddr().getAddrSize()
						);
						loc.pc.push_back(def->getAddr().getOffset());
						symbols.emplace(sym->getName(),
							loc
						);
					}
					else {
						MemoryLocation loc(
							nameRepr->getAddr().getSpace()->getName(),
							nameRepr->getAddr().getOffset(),
							nameRepr->getAddr().getAddrSize()
						);

						// no name representative, so const (merge multiple variable)
						auto itOp = data.beginOp(data.getAddress());
						while (itOp != data.endOp(data.getAddress() + data.getSize()))
						{
							auto op = itOp->second;
							for (auto i = 0; i < op->numInput(); i++)
							{
								auto tmpVn = op->getIn(i);
								if (varnode->getAddr() == tmpVn->getAddr())
								{
									loc.pc.push_back(op->getAddr().getOffset());
								}
							}
							++itOp;
						}
						symbols.emplace(sym->getName(),
							loc
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
				"ram",
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
					"ram", 
					call->getEntryAddress().getOffset(), 
					call->getEntryAddress().getSpace()->getAddrSize()
				)
			);
		}
	}

	/**********************************************************************/
	void GhidraDecompiler::findConstantSymbols(const Funcdata& data, std::map<std::string, MemoryLocation>& symbols) const
	{
		auto iter = data.beginOp(data.getAddress());
		while (iter != data.endOp(data.getAddress() + data.getSize()))
		{
			auto op = iter->second;
			for (auto i = 0; i < op->numInput(); i++)
			{
				auto varnode = op->getIn(i);
				if (varnode->isConstant())
				{
					MemoryLocation loc(
						varnode->getAddr().getSpace()->getName(),
						varnode->getAddr().getOffset(),
						varnode->getAddr().getSpace()->getAddrSize()
					);
					loc.pc.push_back(op->getAddr().getOffset());
					
				}
			}
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

			// clear type factory
			m_architecture->types->clearNoncore();

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
			findConstantSymbols(*func, symbols);
			
			m_architecture->setPrintLanguage("yagi-c-language");

			stringstream ss;
			m_architecture->print->setIndentIncrement(3);
			m_architecture->print->setOutputStream(&ss);

			//print as C
			m_architecture->print->docFunction(func);

			// get back context information

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
			if (compilerType.mode == Compiler::Mode::M16)
			{
				languageMeta = "Real Mode";
			}
			else
			{
				languageMeta = "default";
			}
			
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
		case Compiler::Language::P6502:
			language = "6502";
			languageMeta = "default";
			break;
		case Compiler::Language::Z80:
			language = "z80";
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
		case Compiler::Language::eBPF:
			language = "eBPF";
			languageMeta = "default";
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
		case Compiler::Language::X86_WINDOWS:
			return "__fastcall";
		case Compiler::Language::ARM:
			if (compilerType.mode == Compiler::Mode::M32)
				return "__stdcall";
			else
				return "__cdecl";
		default:
			return "__stdcall";
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

		// compute architecture rule and action
		switch (compilerType.language)
		{
		case Compiler::Language::X86:
		case Compiler::Language::X86_GCC:
		case Compiler::Language::X86_WINDOWS:
			architecture->addInjection("alloca_probe", "alloca_probe");
			architecture->extra_pool_rules.push_back(new RuleWindowsControlFlowGuard("analysis", "guard_dispatch_icall_fptr"));
			break;
		case Compiler::Language::MIPS:
			architecture->addArchAction(new ActionMIPST9Optimization("t9optim"));
			break;
		case Compiler::Language::eBPF:
			architecture->addInitAction(new ActionAddeBPFSyscall("ebpfsyscall"));
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