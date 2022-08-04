#include "yagirule.hh"
#include "yagiarchitecture.hh"
#include "typemanager.hh"
#include "base.hh"

namespace yagi 
{
	/**********************************************************************/
	void RuleWindowsControlFlowGuard::getOpList(vector<uint4>& oplist) const
	{
		oplist.push_back(CPUI_CALLIND);
	}

	/**********************************************************************/
	int4 RuleWindowsControlFlowGuard::applyOp(PcodeOp* op, Funcdata& data)
	{
		auto addrSize = data.getArch()->getDefaultCodeSpace()->getAddrSize();

		auto sym = data.getArch()->symboltab->getGlobalScope()->findContainer(op->getIn(0)->getAddr(), addrSize, op->getAddr());
		
		// If the indirect call match the symbol name of the wrapped function
		// We replace the input varnode from const space (with the associated symbol)
		// with a register namespace
		if (sym != nullptr && sym->getSymbol() != nullptr && sym->getSymbol()->getName() == m_cfgWrapperName)
		{
			auto raxAddr = data.getArch()->getDefaultCodeSpace()->getTrans()->getRegister("RAX").getAddr();
			auto raxVn = data.newVarnode(
				addrSize,
				raxAddr
			);
			// Rewrite input with rax
			data.opSetInput(op, raxVn, 0);
			data.warningHeader("Yagi : Control Flow Guard patching");
		}

		return 0;
	}
} // end of namespace yagi