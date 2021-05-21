#ifndef USE_DANGEROUS_FUNCTIONS
#define USE_DANGEROUS_FUNCTIONS 1
#endif  // USE_DANGEROUS_FUNCTIONS

#ifndef USE_STANDARD_FILE_FUNCTIONS
#define USE_STANDARD_FILE_FUNCTIONS
#endif

#ifndef NO_OBSOLETE_FUNCS
#define NO_OBSOLETE_FUNCS
#endif

#include "plugin.hh"
#include "ghidra.hh"
#include "symbolinfo.hh"
#include "exception.hh"
#include <kernwin.hpp>
#include <sstream>

namespace yagi 
{
	Plugin::Plugin(std::unique_ptr<IDecompiler> decompiler)
		: m_decompiler(std::move(decompiler))
	{}

	/*
	 * \brief	function called when plugin is called
	 */
	bool idaapi Plugin::run(size_t)
	{
		auto func_address = get_screen_ea();

		std::stringstream ss;
		ss << "Ghidra" << std::hex << func_address;
		try
		{
			view(ss.str(), m_decompiler->decompile(func_address));
		}
		catch (Error& e)
		{
			msg(e.what());
		}
		catch (...)
		{
			msg("Error during decompilation");
		}
		return true;
	}

	void Plugin::view(const std::string& name, const std::string& code) const
	{
		strvec_t* sv = new strvec_t();
		std::istringstream iss(code);
		for (std::string line; std::getline(iss, line); )
		{
			sv->push_back(simpleline_t(line.c_str()));
		}

		simpleline_place_t s1;
		simpleline_place_t s2((int)(sv->size() - 1));
		auto w = create_custom_viewer(name.c_str(), &s1, &s2,
			&s1, NULL, sv, nullptr, sv);
		TWidget* code_view = create_code_viewer(w);
		set_code_viewer_is_source(code_view);
		display_widget(code_view, WOPN_DP_TAB);
	}
} // end of namespace yagi