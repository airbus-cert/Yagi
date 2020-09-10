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
#include "plugin.hh"
#include "decompiler.hh"
#include "ghidra.hh"
#include "idatypefactory.hh"
#include "idasymbolfactory.hh"
#include "idalogger.hh"

/*
 *	\brief init function called from IDA directly
 */
static plugmod_t* idaapi yagi_init(void)
{
	yagi::ghidra::init();
	
	auto decompiler = yagi::GhidraDecompiler::build(
		std::make_unique<yagi::IdaLogger>(),
		std::make_shared<yagi::IdaSymbolFactory>(),
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
