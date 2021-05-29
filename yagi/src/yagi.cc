#ifndef USE_DANGEROUS_FUNCTIONS
#define USE_DANGEROUS_FUNCTIONS 1
#endif  // USE_DANGEROUS_FUNCTIONS

#ifndef USE_STANDARD_FILE_FUNCTIONS
#define USE_STANDARD_FILE_FUNCTIONS
#endif

#ifndef NO_OBSOLETE_FUNCS
#define NO_OBSOLETE_FUNCS
#endif

#include <loader.hpp>
#include "ida.hpp"
#include "idp.hpp"
#include "plugin.hh"
#include "idecompile.hh"
#include "decompiler.hh"
#include "ghidra.hh"
#include "idatype.hh"
#include "idasymbol.hh"
#include "idalogger.hh"

static int processor_id() {
#if IDA_SDK_VERSION < 750
	return ph.id;
#else
	return PH.id;
#endif
}

static yagi::Compiler compute_compiler() {
	compiler_info_t info;
	inf_get_cc(&info);

	return yagi::Compiler(yagi::Compiler::Language::X86_WINDOWS, yagi::Compiler::Endianess::LE, yagi::Compiler::Mode::M64);
}

/*
 *	\brief init function called from IDA directly
 */
static plugmod_t* idaapi yagi_init(void)
{
	yagi::ghidra::init();
	
	auto decompiler = yagi::GhidraDecompiler::build(
		compute_compiler(),
		std::make_unique<yagi::IdaLogger>(),
		std::make_unique<yagi::IdaSymbolInfoFactory>(),
		std::make_unique<yagi::IdaTypeInfoFactory>()
	);
	if (decompiler.has_value())
	{
		return new yagi::Plugin(std::move(decompiler.value()));
	}
	else {
		return nullptr;
	}
}

// This is an export present into loader.hpp of IDA SDK
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
	"Alt-F5"
};
