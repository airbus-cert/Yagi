#ifndef __YAGI_ACTION__
#define __YAGI_ACTION__

#include "action.hh"

namespace yagi 
{
	/*!
	 * \brief	This action will try to synchronize name 
	 *			with the IDA frame space of the function
	 */
	class ActionRenameStackVar : public Action
	{
	public:
		ActionRenameStackVar(const string& g) : Action(Action::ruleflags::rule_onceperfunc, "namestackvars", g) {}
		virtual Action* clone(const ActionGroupList& grouplist) const {
			if (!grouplist.contains(getGroup())) return (Action*)0;
			return new ActionRenameStackVar(getGroup());
		}
		/*!
		 * \brief	Will apply the stack rename
		 */
		int4 apply(Funcdata& data) override;
	};

	/*!
	 * \brief	This action will try to synchronize name
	 *			with an IDA netnode
	 */
	class ActionRenameRegistryVar : public Action
	{
	public:
		ActionRenameRegistryVar(const string& g) : Action(Action::ruleflags::rule_onceperfunc, "nameregistryvars", g) {}
		virtual Action* clone(const ActionGroupList& grouplist) const {
			if (!grouplist.contains(getGroup())) return (Action*)0;
			return new ActionRenameStackVar(getGroup());
		}
		/*!
		 * \brief	Will apply the stack rename
		 */
		int4 apply(Funcdata& data) override;
	};
}

#endif