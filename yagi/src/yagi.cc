#ifndef USE_DANGEROUS_FUNCTIONS
#define USE_DANGEROUS_FUNCTIONS 1
#endif  // USE_DANGEROUS_FUNCTIONS

#ifndef USE_STANDARD_FILE_FUNCTIONS
#define USE_STANDARD_FILE_FUNCTIONS
#endif

#ifndef NO_OBSOLETE_FUNCS
#define NO_OBSOLETE_FUNCS
#endif

#include <sstream>

#include <loader.hpp>
#include <ida.hpp>
#include <idp.hpp>
#include <diskio.hpp>
#include <plugin.hh>
#include "decompiler.hh"
#include "ghidradecompiler.hh"
#include "ghidra.hh"
#include "idatype.hh"
#include "idasymbol.hh"
#include "idalogger.hh"
#include "loader.hh"

// Path name separator
#ifdef _WINDOWS
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif




static int processor_id() {
#if IDA_SDK_VERSION < 750
	return ph.id;
#else
	return PH.id;
#endif
}


#if IDA_SDK_VERSION < 760
#define inf_is_32bit_exactly inf_is_32bit
#define inf_is_16bit() !inf_is_64bit() && !inf_is_32bit()
#endif

/*!
 * \brief	try to detect compiler of the binary 
 */
static yagi::Compiler compute_compiler() {
	compiler_info_t info;
	inf_get_cc(&info);

	auto language = yagi::Compiler::Language::X86;
	switch (processor_id())
	{
	case PLFM_386:
		{
			switch (info.id)
			{
			case COMP_MS:
				language = yagi::Compiler::Language::X86_WINDOWS;
				break;
			case COMP_GNU:
				language = yagi::Compiler::Language::X86_GCC;
				break;
			}
		}
		break;
	case PLFM_ARM:
		language = yagi::Compiler::Language::ARM;
		break;
	case PLFM_PPC:
		language = yagi::Compiler::Language::PPC;
		break;
	case PLFM_MIPS:
		language = yagi::Compiler::Language::MIPS;
		break;
	case PLFM_SPARC:
		language = yagi::Compiler::Language::SPARC;
		break;
	case PLFM_AVR:
		language = yagi::Compiler::Language::ATMEL;
		break;
	case PLFM_6502:
		language = yagi::Compiler::Language::P6502;
		break;
	default:
		throw yagi::UnknownCompiler(processor_id());
	}

	auto mode = yagi::Compiler::Mode::M24;
	if (inf_is_64bit())
	{
		mode = yagi::Compiler::Mode::M64;
	}
	if (inf_is_16bit())
	{
		mode = yagi::Compiler::Mode::M16;
	}
	if (inf_is_32bit_exactly())
	{
		mode = yagi::Compiler::Mode::M32;
	}

	return yagi::Compiler(
		language,
		inf_is_be() ? yagi::Compiler::Endianess::BE : yagi::Compiler::Endianess::LE,
		mode
	);
}

/*
 *	\brief init function called from IDA directly
 */
static plugmod_t* idaapi yagi_init(void)
{
	qstrvec_t paths_list;
	std::stringstream s;

	auto logger = std::make_unique<yagi::IdaLogger>();

	// Search Ghidra plugin installation path in $IDAUSR or $IDADIR
	s << PLG_SUBDIR << PATH_SEP << "/Ghidra";
	get_ida_subdirs(&paths_list, s.str().c_str(), IDA_SUBDIR_ONLY_EXISTING);

	if (paths_list.empty()){
		logger->error("Ghidra folder missing. Yagi was not correctly installed.");
		return nullptr;
	}

	// Remove "/Ghidra" in the path
	std::string ghidraPath(paths_list.at(0).c_str());
	ghidraPath = ghidraPath.substr(0, ghidraPath.rfind(PATH_SEP));

	yagi::ghidra::init(ghidraPath);

	try
	{
		auto compilerId = compute_compiler();

		auto decompiler = yagi::GhidraDecompiler::build(
			compilerId,
			std::move(logger),
			std::make_unique<yagi::IdaSymbolInfoFactory>(),
			std::make_unique<yagi::IdaTypeInfoFactory>()
		);
		if (decompiler.has_value())
		{
			return new yagi::Plugin(std::move(decompiler.value()));
		}
	}
	catch (yagi::UnknownCompiler& e)
	{
		logger->error(e.what());
	}
	return nullptr;
}

/*!
 * \brief	This is an export present into loader.hpp of IDA SDK
 */
plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,
	PLUGIN_MULTI,
	yagi_init,
	nullptr,
	nullptr,
	"Yet Another Ghidra Integration",
	"Airbus CERT.\n",
	"yagi",
	"F3"
};
