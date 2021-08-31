#include "yagiaction.hh"
#include "yagiarchitecture.hh"

namespace yagi 
{
	/*!
	 *	\brief	apply data sync with frame space 
	 */
	int4 ActionRenameStackVar::apply(Funcdata& data)
	{
		auto funcSym = static_cast<YagiArchitecture*>(data.getArch())->getSymbolDatabase().find_function(data.getAddress().getOffset());
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
					data.getScopeLocal()->renameSymbol(sym->getSymbol(), name.value());
				}
			}
			iter++;
		}
		return 0;
	}
} // end of namespace yagi