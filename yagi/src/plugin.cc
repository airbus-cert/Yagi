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
#include "idasymbol.hh"
#include <kernwin.hpp>
#include <loader.hpp>
#include <sstream>

namespace yagi 
{
	/**********************************************************************/
	static void _RunYagi()
	{
		auto plugins = get_plugins();
		while (plugins != nullptr)
		{
			if (plugins->name == std::string("yagi"))
			{
				run_plugin(plugins->entry, 0);
				break;
			}
			plugins = plugins->next;
		}
	}

	/**********************************************************************/
	/*!
	 * \brief	Print a string for type declaration for IDA
	 * \param	name	name of the symbol
	 * \param	typeInfo	Type informations associated
	 * \return	a compatible decl type
	 */
	static std::string _PrintDeclType(const std::string& name, const TypeInfo& typeInfo)
	{
		auto typeFunc = typeInfo.toFunc();
		if (typeFunc.has_value())
		{
			auto prototype = typeFunc.value()->getFuncPrototype();
			std::string cc = "";
			try
			{
				cc = typeFunc.value()->getCallingConv();
			}
			catch (UnknownCallingConvention&) {}

			auto paramIter = prototype.begin();

			if (prototype.size() > 0)
			{
				paramIter++;
				std::string parametersDecl = "";

				std::for_each(paramIter, prototype.end(), [&parametersDecl](const std::unique_ptr<TypeInfo>& info) {
					parametersDecl += info->getName() + ",";
					});

				// remove last char if present
				if (parametersDecl.size() > 0 && parametersDecl.back() == ',')
				{
					parametersDecl = parametersDecl.substr(0, parametersDecl.size() - 1);
				}

				return prototype[0]->getName() + " " + cc + " " + name + "(" + parametersDecl + ");";
			}
		}

		// normal case
		return typeInfo.getName() + ";";
	}

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
		case 'X':
			open_xrefs_window(addr->second);
			break;
		case 'N':
			{
				auto symbolInfo = IdaSymbolInfoFactory().find(addr->second);
				if (!symbolInfo.has_value())
				{
					return false;
				}

				auto name = qstring(symbolInfo.value()->getName().c_str());
				if (ask_str(&name, HIST_IDENT, "Please enter item name"))
				{
					set_name(addr->second, name.c_str());
					_RunYagi();
				}
			}
			break;
		case 'Y':
			{
				auto typeInfo = IdaTypeInfoFactory().build(addr->second);
				auto name = qstring(keyword.value().c_str()) + ";";
				if (typeInfo.has_value())
				{
					name = qstring(_PrintDeclType(keyword.value(), *typeInfo.value().get()).c_str());
				}

				if (ask_str(&name, HIST_TYPE, "Please enter the type declaration"))
				{
					tinfo_t idaTypeInfo;
					qstring parsedName;
					if (parse_decl(&idaTypeInfo, &parsedName, nullptr, name.c_str(), PT_TYP))
					{
						set_tinfo(addr->second, &idaTypeInfo);
						_RunYagi();
					}
				}
			}
			break;
		}

		return true;
	}

	/**********************************************************************/
	static void idaapi _Close(TWidget* cv, void* ud)
	{
		auto code = static_cast<Decompiler::Result*>(ud);
		delete code;
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
		_Close,
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

		try
		{
			auto decompilerResult = m_decompiler->decompile(func_address);
			if (decompilerResult.has_value())
			{
				view(decompilerResult.value().name, decompilerResult.value());
			}
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
		auto oldWidget = find_widget(name.c_str());
		if (oldWidget != nullptr)
		{
			close_widget(oldWidget, 0);
		}

		auto w = create_custom_viewer(name.c_str(), &s1, &s2,
			&s1, nullptr, sv, &_ViewHandlers, new Decompiler::Result(code));
		TWidget* code_view = create_code_viewer(w);
		set_code_viewer_is_source(code_view);
		display_widget(code_view, WOPN_DP_TAB);
	}
} // end of namespace yagi