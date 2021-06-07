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
#include "idatype.hh"
#include <kernwin.hpp>
#include <sstream>

namespace yagi 
{
	/**********************************************************************/
	static std::optional<std::string> _ComputeKeyword(TWidget* w)
	{
		int x, y;
		if (get_custom_viewer_place(w, false, &x, &y) == NULL) 
		{
			return std::nullopt;
		}

		auto cursor = get_custom_viewer_curline(w, false);
		qstring result;
		tag_remove(&result, cursor);

		char* start = result.begin() + x;
		char* end = start;

		while (end <= result.end() && (qisalnum(*end) || *end == '_') && *end != '\0') {
			end++;
		}

		while (start >= result.begin() && (qisalnum(*start) || *start == '_')) {
			start--;
		}

		if (start >= end)
		{
			return std::nullopt;
		}

		if (start >= result.end())
		{
			return std::nullopt;
		}

		return std::string(start + 1, end - start - 1);
	}

	/**********************************************************************/
	static bool idaapi _KeyboardCallback(TWidget* w, int key, int shift, void* ud) 
	{
		if (shift != 0)
		{
			return false;
		}

		auto keyword = _ComputeKeyword(w);

		if (!keyword.has_value())
		{
			return false;
		}

		auto code = static_cast<Decompiler::Result*>(ud);
		auto addr = code->symbolAddress.find(keyword.value());

		if (addr == code->symbolAddress.end())
		{
			return false;
		}

		switch (key)
		{
		case 'Y':
			{
				auto typeInfo = IdaTypeInfoFactory().build(addr->second);
				if (!typeInfo.has_value())
				{
					return false;
				}

				auto name = qstring(typeInfo.value()->getName().c_str());
				if (ask_str(&name, HIST_IDENT, "Please enter the type declaration"))
				{
				}
			}
			break;
		}

		return true;
	}

	/**********************************************************************/
	static bool idaapi _DoubleClickCallback(TWidget* w, int shift, void* ud) 
	{
		auto code = static_cast<Decompiler::Result*>(ud);
		auto keyword = _ComputeKeyword(w);
		if (!keyword.has_value())
		{
			return false;
		}

		auto addr = code->symbolAddress.find(keyword.value());

		if (addr == code->symbolAddress.end())
		{
			return false;
		}

		return jumpto(addr->second);
	}


	/**********************************************************************/
	static const custom_viewer_handlers_t _ViewHandlers(
		_KeyboardCallback,
		nullptr,
		nullptr,
		nullptr,
		_DoubleClickCallback,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	);

	/**********************************************************************/
	Plugin::Plugin(std::unique_ptr<Decompiler> decompiler)
		: m_decompiler(std::move(decompiler))
	{}

	/**********************************************************************/
	bool idaapi Plugin::run(size_t)
	{
		auto func_address = get_screen_ea();

		std::stringstream ss;
		ss << "Ghidra" << std::hex << func_address;
		try
		{
			auto decompilerResult = m_decompiler->decompile(func_address);
			view(ss.str(), decompilerResult);
		}
		catch (Error& e)
		{
			msg(e.what());
		}
		catch (...)
		{
			msg("Error during decompilation\n");
		}
		return true;
	}

	/**********************************************************************/
	void Plugin::view(const std::string& name, const Decompiler::Result& code) const
	{
		strvec_t* sv = new strvec_t();
		std::istringstream iss(code.cCode);
		for (std::string line; std::getline(iss, line); )
		{
			sv->push_back(simpleline_t(line.c_str()));
		}

		simpleline_place_t s1;
		simpleline_place_t s2((int)(sv->size() - 1));
		auto w = create_custom_viewer(name.c_str(), &s1, &s2,
			&s1, nullptr, sv, &_ViewHandlers, new Decompiler::Result(code));
		TWidget* code_view = create_code_viewer(w);
		set_code_viewer_is_source(code_view);
		display_widget(code_view, WOPN_DP_TAB);
	}
} // end of namespace yagi