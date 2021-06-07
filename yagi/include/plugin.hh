#ifndef __YAGI_PLUGIN__
#define __YAGI_PLUGIN__

#include <idp.hpp>
#include <memory>
#include <sstream>
#include "decompiler.hh"

namespace yagi {

	/*!
	 * \brief	IdaPlugin definition
	 */
	class Plugin : public plugmod_t {
	protected:
		/*!
		 * \brief	the Ghidra decompiler
		 */
		std::unique_ptr<Decompiler> m_decompiler;

	public:
		/*!
		 * \brief	Plugin ctor
		 */
		explicit Plugin(std::unique_ptr<Decompiler> decompiler);

		/*!
		 * \brief	destructor
		 */
		virtual ~Plugin() = default;

		/*!
		 * \brief	copy id disable
		 */
		Plugin(const Plugin&) = delete;
		Plugin& operator=(const Plugin&) = delete;

		/*!
		 * \brief	move is forbidden
		 */
		Plugin(Plugin&&) noexcept = delete;
		Plugin& operator=(Plugin&&) noexcept = delete;

		/*!
		 * \brief	Run the plugin API
		 */
		virtual bool idaapi run(size_t) override;

		/*!
		 * \brief	View decompilation
		 */
		void view(const std::string& name, const Decompiler::Result& code) const;
	};
}

#endif