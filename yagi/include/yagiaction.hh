#ifndef __YAGI_ACTION__
#define __YAGI_ACTION__

#include "action.hh"

namespace yagi 
{
	/*!
	 * \brief	This action will try to synchronize name 
	 *			with the IDA frame space of the function
	 */
	class ActionSyncStackVar : public Action
	{
	public:
		ActionSyncStackVar(const string& g) : Action(Action::ruleflags::rule_onceperfunc, "syncstackvar", g) {}
		virtual Action* clone(const ActionGroupList& grouplist) const {
			if (!grouplist.contains(getGroup())) return (Action*)0;
			return new ActionSyncStackVar(getGroup());
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
	class ActionRenameVar : public Action
	{
	public:
		ActionRenameVar(const string& g) : Action(Action::ruleflags::rule_onceperfunc, "renamevar", g) {}
		virtual Action* clone(const ActionGroupList& grouplist) const {
			if (!grouplist.contains(getGroup())) return (Action*)0;
			return new ActionRenameVar(getGroup());
		}
		/*!
		 * \brief	Will apply the stack rename
		 */
		int4 apply(Funcdata& data) override;
	};

	class ActionLoadLocalScope : public Action
	{
	protected:
		std::string m_space;
	public:
		ActionLoadLocalScope(const string& g, const string& space) 
			: Action(Action::ruleflags::rule_onceperfunc, "load", g), m_space { space } 
		{}

		virtual Action* clone(const ActionGroupList& grouplist) const {
			if (!grouplist.contains(getGroup())) return (Action*)0;
			return new ActionSyncStackVar(getGroup());
		}
		/*!
		 * \brief	Will apply the stack rename
		 */
		int4 apply(Funcdata& data) override;
	};


	class ActionMIPST9Optimization : public Action
	{
	public:
		ActionMIPST9Optimization(const string& g)
			: Action(Action::ruleflags::rule_onceperfunc, "load", g)
		{}

		virtual Action* clone(const ActionGroupList& grouplist) const {
			if (!grouplist.contains(getGroup())) return (Action*)0;
			return new ActionMIPST9Optimization(getGroup());
		}
		/*!
		 * \brief	Will insert a context for T9 register
		 */
		int4 apply(Funcdata& data) override;
	};
}

#endif