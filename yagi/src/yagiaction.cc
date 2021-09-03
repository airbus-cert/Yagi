#include "yagiaction.hh"
#include "yagiarchitecture.hh"
#include "typemanager.hh"

namespace yagi 
{
	/**********************************************************************/
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
					auto high = data.findHigh(sym->getSymbol()->getName());
					if (high != nullptr)
					{
						data.getScopeLocal()->renameSymbol(high->getSymbol(), name.value());
					}
					else
					{
						data.getScopeLocal()->renameSymbol(sym->getSymbol(), name.value());
					}
					arch->getLogger().info("Apply stack sync var between ", sym->getSymbol()->getName(), name.value());

				}
			}
			iter++;
		}
		return 0;
	}

	/**********************************************************************/
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
					auto high = data.findHigh(sym->getSymbol()->getName());
					if (high != nullptr)
					{
						arch->getLogger().info("Apply registry sync var between ", sym->getSymbol()->getName(), newName.value());
						data.getScopeLocal()->renameSymbol(high->getSymbol(), newName.value());
					}
				}
			}
			iter++;
		}
		return 0;
	}

	/**********************************************************************/
	int4 ActionRetypeRegistryVar::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcSym = arch->getSymbolDatabase().find_function(data.getAddress().getOffset());

		std::vector<std::string> retypeCandidate;
		auto iter = data.getScopeLocal()->begin();
		while (iter != data.getScopeLocal()->end())
		{
			auto sym = *iter;
			auto newType = funcSym.value()->findSymbolType(sym->getSymbol()->getName());
			if (newType.has_value())
			{
				retypeCandidate.push_back(sym->getSymbol()->getName());
			}
			iter++;
		}

		for (auto candidate : retypeCandidate)
		{
			std::vector<Symbol*> res;
			data.getScopeLocal()->findByName(candidate, res);

			for (auto sym : res)
			{
				try
				{
					auto newType = funcSym.value()->findSymbolType(candidate);
					auto high = data.findHigh(sym->getName());
					if (high != nullptr)
					{
						data.getScopeLocal()->retypeSymbol(high->getSymbol(), static_cast<TypeManager*>(arch->types)->findByTypeInfo(*(newType.value())));
						data.getScopeLocal()->setAttribute(high->getSymbol(), Varnode::typelock);
						break;
					}
				}
				catch (LowlevelError& e)
				{
					arch->getLogger().error(e.explain);
				}
			}

		}
		
		return 0;
	}
} // end of namespace yagi