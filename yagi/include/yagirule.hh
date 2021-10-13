#ifndef __YAGI_RULE__
#define __YAGI_RULE__

#include "action.hh"

namespace yagi 
{
	/*!
	 * \brief	Rule define to handle Control flow Guard on Windows
	 *			Control flow guard is a security mechanism that check
	 *			address of indirect call
	 */
	class RuleWindowsControlFlowGuard : public Rule {
	protected:
		/*!
		 *	\brief	Name of the CFG wrapper name that will be patched
		 */
		string m_cfgWrapperName;

	public:
		RuleWindowsControlFlowGuard(const string& g, const string& cfgWrapperName)
			: Rule(g, 0, "cfg_visual"), m_cfgWrapperName{ cfgWrapperName }
		{}

		virtual Rule* clone(const ActionGroupList& grouplist) const {
			if (!grouplist.contains(getGroup())) return (Rule*)0;
			return new RuleWindowsControlFlowGuard(getGroup(), m_cfgWrapperName);
		};
		void getOpList(vector<uint4>& oplist) const override;
		int4 applyOp(PcodeOp* op, Funcdata& data) override;
	};
}

#endif