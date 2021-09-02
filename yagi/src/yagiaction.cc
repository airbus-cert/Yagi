#include "yagiaction.hh"
#include "yagiarchitecture.hh"
#include "typemanager.hh"

namespace yagi 
{
	/*!
	 *	\brief	apply data sync with frame space 
	 */
	int4 ActionRenameStackVar::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcSym = arch->getSymbolDatabase().find_function(data.getAddress().getOffset());
		auto iter = data.getScopeLocal()->begin();
		while (iter != data.getScopeLocal()->end())
		{
			auto sym = *iter;
			if (sym->getAddr().getSpace()->getName() == "stack")
			{
				auto name = funcSym.value()->findStackVar(
					sym->getAddr().getOffset(), 
					sym->getAddr().getSpace()->getAddrSize()
				);

				if (name.has_value())
				{
					arch->getLogger().info("Apply stack sync var betwwen ", sym->getSymbol()->getName(), name.value());
					data.getScopeLocal()->renameSymbol(sym->getSymbol(), name.value());
				}
			}
			iter++;
		}
		return 0;
	}

	/*!
	 *	\brief	apply data sync with netnode for registry
	 */
	int4 ActionRenameRegistryVar::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcSym = arch->getSymbolDatabase().find_function(data.getAddress().getOffset());

		auto iter = data.getScopeLocal()->begin();
		while (iter != data.getScopeLocal()->end())
		{
			auto sym = *iter;
			if (sym->getAddr().getSpace()->getName() == "register")
			{
				auto newName = funcSym.value()->findRegVar(sym->getSymbol()->getName());
				if (newName.has_value())
				{
					data.getScopeLocal()->renameSymbol(sym->getSymbol(), newName.value());
				}
			}
			iter++;
		}
		return 0;
	}
} // end of namespace yagi