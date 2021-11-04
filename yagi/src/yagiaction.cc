#include "yagiaction.hh"
#include "yagiarchitecture.hh"
#include "typemanager.hh"
#include "base.hh"

namespace yagi 
{
	/**********************************************************************/
	/*!
	 *	\brief	apply data sync with frame space 
	 */
	int4 ActionSyncStackVar::apply(Funcdata& data)
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
						arch->getLogger().info("Apply stack name override from frame ", high->getSymbol()->getName(), name.value());
						data.getScopeLocal()->renameSymbol(high->getSymbol(), name.value());
					}
					else
					{
						arch->getLogger().info("Apply stack name override from frame ", sym->getSymbol()->getName(), name.value());
						data.getScopeLocal()->renameSymbol(sym->getSymbol(), name.value());
					}
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
	int4 ActionRenameVar::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcSym = arch->getSymbolDatabase().find_function(data.getAddress().getOffset());

		auto iter = data.beginOpAll();
		while (iter != data.endOpAll())
		{
			auto op = iter->second;

			uint64_t offset;
			auto newName = funcSym.value()->findName(op->getAddr().getOffset(), m_space, offset);

			auto space = m_space;
			if (space == "const")
			{
				space = "stack";
			}

			auto symEntry = data.getScopeLocal()->findAddr(Address(arch->getSpaceByName(space), offset), op->getAddr());

			if (newName.has_value() && symEntry != nullptr)
			{
				auto sym = symEntry->getSymbol();
				data.getScopeLocal()->renameSymbol(sym,  newName.value());
				data.getScopeLocal()->setAttribute(sym, Varnode::namelock);
			}
			iter++;
		}
		return 0;
	}

	/**********************************************************************/
	int4 ActionLoadLocalScope::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcSym = arch->getSymbolDatabase().find_function(data.getAddress().getOffset());

		auto iter = data.beginOpAll();
		while (iter != data.endOpAll())
		{
			auto op = iter->second;

			uint64_t offset;
			auto newType = funcSym.value()->findType(op->getAddr().getOffset(), m_space, offset);

			auto space = m_space;
			if (space == "const")
			{
				space = "stack";
			}
			
			auto opAddr = op->getAddr();

			// for stack based symbol
			// we didn't specify any usepoint
			if (space == "stack")
			{
				opAddr = Address();
			}

			auto symEntry = data.getScopeLocal()->findAddr(Address(arch->getSpaceByName(space), offset), opAddr);

			if (newType.has_value() && data.getScopeLocal()->findAddr(Address(arch->getSpaceByName(space), offset), opAddr) == nullptr)
			{
				if (symEntry == nullptr)
				{
					auto sym = data.getScopeLocal()->addSymbol(
						"",
						static_cast<TypeManager*>(arch->types)->findByTypeInfo(*(newType.value())),
						Address(arch->getSpaceByName(space), offset),
						opAddr
					)->getSymbol();

					data.getScopeLocal()->setAttribute(sym, Varnode::typelock);
					//sym->setIsolated(true);
				}
			}
			iter++;
		}
		return 0;
	}

	/**********************************************************************/
	int4 ActionMIPST9Optimization::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcAddr = data.getAddress();
		auto addrSize = data.getArch()->getDefaultCodeSpace()->getAddrSize();

		// We will add a pcode at the begining of the function
		// t9 = funcAddr -> CPUI_COPY input0 = addrFunc; output = t9
		auto beginIter = data.beginOp(funcAddr);
		auto beginPcode = beginIter->second;

		// find t9 register
		auto t9Addr = data.getArch()->getDefaultCodeSpace()->getTrans()->getRegister("t9").getAddr();
		auto t9Vn = data.newVarnode(
			addrSize,
			t9Addr
		);

		auto newPcode = data.newOp(1, funcAddr);
		data.opSetOpcode(newPcode, CPUI_COPY);	// CPUI_COPY
		data.opSetInput(newPcode, data.newConstant(arch->getDefaultCodeSpace()->getAddrSize(), funcAddr.getOffset()), 0); // input = funcAddr
		data.opSetOutput(newPcode, t9Vn);	// output = t9
		data.opInsertBegin(newPcode, beginPcode->getParent());
		data.warningHeader("Yagi : setting t9 register value with address of the current function");
		return 0;
	}
} // end of namespace yagi