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
		// to handle multiple var define at the same position
		std::map<std::string, uint64_t> localIndex;
		
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcSym = arch->getSymbolDatabase().find_function(data.getAddress().getOffset());

		auto iter = data.getScopeLocal()->begin();
		while (iter != data.getScopeLocal()->end())
		{
			auto sym = *iter;

			auto high = data.findHigh(sym->getSymbol()->getName());
			if (high != nullptr && high->getNameRepresentative() != nullptr && high->getNameRepresentative()->getDef() != nullptr)
			{
				auto newName = funcSym.value()->findRegVar(high->getNameRepresentative()->getDef()->getAddr().getOffset());
				if (newName.has_value())
				{
					std::stringstream ss;
					ss << newName.value();
					auto index = localIndex.find(newName.value());
					if (index == localIndex.end())
					{
						localIndex.emplace(newName.value(), 0);
					}
					else 
					{
						ss << "_" << index->second;
						index->second++;
					}

					auto computeName = ss.str();
					arch->getLogger().info("Apply registry sync var between ", sym->getSymbol()->getName(), computeName);
					data.getScopeLocal()->renameSymbol(high->getSymbol(), computeName);
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

		auto iter = data.beginOpAll();
		while (iter != data.endOpAll())
		{
			auto op = iter->second;
			
			uint64_t offset;
			auto newType = funcSym.value()->findType(op->getAddr().getOffset(), offset);

			if (newType.has_value())
			{
				if (op->getOut() != nullptr && 
					(		op->code() == CPUI_LOAD
						||	op->code() == CPUI_CALL 
						||	op->code() == CPUI_COPY 
						||	op->code() == CPUI_CALLIND 
						||	op->code() == CPUI_MULTIEQUAL 
						||	op->code() == CPUI_INDIRECT))
				{
					op->getOut()->updateType(static_cast<TypeManager*>(arch->types)->findByTypeInfo(*(newType.value())), true, true);
				}
			}
			iter++;
		}
		return 0;
	}
} // end of namespace yagi